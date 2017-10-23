// top level includes
#include "zsglobal.h"

// system includes
#include <iostream>
#include <deque>
#include <set>
#include <sys/stat.h>
#include <utility>
#include <utime.h>

// library includes
#include <cpr/cpr.h>
extern "C" {
    // temporarily include curl as well until cpr can be used directly
    #include <curl/curl.h>
    #include <zsync.h>
    #include <zlib.h>
}

// local includes
#include "zsclient.h"
#include "zsutil.h"
extern "C" {
    #include "legacy_http.h"
    #ifdef ZSYNC_STANDALONE
        #include "legacy_progress.h"
    #endif
}

namespace zsync2 {
    class ZSyncClient::Private {
    public:
        // there might be more than one seed file
        // using a set to avoid duplicate entries
        std::set<std::string> seedFiles;
        
        const std::string pathOrUrlToZSyncFile;
        std::string pathToLocalFile;

        struct zsync_state *zsHandle;

        std::string referer;

        enum State {INITIALIZED, RUNNING, DONE};
        State state;

        long long localUsed;
        long long httpDown;

        // status message variables
#ifndef ZSYNC_STANDALONE
        std::deque<std::string> statusMessages;
#endif

    public:
        Private(std::string pathOrUrlToZSyncFile,
                std::string pathToLocalFile) : pathOrUrlToZSyncFile(std::move(pathOrUrlToZSyncFile)),
                                               pathToLocalFile(std::move(pathToLocalFile)),
                                               zsHandle(nullptr), state(INITIALIZED), localUsed(0), httpDown(0) {
        }
        
        ~Private() = default;
        
    public:
        // by default, the messages are pushed into a queue which can be fetched by calling the client's
        // nextStatusMessage()
        // when defining ZSYNC_STANDALONE, the messages are printed out on stderr directly instead
        // TODO: IDEA: why not allow passing an optional "error=true/false" flag?
        void issueStatusMessage(std::string message) {
#ifndef ZSYNC_STANDALONE
            statusMessages.push_back(message);
#else
            std::cerr << message << std::endl;
#endif
        }

        double calculateProgress() {
            if(!zsHandle)
                return 0;

            long long zgot, ztot;

            zsync_progress(zsHandle, &zgot, &ztot);
            return (zgot / ztot);
        };

        bool setMtime(time_t mtime) {
            struct stat s;
            struct utimbuf u;

            // get access time (shouldn't be modified)
            if (stat(pathToLocalFile.c_str(), &s) != 0) {
                issueStatusMessage("failed to call stat()");
                return false;
            }

            u.actime = s.st_atime;
            u.modtime = mtime;

            if (utime(pathToLocalFile.c_str(), &u) != 0) {
                issueStatusMessage("failed to call utime()");
                return false;
            }

            return true;
        }

        bool readZSyncFile() {
            struct zsync_state *zs;
            std::FILE* f;

            // predefine response to make sure it is in scope after the if statement
            cpr::Response response;

            if (isfile(pathOrUrlToZSyncFile)) {
                f = std::fopen(pathOrUrlToZSyncFile.c_str(), "r");
            } else {
                if (!isUrlAbsolute(pathOrUrlToZSyncFile)) {
                    issueStatusMessage("No such file or directory and not a URL: " + pathOrUrlToZSyncFile);
                    return false;
                }

                response = cpr::Get(pathOrUrlToZSyncFile);

                // might have been redirected to another URL
                // therefore, store final URL of response as referer in case relative URLs will have to be resolved
                referer = response.url;

                // (mis)use response text as in-memory buffer to be able to pass it to the old zsync code
                f = fmemopen((void*) response.text.c_str(), response.text.size(), "r");
            }

            if ((zs = zsync_begin(f)) == nullptr) {
                issueStatusMessage("Failed to parse .zsync file!");
                return false;
            }

            if (fclose(f) != 0) {
                issueStatusMessage("fclose() call failed!");
                return false;
            }

            zsHandle = zs;

            return true;
        }

        // TODO: verify functionality
        bool populatePathToLocalFileFromZSyncFile() {
            auto* p = zsync_filename(zsHandle);

            std::string newPath;

            if (p) {
                std::string zsFilename = p;
                free(p);
                p = nullptr;

                if (zsFilename.find('/') != std::string::npos) {
                    issueStatusMessage("rejected filename specified in " + pathOrUrlToZSyncFile + ", contained path component");
                    return false;
                }

                std::string filenamePrefix = pathPrefix(pathToLocalFile);

                if (zsFilename.substr(0, filenamePrefix.length()) == filenamePrefix)
                    newPath = zsFilename;

                if (!filenamePrefix.empty() && newPath.empty()) {
                    issueStatusMessage("Rejected filename specified in " + pathOrUrlToZSyncFile +
                                       " - prefix " + filenamePrefix + " is different from filename " + zsFilename);
                }
            }

            if (newPath.empty()) {
                newPath = pathPrefix(pathToLocalFile);
                if (newPath.empty())
                    newPath = "zsync-download";
            }

            pathToLocalFile = newPath;
            return true;
        }

