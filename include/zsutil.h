/*
 * zsync2
 * ======
 *
 * zsutil.h: utility library.
 */

#pragma once

// system headers
#include <string>
#include <vector>

namespace zsync2 {
    bool ltrim(std::string &s, char to_trim = ' ');;

    bool rtrim(std::string &s, char to_trim = ' ');

    bool trim(std::string &s, char to_trim = ' ');

    bool isfile(const std::string& path);

    time_t readMtime(const std::string& path);

    std::string toLower(std::string string);

    bool isUrlAbsolute(const std::string& url);

    // returns directory prefix
    std::string pathPrefix(std::string path);

    bool endsWith(std::string const& value, std::string const& ending);

    // resolves a redirection
    // in case of success, sets redirectedUrl and returns true, false otherwise
    bool resolveRedirections(const std::string& absoluteUrl, std::string& redirectedUrl);

    int32_t getPerms(const std::string& path, mode_t& permissions);;

    std::vector<std::string> split(const std::string& s, char delim = ' ');

    std::string base64Decode(const std::string& in);

    std::string bytesToHex(unsigned char *data, int len);
}
