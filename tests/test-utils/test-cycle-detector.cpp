#include "directory-tree-builders/CycleDetector.hpp"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>

namespace {
    /**
     * @brief Mockup class to create and manage test directory structure
     * Uses RAII pattern - sets up in constructor, cleans up in destructor
     */
    class TestDirectoryMockup {
    public:
        const std::filesystem::path basePath;
        const std::filesystem::path dir1;
        const std::filesystem::path dir2;
        const std::filesystem::path dir3;
        const std::filesystem::path nestedDir;
        const std::filesystem::path symlinkTestDir;
        const std::filesystem::path file1;
        const std::filesystem::path file2;
        const std::filesystem::path file3;
        const std::filesystem::path linkToDir2;
        const std::filesystem::path linkToSymlinkTest;
        
        TestDirectoryMockup() 
            : basePath(std::filesystem::temp_directory_path() / "cycle_detector_test_mockup")
            , dir1(basePath / "dir1")
            , dir2(basePath / "dir2")
            , dir3(basePath / "dir3")
            , nestedDir(basePath / "nested" / "subdir")
            , symlinkTestDir(basePath / "symlink_test")
            , file1(basePath / "file1.txt")
            , file2(basePath / "file2.txt")
            , file3(basePath / "nested" / "file3.txt")
            , linkToDir2(symlinkTestDir / "link_to_dir2")
            , linkToSymlinkTest(dir2 / "link_to_symlink_test")
        {
            setup();
        }
        
        ~TestDirectoryMockup() {
            cleanup();
        }
        
        // Delete copy constructor and assignment operator to prevent copying
        TestDirectoryMockup(const TestDirectoryMockup&) = delete;
        TestDirectoryMockup& operator=(const TestDirectoryMockup&) = delete;
        
    private:
        void setup() {
            // Clean up any existing structure first
            std::filesystem::remove_all(basePath);
            
            // Create directory structure
            std::filesystem::create_directories(dir1);
            std::filesystem::create_directories(dir2);
            std::filesystem::create_directories(dir3);
            std::filesystem::create_directories(nestedDir);
            std::filesystem::create_directories(symlinkTestDir);
            
            // Create test files
            std::ofstream(file1) << "test content 1";
            std::ofstream(file2) << "test content 2";
            std::ofstream(file3) << "test content 3";
            
            // Create symbolic links for testing cycles
            std::error_code ec;
            std::filesystem::create_symlink(dir2, linkToDir2, ec);
            std::filesystem::create_symlink(symlinkTestDir, linkToSymlinkTest, ec);
        }
        
        void cleanup() {
            std::filesystem::remove_all(basePath);
        }
    };
    
    // Global mockup instance that lasts for the entire test file
    static TestDirectoryMockup testMockup;
}

/**
 * @test CycleDetector tests
 * Tests the cycle detection functionality for symbolic links and circular dependencies
 */
TEST_CASE("CycleDetector - Basic functionality", "[CycleDetector]") {
    CycleDetector detector;

    SECTION("First visit to a path should return false (no cycle)") {
        // First check should return false (no cycle detected)
        REQUIRE_FALSE(detector.check(testMockup.dir1));
    }

    SECTION("Second visit to same path should return true (cycle detected)") {
        CycleDetector detector;
        
        // First check should return false
        REQUIRE_FALSE(detector.check(testMockup.dir1));
        
        // Second check to same path should return true (cycle detected)
        REQUIRE(detector.check(testMockup.dir1));
    }
}

TEST_CASE("CycleDetector - Non-existent paths", "[CycleDetector]") {
    CycleDetector detector;

    SECTION("Non-existent path should return false") {
        std::filesystem::path nonExistentPath = testMockup.basePath / "non_existent_path_12345";
        
        // Should return false for non-existent path
        REQUIRE_FALSE(detector.check(nonExistentPath));
    }
}

TEST_CASE("CycleDetector - Real files and directories", "[CycleDetector]") {
    CycleDetector detector;
    
    SECTION("Regular files should work correctly") {
        // First check should return false
        REQUIRE_FALSE(detector.check(testMockup.file1));
        
        // Second check should return true (already visited)
        REQUIRE(detector.check(testMockup.file1));
    }
    
    SECTION("Multiple different paths should not interfere") {
        // First visits should all return false
        REQUIRE_FALSE(detector.check(testMockup.dir1));
        REQUIRE_FALSE(detector.check(testMockup.dir2));
        REQUIRE_FALSE(detector.check(testMockup.dir3));
        
        // Revisits should return true
        REQUIRE(detector.check(testMockup.dir1));
        REQUIRE(detector.check(testMockup.dir2));
        REQUIRE(detector.check(testMockup.dir3));
    }
}

TEST_CASE("CycleDetector - Symbolic link cycles", "[CycleDetector]") {
    CycleDetector detector;
    
    SECTION("Direct symbolic link cycle") {
        // Check directories first
        REQUIRE_FALSE(detector.check(testMockup.dir2));
        REQUIRE_FALSE(detector.check(testMockup.symlinkTestDir));
        
        // Now check the symlinks - these should resolve to canonical paths
        // If they point to already visited directories, should detect cycle
        REQUIRE(detector.check(testMockup.linkToSymlinkTest)); // Points to symlink_test which was already visited
        REQUIRE(detector.check(testMockup.linkToDir2)); // Points to dir2 which was already visited
    }
    
    SECTION("Self-referencing symbolic link handling") {
        // Create a self-referencing symlink in our mockup structure
        std::filesystem::path selfLink = testMockup.basePath / "self_link";
        
        std::error_code ec;
        std::filesystem::create_symlink(selfLink, selfLink, ec);
        
        if (!ec) { // Only run test if symlink was created successfully
            // This should handle the self-reference gracefully
            // The exact behavior depends on how std::filesystem::canonical handles this
            bool result = detector.check(selfLink);
            // We mainly want to ensure it doesn't crash
            (void)result; // Suppress unused variable warning
            
            // Clean up the self-referencing link
            std::filesystem::remove(selfLink, ec);
        }
    }
}

TEST_CASE("CycleDetector - Path canonicalization", "[CycleDetector]") {
    CycleDetector detector;
    
    SECTION("Different representations of same path should be detected as cycle") {
        // Create different representations of the same path
        std::filesystem::path path1 = testMockup.nestedDir;
        std::filesystem::path path2 = testMockup.basePath / "nested" / "." / "subdir";
        std::filesystem::path path3 = testMockup.basePath / "nested" / "subdir" / ".";
        
        // First check with path1
        REQUIRE_FALSE(detector.check(path1));
        
        // Checks with different representations should detect as already visited
        REQUIRE(detector.check(path2));
        REQUIRE(detector.check(path3));
    }
}

TEST_CASE("CycleDetector - Independent detector instances", "[CycleDetector]") {
    SECTION("Different detector instances should be independent") {
        CycleDetector detector1;
        CycleDetector detector2;
        
        std::filesystem::path testPath = testMockup.dir1;
        
        // First detector visits the path
        REQUIRE_FALSE(detector1.check(testPath));
        REQUIRE(detector1.check(testPath)); // Second visit should detect cycle
        
        // Second detector should not know about first detector's visits
        REQUIRE_FALSE(detector2.check(testPath)); // First visit for detector2
        REQUIRE(detector2.check(testPath)); // Second visit for detector2 should detect cycle
    }
}