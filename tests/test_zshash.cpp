// gtest includes
#include <gtest/gtest.h>

// system includes
#include <string>

// local includes
#include "zshash.h"

using namespace std;
using namespace zsync2;

namespace {
    TEST(ZSyncHash, TestMD5) {
        auto makeHash = [](const std::string& str) {
            return ZSyncHash<GCRY_MD_MD5>(str).getHash();
        };

        // these hashes were calculated independently with the coreutils helpers
        EXPECT_EQ(makeHash("abc"), "900150983cd24fb0d6963f7d28e17f72");
        EXPECT_EQ(makeHash("def"), "4ed9407630eb1000c0f6b63842defa7d");
        EXPECT_EQ(makeHash("ghi"), "826bbc5d0522f5f20a1da4b60fa8c871");
        EXPECT_EQ(makeHash(""), "d41d8cd98f00b204e9800998ecf8427e");
    }

    TEST(ZSyncHash, TestSHA256) {
        auto makeHash = [](const std::string& str) {
            return ZSyncHash<GCRY_MD_SHA256>(str).getHash();
        };

        // these hashes were calculated independently with the coreutils helpers
        EXPECT_EQ(makeHash("abc"), "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
        EXPECT_EQ(makeHash("def"), "cb8379ac2098aa165029e3938a51da0bcecfc008fd6795f401178647f96c5b34");
        EXPECT_EQ(makeHash("ghi"), "50ae61e841fac4e8f9e40baf2ad36ec868922ea48368c18f9535e47db56dd7fb");
        EXPECT_EQ(makeHash(""), "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
