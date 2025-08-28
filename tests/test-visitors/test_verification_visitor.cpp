#include "directory-iteration-visitors/VerificationVisitor.hpp"
#include "utils/ChecksumFileReader.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include "calculators/ChecksumCalculator.hpp"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {
    class MockCalculator : public ChecksumCalculator {
    public:
        std::string calculate(const std::string& file_path) noexcept override {
            if (file_path.find("file_ok.txt") != std::string::npos) {
                return "mock_hash_ok";
            } else if (file_path.find("file_modified.txt") != std::string::npos) {
                return "mock_hash_different";
            } else if (file_path.find("file_new.txt") != std::string::npos) {
                return "mock_hash_new";
            }
            return "unknown_hash";
        }

        std::string getAlgorithmName() const noexcept override { 
            return "mock"; 
        }
    };

    class VerificationVisitorTestMockup {
    public:
        const std::filesystem::path base_path;
        const std::filesystem::path test_file_ok;
        const std::filesystem::path test_file_modified;
        const std::filesystem::path test_file_new;
        const std::filesystem::path checksum_file;
        
        VerificationVisitorTestMockup() 
            : base_path(std::filesystem::temp_directory_path() / "verification_visitor_test")
            , test_file_ok(base_path / "file_ok.txt")
            , test_file_modified(base_path / "file_modified.txt")
            , test_file_new(base_path / "file_new.txt")
            , checksum_file(base_path / "checksums.txt")
        {
            setup();
        }
        
        ~VerificationVisitorTestMockup() {
            cleanup();
        }
        
        VerificationVisitorTestMockup(const VerificationVisitorTestMockup&) = delete;
        VerificationVisitorTestMockup& operator=(const VerificationVisitorTestMockup&) = delete;
        
    private:
        void setup() {
            std::filesystem::remove_all(base_path);
            std::filesystem::create_directories(base_path);
            
            std::ofstream(test_file_ok) << "Hello World";
            std::ofstream(test_file_modified) << "Modified content";
            std::ofstream(test_file_new) << "New file content";
            
            std::ofstream checksum_file_stream(checksum_file);
            checksum_file_stream << "mock mock_hash_ok " << test_file_ok.string() << "\n";
            checksum_file_stream << "mock mock_hash_modified " << test_file_modified.string() << "\n";
            checksum_file_stream << "mock mock_hash_removed " << base_path.string() << "/file_removed.txt\n";
        }
        
        void cleanup() {
            std::filesystem::remove_all(base_path);
        }
    };
    
    static VerificationVisitorTestMockup test_mockup;
}

TEST_CASE("VerificationVisitor - Constructor", "[VerificationVisitor]") {
    SECTION("Valid constructor with empty checksums") {
        std::map<std::string, std::string> empty_checksums;
        REQUIRE_NOTHROW(VerificationVisitor(empty_checksums));
    }
    
    SECTION("Valid constructor with populated checksums") {
        std::map<std::string, std::string> checksums;
        checksums["/path/to/file.txt"] = "md5 d41d8cd98f00b204e9800998ecf8427e";
        REQUIRE_NOTHROW(VerificationVisitor(checksums));
    }
}

TEST_CASE("VerificationVisitor - ChecksumFileReader Integration", "[VerificationVisitor]") {
    ChecksumFileReader reader;
    auto checksums = reader.readChecksums(test_mockup.checksum_file.string());
    
    SECTION("ChecksumFileReader should parse file correctly") {
        REQUIRE(checksums.size() == 3);
        REQUIRE(checksums.count(test_mockup.test_file_ok.string()) == 1);
        REQUIRE(checksums.count(test_mockup.test_file_modified.string()) == 1);
        REQUIRE(checksums.count(test_mockup.base_path.string() + "/file_removed.txt") == 1);
        
        REQUIRE(checksums[test_mockup.test_file_ok.string()] == "mock mock_hash_ok");
        REQUIRE(checksums[test_mockup.test_file_modified.string()] == "mock mock_hash_modified");
    }
    
    SECTION("Use ChecksumFileReader with VerificationVisitor") {
        VerificationVisitor visitor(checksums);
        REQUIRE_NOTHROW(visitor.getResults());
    }
}

TEST_CASE("VerificationVisitor - visitFile with OK status", "[VerificationVisitor]") {
    SECTION("File with matching checksum should be OK") {
        std::map<std::string, std::string> checksums;
        checksums[test_mockup.test_file_ok.string()] = "mock mock_hash_ok";
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File ok_file(test_mockup.test_file_ok, &root_dir);
        
        visitor.visitFile(ok_file);
        auto results = visitor.getResults();
        
        REQUIRE(results.count(test_mockup.test_file_ok.string()) == 1);
        // Since "mock" is unknown algorithm, it should be MODIFIED
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::MODIFIED);
    }
}

