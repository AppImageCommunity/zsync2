// gtest includes
#include <gtest/gtest.h>

// system includes
#include <string>

// local includes
#include "zsutil.h"

using namespace std;
using namespace zsync2;

namespace {
    TEST(isUrlAbsolute, TestRelativeUrl) {
        EXPECT_FALSE(isUrlAbsolute("test/"));
        EXPECT_FALSE(isUrlAbsolute("/test/"));
        EXPECT_FALSE(isUrlAbsolute("../test"));
    }

    TEST(isUrlAbsolute, TestEmptyString) {
        EXPECT_FALSE(isUrlAbsolute(""));
    }

    TEST(isUrlAbsolute, TestAbsoluteUrl) {
        EXPECT_TRUE(isUrlAbsolute("http://test.123/test"));
        EXPECT_TRUE(isUrlAbsolute("https://test.123/test"));
        EXPECT_TRUE(isUrlAbsolute("ftp://test.123/test"));
    }

    TEST(isUrlAbsolute, TestBrokenUrl) {
        EXPECT_FALSE(isUrlAbsolute("://test.123/test"));
    }
}

namespace {
    TEST(ltrim, TestEmptyString) {
        string empty = "";
        ltrim(empty);
        EXPECT_STREQ(empty.c_str() ,"");
    }

    TEST(ltrim, TestAlreadyTrimmedString) {
        string test = "test";
        ltrim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }

    TEST(ltrim, TestRTrimmedString) {
        string test = "  test";
        ltrim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }

    TEST(ltrim, TestNonTrimmedString) {
        string test = "  test  ";
        ltrim(test);
        EXPECT_STREQ(test.c_str() ,"test  ");
    }
}

namespace {
    TEST(rtrim, TestEmptyString) {
        string empty = "";
        rtrim(empty);
        EXPECT_STREQ(empty.c_str() ,"");
    }

    TEST(rtrim, TestAlreadyTrimmedString) {
        string test = "test";
        rtrim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }

    TEST(rtrim, TestLTrimmedString) {
        string test = "test  ";
        rtrim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }

    TEST(rtrim, TestNonTrimmedString) {
        string test = "  test  ";
        rtrim(test);
        EXPECT_STREQ(test.c_str() ,"  test");
    }
}

namespace {
    TEST(trim, TestEmptyString) {
        string empty = "";
        trim(empty);
        EXPECT_STREQ(empty.c_str() ,"");
    }

    TEST(trim, TestAlreadyTrimmedString) {
        string test = "test";
        trim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }

    TEST(trim, TestLTrimmedString) {
        string test = "test  ";
        trim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }

    TEST(trim, TestRTrimmedString) {
        string test = "test  ";
        trim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }

    TEST(trim, TestNonTrimmedString) {
        string test = "  test  ";
        trim(test);
        EXPECT_STREQ(test.c_str() ,"test");
    }
}

namespace {
    TEST(pathPrefix, TestEmptyString) {
        EXPECT_STREQ(pathPrefix("").c_str(), "");
    }

    TEST(pathPrefix, TestSameDirectory) {
        EXPECT_STREQ(pathPrefix("test").c_str(), "test");
        EXPECT_STREQ(pathPrefix("test.").c_str(), "test");
        EXPECT_STREQ(pathPrefix("TEST").c_str(), "TEST");
        EXPECT_STREQ(pathPrefix("TEST.TEST").c_str(), "TEST");
    }

    TEST(pathPrefix, TestOtherDirectory) {
        EXPECT_STREQ(pathPrefix("test/test").c_str(), "test");
        EXPECT_STREQ(pathPrefix("test/test.").c_str(), "test");
        EXPECT_STREQ(pathPrefix("TEST/").c_str(), "");
        EXPECT_STREQ(pathPrefix("TEST/TEST").c_str(), "TEST");
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
