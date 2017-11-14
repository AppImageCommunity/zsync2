#pragma once

// system includes
#include <string>

namespace zsync2 {
    class ZSyncClient {
    private:
        // opaque private class
        class Private;
        Private *d;

    public:
        explicit ZSyncClient(std::string urlOrPathToZsyncFile, std::string pathToLocalFile = "", bool overwrite = true);
        ~ZSyncClient();

    public:
        // synchronizes a local file with a remote one based on the information in a zsync file given by URL
        bool run();

        // returns progress (double between 0 and 1) that can be used to display progress bars etc.
        double progress();

        // fetch next available status message from the application
        // returns true if a message is available and sets passed string, otherwise returns false
        bool nextStatusMessage(std::string& message);

        // checks whether a new version is available on the server, i.e., an update is necessary
        // there's several methods available:
        // - method 0: hash local file using SHA-1, download meta information from server, compare to server-side SHA1
        //   value (safest method, but also slowest)
        // - method 1: download meta information from server, compare modification time (mtime) to local file's mtim
        //   this method is less reliable, as the user could call touch etc. on the file. However, for app stores
        //   managing the files, this method should be similarly reliable
        // (other methods might follow)
        // updateAvailable parameter is set to true if changes are available (or the file needs to be downloaded in
        // total), false otherwise
        // returns false if update check fails, otherwise true
        bool checkForChanges(bool& updateAvailable, unsigned int method = 0);

        // add seed file that should be searched for usable data during the download process
        void addSeedFile(const std::string& path);

        // set path to path of new file created by the process
        // returns true when the value is available, false in case the value is not there or there is an error
        bool pathToNewFile(std::string& path);

        // set directory from where relative filenames should be resolved
        // can be set only right after initialization
        bool setCwd(const std::string& path);
    };
}
