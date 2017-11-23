// system headers
#include <arpa/inet.h>
#include <iomanip>
#include <iterator>
#include <sstream>
#include <unordered_map>

// library headers
extern "C" {
    #include <rcksum.h>
    #include <sha1.h>
}

// local headers
#include "config.h"
#include "zsmake.h"
#include "zsutil.h"

namespace zsync2 {
    class ZSyncFileMaker::Private {
    private:
        typedef std::vector<char> buffer_t;
        typedef std::map<std::string, std::string> headerFields_t;

    public:
        // path to file
        std::string path;
        std::string zSyncFilePath;

        // header fields' values
        std::string fileName;
        std::string url;

        std::string fileSHA1Hash;
        uint32_t blockSize;

        long length;
        int checksumLength;
        int rSumLength;
        int seqMatches;

        buffer_t blockSums;

        headerFields_t customHeaderFields;

    public:
        explicit Private(const std::string& path) : path(path),
                                                    length(0),
                                                    checksumLength(0),
                                                    blockSize(0),
                                                    rSumLength(0),
                                                    seqMatches(0)
        {
            // make sure to use the filename only
            size_t slashPos;
            if ((slashPos = path.find_last_of('/')) == std::string::npos)
                fileName = path;
            else
                fileName = path.substr(slashPos + 1, path.length());

            // should be created in the current working directory unless specified otherwise
            zSyncFilePath = fileName + ".zsync";

            // initialize URL with file name
            // though, it should be set to the real URL
            url = fileName;
        }

        ~Private() = default;

