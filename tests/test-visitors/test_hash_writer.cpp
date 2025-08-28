#include "directory-iteration-visitors/HashStreamWriter.hpp"
#include "calculators/ChecksumCalculator.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include "file-system-composite/Link.hpp"
#include <catch2/catch_all.hpp>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <memory>

namespace {
    class MockCalculator : public ChecksumCalculator {
    public:
        std::string calculate(const std::string& data) noexcept override {
            if (data.empty()) {
                return "empty_hash";
            }
            return "mock_hash_" + std::to_string(data.length());
        }

        std::string getAlgorithmName() const noexcept override { return "mock"; }
    };

    class HashWriterTestMockup {
    public:
        const std::filesystem::path base_path;
        const std::filesystem::path test_file1;
        const std::filesystem::path test_file2;
        const std::filesystem::path empty_file;
        
        HashWriterTestMockup() 
            : base_path(std::filesystem::temp_directory_path() / "hash_writer_test")
            , test_file1(base_path / "test1.txt")
            , test_file2(base_path / "test2.txt")
            , empty_file(base_path / "empty.txt")
        {
            setup();
        }
        
        ~HashWriterTestMockup() {
            cleanup();
        }
        
        HashWriterTestMockup(const HashWriterTestMockup&) = delete;
        HashWriterTestMockup& operator=(const HashWriterTestMockup&) = delete;
        
    private:
        void setup() {
            std::filesystem::remove_all(base_path);
            std::filesystem::create_directories(base_path);
            
            std::ofstream(test_file1) << "Hello World";
            std::ofstream(test_file2) << "Test content for file 2";
            std::ofstream(empty_file);
        }
        
        void cleanup() {
            std::filesystem::remove_all(base_path);
        }
    };
    
    static HashWriterTestMockup test_mockup;
}

TEST_CASE("HashStreamWriter - Constructor", "[HashStreamWriter]") {
    std::ostringstream output_stream;
    
    SECTION("Valid constructor with mock calculator") {
        auto calculator = std::make_unique<MockCalculator>();
        REQUIRE_NOTHROW(HashStreamWriter(std::move(calculator), output_stream));
    }
    
    SECTION("Constructor with null calculator should throw") {
        std::unique_ptr<ChecksumCalculator> null_calc = nullptr;
        REQUIRE_THROWS_AS(HashStreamWriter(std::move(null_calc), output_stream), std::runtime_error);
    }
}

TEST_CASE("HashStreamWriter - visitFile", "[HashStreamWriter]") {
    std::ostringstream output_stream;
    auto calculator = std::make_unique<MockCalculator>();
    HashStreamWriter writer(std::move(calculator), output_stream);
    
    SECTION("Visit single file with content") {
        Directory root_dir(test_mockup.base_path);
        File test_file(test_mockup.test_file1, &root_dir);
        
        writer.visitFile(test_file);
        
        std::string output = output_stream.str();
        REQUIRE_FALSE(output.empty());
        REQUIRE(output.find("mock_hash_11") != std::string::npos);
        REQUIRE(output.find(test_mockup.test_file1.string()) != std::string::npos);
        REQUIRE(output.back() == '\n');
    }
    
    SECTION("Visit empty file") {
        Directory root_dir(test_mockup.base_path);
        File empty_test_file(test_mockup.empty_file, &root_dir);
        
        REQUIRE_THROWS(writer.visitFile(empty_test_file));
    }
    
    SECTION("Visit multiple files") {
        Directory root_dir(test_mockup.base_path);
        File file1(test_mockup.test_file1, &root_dir);
        File file2(test_mockup.test_file2, &root_dir);
        
        writer.visitFile(file1);
        writer.visitFile(file2);
        
        std::string output = output_stream.str();
        
        std::istringstream line_stream(output);
        std::string line;
        int line_count = 0;
        
        while (std::getline(line_stream, line)) {
            line_count++;
            if (line_count == 1) {
                REQUIRE(line.find("mock_hash_11") != std::string::npos);
                REQUIRE(line.find(test_mockup.test_file1.string()) != std::string::npos);
            } else if (line_count == 2) {
                REQUIRE(line.find("mock_hash_23") != std::string::npos);
                REQUIRE(line.find(test_mockup.test_file2.string()) != std::string::npos);
            }
        }
        
        REQUIRE(line_count == 2);
    }
}

