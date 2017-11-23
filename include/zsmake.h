#pragma once

#include <string>
#include <functional>

namespace zsync2 {
    class ZSyncFileMaker {
    private:
        // opaque private class
        class Private;
        Private* d;

    public:
        // default constructor
        explicit ZSyncFileMaker(const std::string& filename);
        ~ZSyncFileMaker();

    public:
        // generate file as a string
        bool dump(std::string& data);

        // set blocksize
        void setBlockSize(uint32_t blockSize);

        // calculate checksums for blocks in
        bool calculateBlockSums();

        // create file and store data in it
        bool saveFile();

        // will be called for every log message issued by the code
        bool setLogMessageCallback(std::function<void(std::string)> callback);
    };
}