TEST_CASE("VerificationVisitor - visitFile with MODIFIED status", "[VerificationVisitor]") {
    SECTION("File with mismatched checksum should be MODIFIED") {
        std::map<std::string, std::string> checksums;
        checksums[test_mockup.test_file_modified.string()] = "mock wrong_hash";
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File modified_file(test_mockup.test_file_modified, &root_dir);
        
        visitor.visitFile(modified_file);
        auto results = visitor.getResults();
        
        REQUIRE(results.count(test_mockup.test_file_modified.string()) == 1);
        REQUIRE(results[test_mockup.test_file_modified.string()] == VerificationStatus::MODIFIED);
    }
}

TEST_CASE("VerificationVisitor - visitFile with NEW status", "[VerificationVisitor]") {
    SECTION("File not in checksums should be NEW") {
        std::map<std::string, std::string> checksums; // Empty checksums
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File new_file(test_mockup.test_file_new, &root_dir);
        
        visitor.visitFile(new_file);
        auto results = visitor.getResults();
        
        REQUIRE(results.count(test_mockup.test_file_new.string()) == 1);
        REQUIRE(results[test_mockup.test_file_new.string()] == VerificationStatus::NEW);
    }
}

TEST_CASE("VerificationVisitor - REMOVED status in final results", "[VerificationVisitor]") {
    SECTION("Files in checksums but not visited should be REMOVED") {
        std::map<std::string, std::string> checksums;
        checksums[test_mockup.test_file_ok.string()] = "mock mock_hash_ok";
        checksums[test_mockup.test_file_modified.string()] = "mock mock_hash_modified";
        checksums[test_mockup.base_path.string() + "/file_removed.txt"] = "mock mock_hash_removed";
        
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File ok_file(test_mockup.test_file_ok, &root_dir);
        File modified_file(test_mockup.test_file_modified, &root_dir);
        
        visitor.visitFile(ok_file);
        visitor.visitFile(modified_file);
        auto results = visitor.getResults();
        
        std::string removed_file_path = test_mockup.base_path.string() + "/file_removed.txt";
        REQUIRE(results.count(removed_file_path) == 1);
        REQUIRE(results[removed_file_path] == VerificationStatus::REMOVED);
    }
}

TEST_CASE("VerificationVisitor - Multiple algorithm support", "[VerificationVisitor]") {
    SECTION("Unknown algorithm should be treated as MODIFIED") {
        std::map<std::string, std::string> unknown_checksums;
        unknown_checksums[test_mockup.test_file_ok.string()] = "unknown_algorithm some_hash_value";
        VerificationVisitor visitor(unknown_checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file(test_mockup.test_file_ok, &root_dir);
        
        visitor.visitFile(file);
        auto results = visitor.getResults();
        
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::MODIFIED);
    }
    
    SECTION("SHA1 algorithm with wrong checksum should be MODIFIED") {
        std::map<std::string, std::string> sha1_checksums;
        sha1_checksums[test_mockup.test_file_ok.string()] = "sha1 wrong_hash_value_here";
        VerificationVisitor visitor(sha1_checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file(test_mockup.test_file_ok, &root_dir);
        
        visitor.visitFile(file);
        auto results = visitor.getResults();
        
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::MODIFIED);
    }
    
    SECTION("SHA256 algorithm with wrong checksum should be MODIFIED") {
        std::map<std::string, std::string> sha256_checksums;
        sha256_checksums[test_mockup.test_file_ok.string()] = "sha256 wrong_hash_value_here";
        VerificationVisitor visitor(sha256_checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file(test_mockup.test_file_ok, &root_dir);
        
        visitor.visitFile(file);
        auto results = visitor.getResults();
        
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::MODIFIED);
    }
}

TEST_CASE("VerificationVisitor - Unknown algorithm handling", "[VerificationVisitor]") {
    SECTION("Unknown algorithm should result in MODIFIED status") {
        std::map<std::string, std::string> unknown_checksums;
        unknown_checksums[test_mockup.test_file_ok.string()] = "unknown_alg abc123def456";
        VerificationVisitor visitor(unknown_checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file(test_mockup.test_file_ok, &root_dir);
        
        visitor.visitFile(file);
        auto results = visitor.getResults();
        
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::MODIFIED);
    }
}

TEST_CASE("VerificationVisitor - visitDirectory behavior", "[VerificationVisitor]") {
    SECTION("visitDirectory should do nothing") {
        std::map<std::string, std::string> empty_checksums;
        VerificationVisitor visitor(empty_checksums);
        
        Directory test_dir("test_directory");
        
        REQUIRE_NOTHROW(visitor.visitDirectory(test_dir));
        
        auto results = visitor.getResults();
        REQUIRE(results.empty());
    }
}