TEST_CASE("HashStreamWriter - visitDirectory", "[HashStreamWriter]") {
    std::ostringstream output_stream;
    auto calculator = std::make_unique<MockCalculator>();
    HashStreamWriter writer(std::move(calculator), output_stream);
    
    SECTION("Visit directory should produce no output") {
        Directory test_dir("test_root");
        
        writer.visitDirectory(test_dir);
        
        std::string output = output_stream.str();
        REQUIRE(output.empty());
    }
}

TEST_CASE("HashStreamWriter - visitLink", "[HashStreamWriter]") {
    std::ostringstream output_stream;
    auto calculator = std::make_unique<MockCalculator>();
    HashStreamWriter writer(std::move(calculator), output_stream);
    
    SECTION("Visit link to existing file should process target") {
        Directory root_dir(test_mockup.base_path);
        Link test_link("link_to_file", "target.txt", &root_dir);
        
        // Create a target file and resolve the link to it
        auto target_file = std::make_unique<File>(test_mockup.test_file1, &root_dir);
        test_link.setResolveTarget(std::move(target_file));
        
        writer.visitLink(test_link);
        
        std::string output = output_stream.str();
        REQUIRE_FALSE(output.empty());
        REQUIRE(output.find("mock_hash_11") != std::string::npos);
    }
    
    SECTION("Visit link with no resolved target should produce no output") {
        Directory root_dir(test_mockup.base_path);
        Link broken_test_link("broken_link", "non_existent_target", &root_dir);
        
        writer.visitLink(broken_test_link);
        
        std::string output = output_stream.str();
        REQUIRE(output.empty());
    }
}

TEST_CASE("HashStreamWriter - Output format", "[HashStreamWriter]") {
    std::ostringstream output_stream;
    auto calculator = std::make_unique<MockCalculator>();
    HashStreamWriter writer(std::move(calculator), output_stream);
    
    SECTION("Output format should be <hash-type><space><hash><space><path><newline>") {
        Directory root_dir(test_mockup.base_path);
        File test_file(test_mockup.test_file1, &root_dir);
        
        writer.visitFile(test_file);
        
        std::string output = output_stream.str();
        std::istringstream line_stream(output);
        std::string line;
        
        REQUIRE(std::getline(line_stream, line));
        
        size_t space_pos1 = line.find(' ');
        size_t space_pos2 = line.rfind(' ');
        REQUIRE(space_pos1 != std::string::npos);
        
        std::string hash_type_part = line.substr(0, space_pos1);
        std::string hash_part = line.substr(space_pos1 + 1, strlen("mock_hash_11"));
        std::string path_part = line.substr(space_pos2 + 1);
        
        REQUIRE(hash_type_part == "mock");
        REQUIRE(hash_part == "mock_hash_11");
        REQUIRE(path_part == test_mockup.test_file1.string());
        
        REQUIRE_FALSE(std::getline(line_stream, line));
    }
}

class DeterministicCalculator : public ChecksumCalculator {
public:
    std::string calculate(const std::string& data) noexcept override {
        if (data == "Hello World") {
            return "abcd1234";
        } else if (data == "Test content for file 2") {
            return "efgh5678";
        } else if (data.empty()) {
            return "empty0000";
        }
        return "unknown";
    }

    std::string getAlgorithmName() const noexcept override { return "mock"; }
    
};

TEST_CASE("HashStreamWriter - Integration with real-like calculator", "[HashStreamWriter]") {
    std::ostringstream output_stream;
    auto calculator = std::make_unique<DeterministicCalculator>();
    HashStreamWriter writer(std::move(calculator), output_stream);
    
    SECTION("Process files with deterministic hashes") {
        Directory root_dir(test_mockup.base_path);
        File file1(test_mockup.test_file1, &root_dir);
        File file2(test_mockup.test_file2, &root_dir);
        
        writer.visitFile(file1);
        writer.visitFile(file2);
        
        std::string output = output_stream.str();
        std::istringstream line_stream(output);
        std::string line;
        
        REQUIRE(std::getline(line_stream, line));
        REQUIRE(line.find("abcd1234") != std::string::npos);
        REQUIRE(line.find(test_mockup.test_file1.string()) != std::string::npos);
        
        REQUIRE(std::getline(line_stream, line));
        REQUIRE(line.find("efgh5678") != std::string::npos);
        REQUIRE(line.find(test_mockup.test_file2.string()) != std::string::npos);
    }
}