        // open a gz-compressed file using zlib
        // the function produces a transparent wrapper for zlib, so that normal file operations (fread, fseek etc.) can
        // be used without noticing any difference
        std::FILE* openGzFile(std::string filePath) {
            // open file using zlib
            auto f = gzopen(filePath.c_str(), "r");

            // if file couldn't be opened, behave like normal fopen()
            if (f == nullptr)
                return nullptr;

            // map file functions to zlib functions
            // NOTE: fopencookie is a Linux-only solution
            // for other platforms, this will have to be adapted (e.g., using funopen() on BSD)
            cookie_io_functions_t iofuncs = {
                reinterpret_cast<__ssize_t (*)(void *, char *, size_t)>(gzread),
                reinterpret_cast<__ssize_t (*)(void *, const char *, size_t)>(gzwrite),
                reinterpret_cast<int (*)(void *, __off64_t *, int)>(gzseek),
                reinterpret_cast<int (*)(void *)>(gzclose)
            };
            return fopencookie(nullptr, "r", iofuncs);
        }

        bool readSeedFile(std::string pathToSeedFile) {
            std::FILE* f;

            // check whether to decompress this file
            if (zsync_hint_decompress(zsHandle) && pathToSeedFile.length() > 3 && endsWith(pathToSeedFile, ".gz")) {
                f = openGzFile(pathToSeedFile);

                if (!f) {
                    issueStatusMessage("Failed to open gzip compressed file " + pathToSeedFile);
                    return false;
                }
            } else {
                f = fopen(pathToSeedFile.c_str(), "r");

                if (!f) {
                    issueStatusMessage("Failed to open file " + pathToSeedFile);
                    return false;
                }
            }

            zsync_submit_source_file(zsHandle, f, false);

            if (fclose(f) != 0) {
                issueStatusMessage("fclose() on file handle failed!");
                return false;
            }

            return true;
        }

        bool verifyDownloadedFile(std::string tempFilePath) {
            auto r = zsync_complete(zsHandle);

            issueStatusMessage("verifying download...");

            switch (r) {
                case -1:
                    issueStatusMessage("aborting, download available in " + tempFilePath);
                    return false;
                case 0:
                    issueStatusMessage("no recognized checksum found");
                    break;
                case 1:
                    issueStatusMessage("checksum matches OK");
                    break;
                default:
                    issueStatusMessage("verification failed: unrecognized error code");
                    return false;
            }

            return true;
        }

        bool makeUrlAbsolute(const std::string& base, const std::string& relative, std::string& final) {
            if (isUrlAbsolute(relative)) {
                final = relative;
                return true;
            }

            if (base.empty())
                return false;

            // check if URL is relative to root, not current "directory"
            if (relative.front() == '/') {
                // find end of scheme
                auto schemePos = base.find("://");

                // if there's no scheme prefix, one can't make the relative URL absolute
                if (schemePos == std::string::npos)
                    return false;

                // search for next slash
                auto pos = base.find('/', schemePos + 1);

                //
                if (pos == std::string::npos)
                    pos = base.length();

                final = base.substr(0, pos - 1) + relative;
            } else {
                // find "directory" in relative path
                auto pos = relative.find_last_of('?');

                if (pos == std::string::npos)
                    pos = relative.find_last_of('#');

                if (pos == std::string::npos)
                    pos = base.length();

                auto slashPos = base.find_last_of('/');
                if (slashPos == std::string::npos)
                    return false;

                final = base.substr(0, slashPos + 1) + relative;
            }

            return true;
        }

