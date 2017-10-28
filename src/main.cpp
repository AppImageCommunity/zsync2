// system headers
#include <iostream>
#include <cstring>

// library headers
#include <args.hxx>

// local headers
#include "zsclient.h"

using namespace std;

int main(const int argc, const char** argv) {
    args::ArgumentParser parser("zsync2 -- the probably easiest efficient way to update files",
                                "Brand new C++11 frontend to zsync.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlagList<std::string> seedFiles(parser, "seed file", "Use data from this file during update process. "
                                                                    "Can be specified more than once.", {'I'});
    args::Flag checkForChanges(parser, "checkForChanges", "Check for changes on the server. Exits with code 1 if the "
                                                          "file changed on the server, otherwise 0.", {'j'});
    args::Flag verbose(parser, "verbose", "Switch on verbose mode", {'v'});
    args::Positional<std::string> pathOrUrl(parser, "path or URL", "Path or URL to .zsync(2) file");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help&) {
        cerr << parser;
        return 0;
    } catch (args::ParseError& e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    if (verbose) {
        putenv(strdup("CURLOPT_VERBOSE=1"));
    }

    if (!pathOrUrl) {
        cerr << parser;
        return 2;
    }

    cout << pathOrUrl.Get() << endl;
    zsync2::ZSyncClient client(pathOrUrl.Get());

    if(seedFiles) {
        for (const auto &seedFile : seedFiles.Get()) {
            client.addSeedFile(seedFile);
        }
    }

    if(!client.run())
        return 1;

    return 0;
}
