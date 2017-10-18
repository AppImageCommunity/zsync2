// system headers
#include <iostream>
#include <string>

// library headers
#include <args.hxx>

// local headers
#include "zsclient.h"

using namespace std;

int main(const int argc, const char** argv) {
    args::ArgumentParser parser("zsync2 -- the probably easiest efficient way to update files",
                                "Brand new C++11 frontend to zsync.");
    args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});
    args::Positional<std::string> pathOrUrl(parser, "Path or URL", "Path or URL to .zsync(2) file");

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        cerr << parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    if (!pathOrUrl) {
        cerr << parser;
        return 0;
    }

    cout << pathOrUrl.Get() << endl;
    zsync2::ZSyncClient client(pathOrUrl.Get());

    if(!client.run())
        return 1;

    return 0;
}
