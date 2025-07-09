#include "calculators/Md5Calculator.hpp"
#include <catch2/catch_all.hpp>

/**
 * @test MD5 checksum calculation
 * Expected results are from http://www.sha1-online.com/
 */
TEST_CASE("MD5 Checksum Calculation", "[Md5Calculator]") {
    Md5Calculator md5Calculator;

    SECTION("Empty string") {
        std::string input = "";
        std::string expected = "d41d8cd98f00b204e9800998ecf8427e";
        REQUIRE(md5Calculator.calculate(input) == expected);
    }

    SECTION("Single character") {
        std::string input = "a";
        std::string expected = "0cc175b9c0f1b6a831c399e269772661";
        REQUIRE(md5Calculator.calculate(input) == expected);
    }

    SECTION("Short string") {
        std::string input = "hello";
        std::string expected = "5d41402abc4b2a76b9719d911017c592";
        REQUIRE(md5Calculator.calculate(input) == expected);
    }

    SECTION("Longer string") {
        std::string input = "The quick brown fox jumps over the lazy dog";
        std::string expected = "9e107d9d372bb6826bd81d3542a419d6"; 
        REQUIRE(md5Calculator.calculate(input) == expected);
    }
}

