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
        ZSyncClient(std::string urlOrPathToZsyncFile, std::string pathToLocalFile = "");
        ~ZSyncClient();

    public:
        // synchronizes a local file with a remote one based on the information in a zsync file given by URL
        bool run();
        // returns progress (double between 0 and 1) that can be used to display progress bars etc.
        double progress();
        // fetch next available status message from the application
        // returns true if a message is available and sets passed string, otherwise returns false
        bool nextStatusMessage(std::string& message);
    };
}
