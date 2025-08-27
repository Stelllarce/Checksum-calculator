#include <catch2/catch_all.hpp>
#include <sstream>
#include <stdexcept>
#include "file-system-composite/File.hpp"
#include "file-system-composite/Directory.hpp"

TEST_CASE("File Constructor", "[File]") {
    SECTION("Valid construction with owner") {
        Directory root_dir("test_dir");
        File test_file("test.txt", &root_dir);
        
        REQUIRE(test_file.getName() == "test.txt");
        REQUIRE(test_file.getOwner() == &root_dir);
    }
    
    SECTION("Constructor throws when owner is null") {
        REQUIRE_THROWS_AS(File("test.txt", nullptr), std::logic_error);
    }
}

TEST_CASE("File getName", "[File]") {
    Directory root_dir("test_dir");
    
    SECTION("Simple filename") {
        File test_file("document.pdf", &root_dir);
        REQUIRE(test_file.getName() == "document.pdf");
    }
    
    SECTION("Filename with extension") {
        File test_file("archive.tar.gz", &root_dir);
        REQUIRE(test_file.getName() == "archive.tar.gz");
    }
    
    SECTION("Filename without extension") {
        File test_file("README", &root_dir);
        REQUIRE(test_file.getName() == "README");
    }
}

TEST_CASE("File getSize and setSize", "[File]") {
    Directory root_dir("test_dir");
    File test_file("test.txt", &root_dir);
    
    SECTION("Initial size is zero") {
        REQUIRE(test_file.getSize() == 0);
    }
    
    SECTION("Set and get cached size") {
        REQUIRE(test_file.setSize(1024) == true);
        REQUIRE(test_file.getSize() == 1024);
    }
    
    SECTION("Size remains cached after multiple calls") {
        test_file.setSize(512);
        REQUIRE(test_file.getSize() == 512);
        REQUIRE(test_file.getSize() == 512);
    }
    
    SECTION("Overwrite cached size") {
        test_file.setSize(100);
        REQUIRE(test_file.getSize() == 100);
        test_file.setSize(200);
        REQUIRE(test_file.getSize() == 200);
    }
}

TEST_CASE("File read from stream", "[File]") {
    Directory root_dir("test_dir");
    File test_file("test.txt", &root_dir);
    
    SECTION("Read empty stream") {
        std::istringstream empty_stream("");
        std::vector<char> result = test_file.read(empty_stream);
        REQUIRE(result.empty());
    }
    
    SECTION("Read simple text") {
        std::string test_data = "hello world";
        std::istringstream text_stream(test_data);
        std::vector<char> result = test_file.read(text_stream);
        
        REQUIRE(result.size() == test_data.size());
        std::string result_string(result.begin(), result.end());
        REQUIRE(result_string == test_data);
    }
    
    SECTION("Read multiline text") {
        std::string test_data = "line one\nline two\nline three";
        std::istringstream text_stream(test_data);
        std::vector<char> result = test_file.read(text_stream);
        
        REQUIRE(result.size() == test_data.size());
        std::string result_string(result.begin(), result.end());
        REQUIRE(result_string == test_data);
    }
    
    SECTION("Read binary data") {
        std::string binary_data = "\x00\x01\x02\xFF\xFE\xFD";
        std::istringstream binary_stream(binary_data);
        std::vector<char> result = test_file.read(binary_stream);
        
        REQUIRE(result.size() == binary_data.size());
        for (size_t i = 0; i < result.size(); ++i) {
            REQUIRE(static_cast<unsigned char>(result[i]) == static_cast<unsigned char>(binary_data[i]));
        }
    }
    
    SECTION("Read large text") {
        std::string large_text(10000, 'a');
        std::istringstream large_stream(large_text);
        std::vector<char> result = test_file.read(large_stream);
        
        REQUIRE(result.size() == large_text.size());
        std::string result_string(result.begin(), result.end());
        REQUIRE(result_string == large_text);
    }
    
    SECTION("Read text with special characters") {
        std::string special_text = "Hello\tWorld\n\rSpecial chars: !@#$%^&*()";
        std::istringstream special_stream(special_text);
        std::vector<char> result = test_file.read(special_stream);
        
        REQUIRE(result.size() == special_text.size());
        std::string result_string(result.begin(), result.end());
        REQUIRE(result_string == special_text);
    }
}

TEST_CASE("File path handling", "[File]") {
    Directory root_dir("root");
    
    SECTION("File with simple name") {
        File test_file("simple.txt", &root_dir);
        REQUIRE(test_file.getName() == "simple.txt");
    }
    
    SECTION("File with complex name") {
        File test_file("my-file_v2.backup.tar.gz", &root_dir);
        REQUIRE(test_file.getName() == "my-file_v2.backup.tar.gz");
    }
}

TEST_CASE("File edge cases", "[File]") {
    Directory root_dir("test_dir");
    
    SECTION("Multiple read operations from same stream") {
        std::string test_data = "consistent data";
        File test_file("test.txt", &root_dir);
        
        std::istringstream stream1(test_data);
        std::vector<char> result1 = test_file.read(stream1);
        
        std::istringstream stream2(test_data);
        std::vector<char> result2 = test_file.read(stream2);
        
        REQUIRE(result1 == result2);
    }
    
    SECTION("Read operations preserve size cache") {
        Directory root_dir("test_dir");
        File test_file("test.txt", &root_dir);
        test_file.setSize(42);
        
        std::istringstream stream("some data");
        test_file.read(stream);
        
        REQUIRE(test_file.getSize() == 42);
    }
}