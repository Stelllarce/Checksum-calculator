#include "calculators/SHA256Calculator.hpp"
#include <catch2/catch_all.hpp>
/**
 * @test SHA256 checksum calculation
 * Expected results are from http://www.sha1-online.com/
 */
TEST_CASE("SHA256 Checksum Calculation", "[SHA256Calculator]") {
    SHA256Calculator sha256Calculator;

    SECTION("Empty string") {
        std::string input = "";
        std::string expected = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
        REQUIRE(sha256Calculator.calculate(input) == expected);
    }

    SECTION("Single character") {
        std::string input = "a";
        std::string expected = "ca978112ca1bbdcafac231b39a23dc4da786eff8147c4e72b9807785afee48bb";
        REQUIRE(sha256Calculator.calculate(input) == expected);
    }

    SECTION("Short string") {
        std::string input = "hello";
        std::string expected = "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824";
        REQUIRE(sha256Calculator.calculate(input) == expected);
    }

    SECTION("Longer string") {
        std::string input = "The quick brown fox jumps over the lazy dog";
        std::string expected = "d7a8fbb307d7809469ca9abcb0082e4f8d5651e46d3cdb762d02d0bf37c9e592";
        REQUIRE(sha256Calculator.calculate(input) == expected);
    }
}