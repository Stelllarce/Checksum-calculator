#include "utils/ChecksumFileReader.hpp"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

namespace {
    class ChecksumFileReaderTestMockup {
    public:
        const std::filesystem::path base_path;
        const std::filesystem::path valid_checksum_file;
        const std::filesystem::path empty_checksum_file;
        const std::filesystem::path malformed_checksum_file;
        const std::filesystem::path nonexistent_file;
        
        ChecksumFileReaderTestMockup() 
            : base_path(std::filesystem::temp_directory_path() / "checksum_reader_test")
            , valid_checksum_file(base_path / "valid_checksums.txt")
            , empty_checksum_file(base_path / "empty_checksums.txt")
            , malformed_checksum_file(base_path / "malformed_checksums.txt")
            , nonexistent_file(base_path / "does_not_exist.txt")
        {
            setup();
        }
        
        ~ChecksumFileReaderTestMockup() {
            cleanup();
        }
        
        ChecksumFileReaderTestMockup(const ChecksumFileReaderTestMockup&) = delete;
        ChecksumFileReaderTestMockup& operator=(const ChecksumFileReaderTestMockup&) = delete;
        
    private:
        void setup() {
            std::filesystem::remove_all(base_path);
            std::filesystem::create_directories(base_path);
            
            std::ofstream valid_file(valid_checksum_file);
            valid_file << "md5 d41d8cd98f00b204e9800998ecf8427e /path/to/file1.txt\n";
            valid_file << "sha1 da39a3ee5e6b4b0d3255bfef95601890afd80709 /path/to/file2.txt\n";
            valid_file << "sha256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 /path/to/file3.txt\n";
            
            std::ofstream empty_file(empty_checksum_file);
            
            std::ofstream malformed_file(malformed_checksum_file);
            malformed_file << "incomplete line\n";
            malformed_file << "md5 only_two_parts\n";
            malformed_file << "sha1 valid_hash /valid/path.txt\n";
            malformed_file << "\n";
            malformed_file << "   \n";
        }
        
        void cleanup() {
            std::filesystem::remove_all(base_path);
        }
    };
    
    static ChecksumFileReaderTestMockup test_mockup;
}

TEST_CASE("ChecksumFileReader - Valid checksum file", "[ChecksumFileReader]") {
    ChecksumFileReader reader;
    
    auto checksums = reader.readChecksums(test_mockup.valid_checksum_file.string());
    
    REQUIRE(checksums.size() == 3);
    REQUIRE(checksums.count("/path/to/file1.txt") == 1);
    REQUIRE(checksums.count("/path/to/file2.txt") == 1);
    REQUIRE(checksums.count("/path/to/file3.txt") == 1);
    
    REQUIRE(checksums["/path/to/file1.txt"] == "md5 d41d8cd98f00b204e9800998ecf8427e");
    REQUIRE(checksums["/path/to/file2.txt"] == "sha1 da39a3ee5e6b4b0d3255bfef95601890afd80709");
    REQUIRE(checksums["/path/to/file3.txt"] == "sha256 e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
}

TEST_CASE("ChecksumFileReader - Empty checksum file", "[ChecksumFileReader]") {
    ChecksumFileReader reader;
    
    auto checksums = reader.readChecksums(test_mockup.empty_checksum_file.string());
    
    REQUIRE(checksums.empty());
}

TEST_CASE("ChecksumFileReader - Nonexistent file", "[ChecksumFileReader]") {
    ChecksumFileReader reader;
    
    auto checksums = reader.readChecksums(test_mockup.nonexistent_file.string());
    
    REQUIRE(checksums.empty());
}

TEST_CASE("ChecksumFileReader - Malformed entries", "[ChecksumFileReader]") {
    ChecksumFileReader reader;
    
    auto checksums = reader.readChecksums(test_mockup.malformed_checksum_file.string());
    
    REQUIRE(checksums.size() == 1);
    REQUIRE(checksums.count("/valid/path.txt") == 1);
    REQUIRE(checksums["/valid/path.txt"] == "sha1 valid_hash");
}

TEST_CASE("ChecksumFileReader - Single line file", "[ChecksumFileReader]") {
    ChecksumFileReader reader;
    std::filesystem::path single_line_file = test_mockup.base_path / "single_line.txt";
    
    std::ofstream file(single_line_file);
    file << "md5 abc123def456 /single/file.txt";
    file.close();
    
    auto checksums = reader.readChecksums(single_line_file.string());
    
    REQUIRE(checksums.size() == 1);
    REQUIRE(checksums["/single/file.txt"] == "md5 abc123def456");
    
    std::filesystem::remove(single_line_file);
}

TEST_CASE("ChecksumFileReader - File paths with spaces", "[ChecksumFileReader]") {
    ChecksumFileReader reader;
    std::filesystem::path spaces_file = test_mockup.base_path / "spaces.txt";
    
    std::ofstream file(spaces_file);
    file << "md5 hash123 /path/with spaces/file.txt\n";
    file.close();
    
    auto checksums = reader.readChecksums(spaces_file.string());
    
    REQUIRE(checksums.size() == 1);
    REQUIRE(checksums.count("/path/with") == 1);
    REQUIRE(checksums["/path/with"] == "md5 hash123");
    
    std::filesystem::remove(spaces_file);
}

TEST_CASE("ChecksumFileReader - Different algorithm formats", "[ChecksumFileReader]") {
    ChecksumFileReader reader;
    std::filesystem::path formats_file = test_mockup.base_path / "formats.txt";
    
    std::ofstream file(formats_file);
    file << "MD5 uppercase_alg /file1.txt\n";
    file << "sha512 longer_hash /file2.txt\n";
    file << "blake2b custom_algorithm /file3.txt\n";
    file.close();
    
    auto checksums = reader.readChecksums(formats_file.string());
    
    REQUIRE(checksums.size() == 3);
    REQUIRE(checksums["/file1.txt"] == "MD5 uppercase_alg");
    REQUIRE(checksums["/file2.txt"] == "sha512 longer_hash");
    REQUIRE(checksums["/file3.txt"] == "blake2b custom_algorithm");
    
    std::filesystem::remove(formats_file);
}