        int fetchRemainingBlocksHttp(std::string url, int urlType) {
            // use static const int instead of a define
            static const auto BUFFERSIZE = 8192;

            int ret = 0;

            struct range_fetch* rf;
            unsigned char* buf = nullptr;
            struct zsync_receiver* zr;

            // URL might be relative -- we need an absolute URL to do a fetch
            std::string absoluteUrl;

            if (!makeUrlAbsolute(referer, url, absoluteUrl)) {
                issueStatusMessage("URL '" + url + "' from .zsync file is relative, which cannot be resolved without "
                                   "knowing the URL to the .zsync file (you're most likely trying to use a .zsync "
                                   "file you downloaded from the internet). Without knowing the original URL, it is "
                                   "impossible to resolve the URL from the .zsync file. Please specify a URL with the "
                                   "-u flag, or edit and fix the lines in the .zsync file directly.");
                return -1;
            }

            /* Start a range fetch and a zsync receiver */
            rf = range_fetch_start(absoluteUrl.c_str());
            if (rf == nullptr)
                return -1;

            zr = zsync_begin_receive(zsHandle, urlType);
            if (zr == nullptr) {
                range_fetch_end(rf);
                return -1;
            }

            issueStatusMessage("Downloading from " + absoluteUrl);

            /* Create a read buffer */
            buf = static_cast<unsigned char *>(malloc(BUFFERSIZE));
            if (buf == nullptr) {
                zsync_end_receive(zr);
                range_fetch_end(rf);
                return -1;
            }

            {   /* Get a set of byte ranges that we need to complete the target */
                int nrange;
                auto* zbyterange = zsync_needed_byte_ranges(zsHandle, &nrange, urlType);
                if (zbyterange == nullptr)
                    return 1;
                if (nrange == 0)
                    return 0;

                for(int i = 0; i < 2 * nrange; i++){
                    auto beginbyte = zbyterange[i];
                    i++;
                    auto endbyte = zbyterange[i];
                    off_t single_range[2] = {beginbyte, endbyte};
                    /* And give that to the range fetcher */
                    /* Only one range at a time because Akamai can't handle more than one range per request */
                    range_fetch_addranges(rf, single_range, 1);

                    {
                        int len;
                        off_t zoffset;

                        #ifdef ZSYNC_STANDALONE
                        struct progress p = { 0, 0, 0, 0 };

                        /* Set up progress display to run during the fetch */
                        fputc('\n', stderr);
                        do_progress(&p, (float) calculateProgress() * 100.0f, range_fetch_bytes_down(rf));
                        #endif

                        /* Loop while we're receiving data, until we're done or there is an error */
                        while (!ret
                               && (len = get_range_block(rf, &zoffset, buf, BUFFERSIZE)) > 0) {
                            /* Pass received data to the zsync receiver, which writes it to the
                             * appropriate location in the target file */
                            if (zsync_receive_data(zr, buf, zoffset, len) != 0)
                                ret = 1;

                            #ifdef ZSYNC_STANDALONE
                            /* Maintain progress display */
                            do_progress(&p, (float) calculateProgress() * 100.0f,
                                        range_fetch_bytes_down(rf));
                            #endif

                            // Needed in case next call returns len=0 and we need to signal where the EOF was.
                            zoffset += len;
                        }

                        /* If error, we need to flag that to our caller */
                        if (len < 0) {
                            fprintf(stdout, "%d returned\n", len);
                            ret = -1;
                        }
                        else{    /* Else, let the zsync receiver know that we're at EOF; there
                         *could be data in its buffer that it can use or needs to process */
                            zsync_receive_data(zr, nullptr, zoffset, 0);
                        }

                        #ifdef ZSYNC_STANDALONE
                        end_progress(&p, zsync_status(zsHandle) >= 2 ? 2 : len == 0 ? 1 : 0);
                        #endif
                    }

                }

                free(zbyterange);

            }

            /* Clean up */
            free(buf);
            httpDown += range_fetch_bytes_down(rf);
            zsync_end_receive(zr);
            range_fetch_end(rf);
            return ret;
        }

        bool fetchRemainingBlocks() {
            int n = 0, utype = 0;
            const auto* url = zsync_get_urls(zsHandle, &n, &utype);

            // copy value for later use
            int okUrls = n;

            if (!url) {
                issueStatusMessage("no URLs available from zsync?");
                return false;
            }

            int* status;
            status = static_cast<int*>(calloc(n, sizeof *status));

            while (zsync_status(zsHandle) < 2 && okUrls) {
                int attempt = rand() % n;

                if (!status[attempt]) {
                    const std::string tryurl = url[attempt];

                    auto result = fetchRemainingBlocksHttp(tryurl, utype);

                    if (result != 0) {
                        issueStatusMessage("failed to retrieve from " + tryurl + ", status " + std::to_string(result));
                        return false;
                    }
                }
            }

            return true;
        }