    private:
        bool insertMTimeHeader(headerFields_t& headerFields) {
            auto mtime = readMtime(path);

            if(mtime != -1) {
                char buffer[32];
                struct tm mtime_tm;

                if(gmtime_r(&mtime, &mtime_tm) != nullptr) {
                    if(strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %z", &mtime_tm) <= 0)
                        return false;

                    headerFields.insert(std::make_pair("MTime", buffer));
                    return true;
                }
            }

            return false;
        }

    public:
        bool writeBlockSums(const buffer_t& buffer, size_t bytesRead) {
            buffer_t checksum(CHECKSUM_SIZE);

            // add padding to last block
            if (bytesRead < blockSize)
                std::fill(checksum.begin() + bytesRead, checksum.end(), 0);

            // sometimes, the inconsistent use of char and unsigned char within the old zsync code base can get
            // annoying...
            auto r = rcksum_calc_rsum_block(reinterpret_cast<const unsigned char*>(buffer.data()), blockSize);
            rcksum_calc_checksum(
                reinterpret_cast<unsigned char*>(checksum.data()),
                reinterpret_cast<const unsigned char*>(buffer.data()),
                blockSize
            );
            r.a = htons(r.a);
            r.b = htons(r.b);

            // insert raw bytes into
            auto oldSize = blockSums.size();
            blockSums.resize(oldSize + sizeof(r));
            memcpy(&blockSums[oldSize], &r, sizeof(r));

            std::copy(checksum.begin(), checksum.end(), std::back_inserter(blockSums));

            return true;
        }

        bool readStreamWriteBlockSums(std::ifstream& inFile, SHA1_CTX& sha1Ctx) {
            buffer_t buffer(blockSize);

            while (inFile.read(buffer.data(), buffer.size())) {
                auto bytesRead = (size_t) inFile.gcount();

                if (bytesRead > 0) {
                    SHA1Update(&sha1Ctx, reinterpret_cast<const uint8_t*>(buffer.data()), bytesRead);

                    writeBlockSums(buffer, bytesRead);
                    length += bytesRead;
                } else {
                    // TODO: check for stream error and display error message
                    return false;
                }
            }

            return true;
        }

        bool calculateBlockSums() {
            // read the input file and construct the checksum of the whole file, and the per-block checksums

            // create SHA1 context
            SHA1_CTX sha1Ctx;
            SHA1Init(&sha1Ctx);

            std::ifstream ifs(path);
            if (!ifs)
                return false;

            if (blockSize == 0)
                blockSize = (ifs.tellg() < 100000000) ? 2048 : 4096;

            if (!readStreamWriteBlockSums(ifs, sha1Ctx))
                return false;


            // decide how long a rsum hash and checksum hash per block we need for this file
            seqMatches = (length > blockSize) ? 2 : 1;
            rSumLength = static_cast<int>(ceil(((log(length) + log(blockSize)) / log(2) - 8.6) / seqMatches / 8));

            // apply limits
            if (rSumLength > 4)
                rSumLength = 4;
            else if (rSumLength < 2)
                rSumLength = 2;

            // calculate two different checksum lengths and pick the maximum
            checksumLength = static_cast<int>(ceil((20 + (log(length) + log(1 + length / blockSize)) / log(2)) / seqMatches / 8));
            {
                auto checksumLength2 = static_cast<int>((7.9 + (20 + log(1 + length / blockSize) / log(2))) / 8);
                if (checksumLength < checksumLength2)
                    checksumLength = checksumLength2;
            }

            // calculate SHA-1 hash sum
            {
                std::vector<uint8_t> digest(SHA1_DIGEST_LENGTH);
                SHA1Final(digest.data(), &sha1Ctx);

                std::ostringstream oss;
                for (auto& i : digest)
                    oss << std::setfill('0') << std::setw(2) << std::hex << (int) i;

                fileSHA1Hash = oss.str();
            }

            return true;
        }

        bool dump(std::string& data) {
            // prepare file header
            // create copy of customHeaderFields and insert default headers
            auto headerFields = customHeaderFields;

            headerFields["zsync"] = VERSION;
            headerFields["Filename"] = fileName;

            if (!insertMTimeHeader(headerFields))
                return false;

            headerFields["Blocksize"] = std::to_string(blockSize);
            headerFields["Length"] = std::to_string(length);

            headerFields["URL"] = url;
            headerFields["SHA-1"] = fileSHA1Hash;

            std::ostringstream hashLengths;
            hashLengths << seqMatches << "," << rSumLength << "," << checksumLength;
            headerFields["Hash-Lengths"] = hashLengths.str();

            // now, create .zsync file
            std::ostringstream oss;

            constexpr char endl[] = "\n";

            // write header
            for (const auto& pair : headerFields) {
                oss << pair.first << ": " << pair.second << endl;
            }

            // insert separator to mark end of header
            oss << endl;

            // copy block hashes
            {
                // create buffer
                buffer_t buffer(20);
                std::fill(buffer.begin(), buffer.end(), 0);

                auto readChunk = [&buffer, this]() {
                    size_t length = 0;

                    if (blockSums.empty())
                        return length;

                    length = buffer.size();

                    if (blockSums.size() < buffer.size()) {
                        length = blockSums.size();
                        std::fill(buffer.begin() + length, buffer.end(), 0);
                    }

                    std::copy(blockSums.begin(), blockSums.begin() + length, buffer.begin());
                    blockSums.erase(blockSums.begin(), blockSums.begin() + length);

                    return length;
                };

                while (readChunk() > 0) {
                    std::copy(buffer.begin() + 4 - rSumLength, buffer.begin() + 4, std::ostream_iterator<char>(oss));
                    std::copy(buffer.begin() + 4, buffer.begin() + 4 + checksumLength, std::ostream_iterator<char>(oss));
                }
            }

            data = oss.str();
            return true;
        }
    };

    ZSyncFileMaker::ZSyncFileMaker(const std::string& filename) {
        d = new Private(filename);
    }

    ZSyncFileMaker::~ZSyncFileMaker() {
        delete d;
    }

    bool ZSyncFileMaker::calculateBlockSums() {
        return d->calculateBlockSums();
    }

    bool ZSyncFileMaker::dump(std::string& data) {
        return d->dump(data);
    }

    void ZSyncFileMaker::setBlockSize(uint32_t blockSize) {
        d->blockSize = blockSize;
    }

    bool ZSyncFileMaker::saveZSyncFile(std::string outFilePath) {
        if (outFilePath.empty())
            outFilePath = d->zSyncFilePath;

        std::ofstream ofs(outFilePath);

        if (!ofs)
            return false;

        std::string data;
        if (!dump(data))
            return false;

        ofs << data;

        return true;
    }
}
