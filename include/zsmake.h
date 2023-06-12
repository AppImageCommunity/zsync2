#pragma once

#include <functional>
#include <map>
#include <string>
#include <cstdint>

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
        bool saveZSyncFile(std::string outFilePath = "");

        // sets an absolute or relative URL to the target file, seen from the future location of the .zsync file on
        // the server
        void setUrl(const std::string& url);

        // will be called for every log message issued by the code
        void setLogMessageCallback(std::function<void(std::string)> callback);

        // add custom header field
        // returns true when there is no header with such a key yet, otherwise overwrites the existing value and
        // returns false
        // getHeaderFields() can be used to check whether a header with a given key exists already
        // beware that this function does not check whether any essential headers are set, these will be silently
        // overwritten within dump()
        void addCustomHeaderField(const std::string& key, const std::string& value);

        // returns all custom headers set by the user
        std::map<std::string, std::string> getCustomHeaderFields();
    };
}