        bool run() {
            // exit if run has been called before
            if (state != INITIALIZED) {
                issueStatusMessage("Could not start client: running/done already!");
                return false;
            }

            state = RUNNING;

            /**** step 1: read .zsync file ****/
            if (!readZSyncFile()) {
                state = DONE;
                return false;
            }

            // check whether path was explicitly passed
            // otherwise, use the one defined in the .zsync file
            if (pathToLocalFile.empty())
                if (!populatePathToLocalFileFromZSyncFile()) {
                    state = DONE;
                    return false;
                }

            // calculate path to temporary file
            auto tempFilePath = pathToLocalFile + ".part";

            {
                /**** step 2: read in available data from seed files and fill in existing data into target file ****/
                if (isfile(pathToLocalFile)) {
                    issueStatusMessage(pathToLocalFile + " found, using as seed file");
                    seedFiles.insert(pathToLocalFile);
                }

                // if the temporary file exists, it's likely left over from a previous attempt that got interrupted
                // due to how zsync works, one can't just "resume" from this file like a normal HTTP client would do (also,
                // the server file might have changed in the meantime), but one can certainly make use of it as a seed file
                if (isfile(tempFilePath)) {
                    issueStatusMessage(tempFilePath + " found, using as seed file");
                    seedFiles.insert(tempFilePath);
                }

                // try to make use of any seed file provided
                for (const auto &seedFile : seedFiles) {
                    // exit loop if file is complete
                    if (zsync_status(zsHandle) >= 2) {
                        state = DONE;
                        break;
                    }

                    issueStatusMessage("Reading seed file: " + seedFile);
                    if(!readSeedFile(seedFile)) {
                        state = DONE;
                        return false;
                    }
                }

                // first, store current value
                zsync_progress(zsHandle, &localUsed, nullptr);
                // now, show how far that got us
                issueStatusMessage("Usable data from seed files: " + std::to_string(calculateProgress() * 100.0f) + "%");
            }

            // libzsync has been writing to a randomely-named temp file so far -
            // because we didn't want to overwrite the .part from previous runs. Now
            // we've read any previous .part, we can replace it with our new
            // in-progress run (which should be a superset of the old .part - unless
            // the content changed, in which case it still contains anything relevant
            // from the old .part).
            issueStatusMessage("Renaming temp file");
             if (zsync_rename_file(zsHandle, tempFilePath.c_str()) != 0) {
                 state = DONE;
                 return false;
             }

            // step 3: fetch remaining blocks via the URLs from the .zsync
            issueStatusMessage("Fetching remaining blocks");
            if(!fetchRemainingBlocks()) {
                state = DONE;
                return false;
            }

            // step 4: verify download
            issueStatusMessage("Verifying downloaded file");
            if(!verifyDownloadedFile(tempFilePath)) {
                state = DONE;
                return false;
            }

            // Get any mtime that we is suggested to set for the file, and then shut
            // down the zsync_state as we are done on the file transfer. Getting the
            // current name of the file at the same time.
            auto mtime = zsync_mtime(zsHandle);
            tempFilePath = zsync_end(zsHandle);

            // step 5: replace original file by completed .part file
            if (!pathToLocalFile.empty()) {
                bool ok = true;
                std::string oldFileBackup = pathToLocalFile + ".zs-old";

                if (isfile(pathToLocalFile)) {
                    // remove previous update
                    // error check unnecessary -- failures will be checked for in the next part
                    unlink(oldFileBackup.c_str());

                    if (link(pathToLocalFile.c_str(), oldFileBackup.c_str()) != 0 &&
                        (errno != EPERM || rename(pathToLocalFile.c_str(), oldFileBackup.c_str()) != 0)) {
                        issueStatusMessage("unable to back up old file " + oldFileBackup +
                                           " - completed download left in " + tempFilePath);
                        // prevent overwrite of old file below
                        ok = false;
                    }
                }

                if (ok) {
                    if (rename(tempFilePath.c_str(), pathToLocalFile.c_str())) {
                        if (mtime != -1)
                            setMtime(mtime);
                    } else {
                        issueStatusMessage("unable to back up old file " + oldFileBackup +
                                           " - completed download left in " + tempFilePath);
                    }
                }
            } else {
                issueStatusMessage("No filename specified for download - completed download left in " + tempFilePath);
            }

            // final stats and cleanup
            issueStatusMessage("used " + std::to_string(localUsed) + " local, fetched " + std::to_string(httpDown));
            state = DONE;
            return true;
        }
    };

    ZSyncClient::ZSyncClient(const std::string pathOrUrlToZSyncFile, const std::string pathToLocalFile) {
        d = new Private(pathOrUrlToZSyncFile, pathToLocalFile);
    }
    ZSyncClient::~ZSyncClient() {
        delete d;
    };

#ifndef ZSYNC_STANDALONE
    bool ZSyncClient::nextStatusMessage(std::string &message) {
        if (!d->statusMessages.empty()) {
            message = d->statusMessages.front();
            d->statusMessages.pop_front();
            return true;
        }

        return false;
    }
#endif

    double ZSyncClient::progress() {
        return d->calculateProgress();
    }

    bool ZSyncClient::run() {
        auto result = d->run();

        // make sure to change state to DONE, regardless of whatever happens in d->run()
        d->state = d->DONE;

        return result;
    }
}
