#define CATCH_CONFIG_MAIN
#include "../src/common/utils.h"
#include <catch2/catch.hpp>

TEST_CASE("Test getFileDigest", "[getFileDigest]") {
    REQUIRE(getFileDigest("test/hashTestFile.txt") == getFileDigest("test/hashTestFile.txt"));
    REQUIRE(digest_to_string(getFileDigest("test/hashTestFile.txt")) == "33221ee02b7aea8");
}