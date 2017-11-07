/*
 * zsync2
 * ======
 *
 * zsutil.h: Header-only utility library.
 */

#pragma once

// system headers
#include <algorithm>
#include <fstream>
#include <time.h>
#include <sys/stat.h>

namespace zsync2 {
    static inline bool ltrim(std::string &s, char to_trim = ' ') {
        // TODO: find more efficient way to check whether elements have been removed
        size_t initialLength = s.length();
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        return s.length() < initialLength;
    }

    static inline bool rtrim(std::string &s, char to_trim = ' ') {
        // TODO: find more efficient way to check whether elements have been removed
        auto initialLength = s.length();
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), s.end());
        return s.length() < initialLength;
    }

    static inline bool trim(std::string &s, char to_trim = ' ') {
        // returns true if either modifies s
        auto ltrim_result = ltrim(s, to_trim);
        return rtrim(s, to_trim) && ltrim_result;
    }

    static inline bool isfile(std::string path) {
        std::ifstream ifs(path);
        return (bool) ifs && ifs.good();
    }

    static inline time_t mtime(std::string path) {
        struct stat fstat;

        if (stat(path.c_str(), &fstat) == 0) {
            return fstat.st_mtim.tv_sec;
        }

        return -1;
    }

    static inline std::string toLower(std::string string) {
        std::transform(string.begin(), string.end(), string.begin(), ::tolower);
        return string;
    }

    static inline bool isUrlAbsolute(const std::string url) {
        static const char special[] = { ":/?" };

        auto firstSpecial = url.find_first_of(special);

        // if none of those characters has been found, it can't be an absolute URL
        if (firstSpecial == std::string::npos)
            return false;

        // must not be first character
        if (firstSpecial <= 0)
            return false;

        const auto& firstSpecialValue = url.c_str()[firstSpecial];

        if (firstSpecialValue == ':')
            return true;

        return false;
    }

    // returns directory prefix
    static inline std::string pathPrefix(std::string path) {
        // remove everything before last /
        auto lastSlash = path.find_last_of('/');
        if (lastSlash != std::string::npos) {
            lastSlash++;
            path = path.substr(lastSlash, path.length());
        }

        // find first non-alphanumeric character
        auto firstNonAlphaNum = std::distance(path.begin(), std::find_if_not(path.begin(), path.end(), isalnum));

        // return alphanumeric part
        return path.substr(0, firstNonAlphaNum);
    }

    static inline bool endsWith(std::string const& value, std::string const& ending)
    {
        if (ending.size() > value.size())
            return false;

        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    }

    // resolves a redirection
    // in case of success, sets redirectedUrl and returns true, false otherwise
    static bool resolveRedirections(const std::string& absoluteUrl, std::string& redirectedUrl) {
        auto response = cpr::Head(absoluteUrl);

        // check for proper response code
        // 4xx and 5xx responses can be considered okay for a redirection resolver, as they're valid responses
        // however, 3xx responses shouldn't be seen here any more, as CPR should have followed any possible
        // redirection already
        if (response.status_code >= 300 && response.status_code < 400)
            return false;

        redirectedUrl = response.url;
        return true;
    }
}
