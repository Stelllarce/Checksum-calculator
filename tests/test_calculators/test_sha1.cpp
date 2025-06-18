#include "calculators/SHA1Calculator.hpp"
#include <catch2/catch_all.hpp>

/**
 * @test SHA1 checksum calculation
 * Expected results are from http://www.sha1-online.com/
 */
TEST_CASE("SHA1 Checksum Calculation", "[SHA1Calculator]") {
    SHA1Calculator sha1Calculator;

    SECTION("Empty string") {
        std::string input = "";
        std::string expected = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
        REQUIRE(sha1Calculator.calculate(input) == expected);
    }

    SECTION("Single character") {
        std::string input = "a";
        std::string expected = "86f7e437faa5a7fce15d1ddcb9eaeaea377667b8";
        REQUIRE(sha1Calculator.calculate(input) == expected);
    }

    SECTION("Short string") {
        std::string input = "hello";
        std::string expected = "aaf4c61ddcc5e8a2dabede0f3b482cd9aea9434d";
        REQUIRE(sha1Calculator.calculate(input) == expected);
    }

    SECTION("Longer string") {
        std::string input = "The quick brown fox jumps over the lazy dog";
        std::string expected = "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12";
        REQUIRE(sha1Calculator.calculate(input) == expected);
    }
}