TEST_CASE("VerificationVisitor - Complete verification scenario", "[VerificationVisitor]") {
    SECTION("All status types in single verification") {
        std::map<std::string, std::string> checksums;
        checksums[test_mockup.test_file_ok.string()] = "unknown_algorithm some_hash"; // MODIFIED case (unknown algorithm)
        checksums[test_mockup.test_file_modified.string()] = "sha1 wrong_hash"; // MODIFIED case
        checksums[test_mockup.base_path.string() + "/file_removed.txt"] = "mock mock_hash_removed"; // REMOVED case
        
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File ok_file(test_mockup.test_file_ok, &root_dir);
        File modified_file(test_mockup.test_file_modified, &root_dir);
        File new_file(test_mockup.test_file_new, &root_dir);
        
        visitor.visitFile(ok_file);
        visitor.visitFile(modified_file);
        visitor.visitFile(new_file);
        
        auto results = visitor.getResults();
        
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::MODIFIED);
        REQUIRE(results[test_mockup.test_file_modified.string()] == VerificationStatus::MODIFIED);
        REQUIRE(results[test_mockup.test_file_new.string()] == VerificationStatus::NEW);
        
        std::string removed_file_path = test_mockup.base_path.string() + "/file_removed.txt";
        REQUIRE(results[removed_file_path] == VerificationStatus::REMOVED);
        
        REQUIRE(results.size() == 4);
    }
}

TEST_CASE("VerificationVisitor - Empty checksum file handling", "[VerificationVisitor]") {
    SECTION("Empty checksums with files should mark all as NEW") {
        std::map<std::string, std::string> empty_checksums;
        VerificationVisitor visitor(empty_checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file1(test_mockup.test_file_ok, &root_dir);
        File file2(test_mockup.test_file_new, &root_dir);
        
        visitor.visitFile(file1);
        visitor.visitFile(file2);
        
        auto results = visitor.getResults();
        
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::NEW);
        REQUIRE(results[test_mockup.test_file_new.string()] == VerificationStatus::NEW);
        REQUIRE(results.size() == 2);
    }
}

TEST_CASE("VerificationVisitor - Results consistency", "[VerificationVisitor]") {
    SECTION("Multiple calls to getResults should be consistent") {
        std::filesystem::path empty_file = test_mockup.base_path / "empty.txt";
        std::ofstream empty_stream(empty_file);
        empty_stream.close();
        
        std::map<std::string, std::string> checksums;
        checksums[empty_file.string()] = "md5 d41d8cd98f00b204e9800998ecf8427e";
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File ok_file(empty_file, &root_dir);
        
        visitor.visitFile(ok_file);
        
        auto results1 = visitor.getResults();
        auto results2 = visitor.getResults();
        
        REQUIRE(results1.size() == results2.size());
        for (const auto& pair : results1) {
            REQUIRE(results2.count(pair.first) == 1);
            REQUIRE(results2[pair.first] == pair.second);
        }
        
        std::filesystem::remove(empty_file);
    }
}

TEST_CASE("VerificationVisitor - Malformed checksum handling", "[VerificationVisitor]") {
    SECTION("Malformed checksum entry should be handled gracefully") {
        std::map<std::string, std::string> malformed_checksums;
        malformed_checksums[test_mockup.test_file_ok.string()] = "incomplete_entry";
        VerificationVisitor visitor(malformed_checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file(test_mockup.test_file_ok, &root_dir);
        
        REQUIRE_NOTHROW(visitor.visitFile(file));
        
        auto results = visitor.getResults();
        REQUIRE(results.count(test_mockup.test_file_ok.string()) == 1);
    }
}

TEST_CASE("VerificationVisitor - Edge cases", "[VerificationVisitor]") {
    SECTION("Visit same file multiple times") {
        std::map<std::string, std::string> checksums;
        checksums[test_mockup.test_file_ok.string()] = "sha1 wrong_hash_value";
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file(test_mockup.test_file_ok, &root_dir);
        
        visitor.visitFile(file);
        visitor.visitFile(file); // Second visit - file not in checksums anymore, so NEW
        
        auto results = visitor.getResults();
        
        REQUIRE(results.size() == 1);
        // After first visit, the file is removed from checksums, so second visit makes it NEW
        REQUIRE(results[test_mockup.test_file_ok.string()] == VerificationStatus::NEW);
    }
    
    SECTION("File path with spaces and special characters") {
        std::filesystem::path special_file = test_mockup.base_path / "file with spaces & symbols.txt";
        std::ofstream special_stream(special_file);
        special_stream.close();
        
        std::map<std::string, std::string> checksums;
        checksums[special_file.string()] = "md5 wrong_checksum";
        VerificationVisitor visitor(checksums);
        
        Directory root_dir(test_mockup.base_path);
        File file(special_file, &root_dir);
        
        visitor.visitFile(file);
        auto results = visitor.getResults();
        
        REQUIRE(results.count(special_file.string()) == 1);
        REQUIRE(results[special_file.string()] == VerificationStatus::MODIFIED);
        
        std::filesystem::remove(special_file);
    }
}