// system headers
#include <iostream>

// library headers
#include <args.hxx>

// local headers
#include "zsmake.h"
#include "zsutil.h"

using namespace std;
using namespace zsync2;

int main(int argc, char** argv) {
    args::ArgumentParser parser(
        "Creates a .zsync file for an existing file.",
        "to be filled in with something useful."
    );

    args::HelpFlag help(parser, "help", "Displays this help text", {'h', "help"});

    args::ValueFlag<string> updateUrl(parser, "url",
        "URL the .zsync file should point to (may be relative)",
        {'u', "url"}
    );

    args::ValueFlag<uint32_t> blockSize(parser, "blocksize",
        "Specify the blocksize to the underlying rsync algorithm. A smaller blocksize may be more  efficient "
        "for files where there are likely to be lots of small, scattered changes between downloads; "
        "a larger blocksize is more efficient for files with fewer or less scattered changes. This blocksize must be "
        "a power of two. If not specified, zsyncmake chooses one which it thinks is best for this file (currently "
        "either 2048 or 4096 depending on file size) - so normally you should not need to override the default.",
        {'b', "blocksize"});

    args::Positional<string> fileName(parser, "filename",
        "Name of the file which a .zsync file should be generated for."
    );

    try {
        parser.ParseCLI(argc, (const char **) argv);
    } catch (args::Help) {
        cerr << parser;
        return 0;
    } catch (args::ParseError e) {
        cerr << e.what() << endl << endl;
        cerr << parser;
        return 1;
    }

    if (!fileName) {
        cerr << "Error: need to specify filename!" << endl << endl;
        cerr << parser;
        return 1;
    }

    ZSyncFileMaker maker(fileName.Get());

    // validate parameters
    if(updateUrl) {
        maker.setUrl(updateUrl.Get());
    }

    if(!isfile(fileName.Get())) {
        cerr << "Error: no such file or directory: " << fileName.Get() << endl;
        return 1;
    }

    if (blockSize)
        maker.setBlockSize(blockSize.Get());

    if (!maker.calculateBlockSums()) {
        cerr << "Failed to calculate block sums!" << endl;
        return 2;
    }

    if (!maker.saveZSyncFile())
        return 1;
}
