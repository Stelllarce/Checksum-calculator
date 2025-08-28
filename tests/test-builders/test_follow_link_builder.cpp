#include "directory-tree-builders/LinkFollowBuilder.hpp"
#include "directory-tree-builders/CycleDetector.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <memory>

namespace {
    class LinkFollowBuilderTestMockup {
    public:
        const std::filesystem::path base_path;
        const std::filesystem::path source_dir;
        const std::filesystem::path target_dir;
        const std::filesystem::path nested_dir;
        const std::filesystem::path test_file;
        const std::filesystem::path nested_file;
        const std::filesystem::path link_to_file;
        const std::filesystem::path link_to_dir;
        const std::filesystem::path link_to_nested;
        const std::filesystem::path circular_link1;
        const std::filesystem::path circular_link2;
        const std::filesystem::path broken_link;
        
        LinkFollowBuilderTestMockup() 
            : base_path(std::filesystem::temp_directory_path() / "link_follow_builder_test")
            , source_dir(base_path / "source")
            , target_dir(base_path / "target")
            , nested_dir(target_dir / "nested")
            , test_file(target_dir / "test_file.txt")
            , nested_file(nested_dir / "nested_file.txt")
            , link_to_file(source_dir / "link_to_file")
            , link_to_dir(source_dir / "link_to_dir")
            , link_to_nested(source_dir / "link_to_nested")
            , circular_link1(source_dir / "circular1")
            , circular_link2(target_dir / "circular2")
            , broken_link(source_dir / "broken_link")
        {
            setup();
        }
        
        ~LinkFollowBuilderTestMockup() {
            cleanup();
        }
        
        LinkFollowBuilderTestMockup(const LinkFollowBuilderTestMockup&) = delete;
        LinkFollowBuilderTestMockup& operator=(const LinkFollowBuilderTestMockup&) = delete;
        
    private:
        void setup() {
            std::filesystem::remove_all(base_path);
            
            std::filesystem::create_directories(source_dir);
            std::filesystem::create_directories(target_dir);
            std::filesystem::create_directories(nested_dir);
            
            std::ofstream(test_file) << "test file content";
            std::ofstream(nested_file) << "nested file content";
            
            std::error_code ec;
            std::filesystem::create_symlink(test_file, link_to_file, ec);
            std::filesystem::create_symlink(target_dir, link_to_dir, ec);
            std::filesystem::create_symlink(nested_dir, link_to_nested, ec);
            
            std::filesystem::create_symlink(circular_link2, circular_link1, ec);
            std::filesystem::create_symlink(circular_link1, circular_link2, ec);
            
            std::filesystem::create_symlink(base_path / "non_existent", broken_link, ec);
        }
        
        void cleanup() {
            std::filesystem::remove_all(base_path);
        }
    };
    
    static LinkFollowBuilderTestMockup test_mockup;
}

TEST_CASE("LinkFollowBuilder - Constructor and basic setup", "[LinkFollowBuilder]") {
    SECTION("Constructor creates an empty tree") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        
        auto tree = builder.getTree();
        REQUIRE(tree == nullptr);
    }
}

TEST_CASE("LinkFollowBuilder - Following file links", "[LinkFollowBuilder]") {
    SECTION("Follow link to regular file") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        
        builder.startBuildDirectory("root");
        auto result = builder.buildLink("linked_file", test_mockup.test_file);
        builder.endBuildDirectory();

        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto linked_file = tree->getChild("linked_file");
        REQUIRE(linked_file != nullptr);
        REQUIRE(linked_file->getName() == "linked_file");
    }
    
    SECTION("Follow link to file through symbolic link") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        
        builder.startBuildDirectory("root");
        auto result = builder.buildLink("symlinked_file", test_mockup.link_to_file);
        builder.endBuildDirectory();

        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto linked_file = tree->getChild("symlinked_file");
        REQUIRE(linked_file != nullptr);
        REQUIRE(linked_file->getName() == "symlinked_file");
    }
}

TEST_CASE("LinkFollowBuilder - Following directory links", "[LinkFollowBuilder]") {
    SECTION("Follow link to directory") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        
        builder.startBuildDirectory("root");
        auto result = builder.buildLink("linked_dir", test_mockup.target_dir);
        builder.endBuildDirectory();

        REQUIRE(result != nullptr);
        REQUIRE(result->getName() == "linked_dir");
        
        auto tree = builder.getTree();
        auto linked_dir = tree->getChild("linked_dir");
        REQUIRE(linked_dir != nullptr);
        REQUIRE(linked_dir->getName() == "linked_dir");
    }
    
    SECTION("Follow link to directory through symbolic link") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        
        builder.startBuildDirectory("root");
        auto result = builder.buildLink("symlinked_dir", test_mockup.link_to_dir);
        builder.endBuildDirectory();
        
        REQUIRE(result != nullptr);
        REQUIRE(result->getName() == "symlinked_dir");
        
        auto tree = builder.getTree();
        auto linked_dir = tree->getChild("symlinked_dir");
        REQUIRE(linked_dir != nullptr);
        REQUIRE(linked_dir->getName() == "symlinked_dir");
    }
}

TEST_CASE("LinkFollowBuilder - Cycle detection integration", "[LinkFollowBuilder]") {
    SECTION("Same path visited twice - should detect cycle on second visit") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        
        builder.startBuildDirectory("root");
        auto result1 = builder.buildLink("first_visit", test_mockup.test_file);
        REQUIRE(result1 == nullptr);
        
        auto tree = builder.getTree();
        auto first_file = tree->getChild("first_visit");
        REQUIRE(first_file != nullptr);
        
        auto result2 = builder.buildLink("second_visit", test_mockup.test_file);
        REQUIRE(result2 == nullptr);
        
        auto second_file = tree->getChild("second_visit");
        REQUIRE(second_file == nullptr);
        builder.endBuildDirectory();
    }
    
    SECTION("Different paths should not interfere") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        builder.startBuildDirectory("root");

        auto result1 = builder.buildLink("file1", test_mockup.test_file);
        auto result2 = builder.buildLink("file2", test_mockup.nested_file);
        
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
        
        auto tree = builder.getTree();
        auto file1 = tree->getChild("file1");
        auto file2 = tree->getChild("file2");
        
        REQUIRE(file1 != nullptr);
        REQUIRE(file2 != nullptr);
        REQUIRE(file1->getName() == "file1");
        REQUIRE(file2->getName() == "file2");
        
        builder.endBuildDirectory();
    }
}

TEST_CASE("LinkFollowBuilder - Real cycle detection", "[LinkFollowBuilder]") {
    SECTION("Detect circular symbolic links") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        builder.startBuildDirectory("root");

        auto result1 = builder.buildLink("circular1", test_mockup.circular_link1);
        auto result2 = builder.buildLink("circular2", test_mockup.circular_link2);
        
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
        
        auto tree = builder.getTree();
        
        REQUIRE(tree != nullptr);
        builder.endBuildDirectory();
    }
}

TEST_CASE("LinkFollowBuilder - Error handling", "[LinkFollowBuilder]") {
    SECTION("Broken symbolic link") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        builder.startBuildDirectory("root");
        
        auto result = builder.buildLink("broken", test_mockup.broken_link);
        
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto broken_item = tree->getChild("broken");
        REQUIRE(broken_item == nullptr);
        builder.endBuildDirectory();
    }
    
    SECTION("Non-existent path") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        builder.startBuildDirectory("root");
        
        std::filesystem::path non_existent = test_mockup.base_path / "does_not_exist";
        auto result = builder.buildLink("non_existent", non_existent);
        
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto non_existent_item = tree->getChild("non_existent");
        REQUIRE(non_existent_item == nullptr);
        builder.endBuildDirectory();
    }
}

TEST_CASE("LinkFollowBuilder - Multiple links in same tree", "[LinkFollowBuilder]") {
    SECTION("Build multiple different links") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        builder.startBuildDirectory("root");
        
        auto result1 = builder.buildLink("file_link", test_mockup.test_file);
        auto result2 = builder.buildLink("dir_link", test_mockup.target_dir);
        builder.endBuildDirectory();
        auto result3 = builder.buildLink("nested_file_link", test_mockup.nested_file);
        
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 != nullptr);
        REQUIRE(result2->getName() == "dir_link");
        REQUIRE(result3 == nullptr);
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto file_link = tree->getChild("file_link");
        auto dir_link = tree->getChild("dir_link");
        auto nested_file_link = tree->getChild("nested_file_link");
        
        REQUIRE(file_link != nullptr);
        REQUIRE(dir_link != nullptr);
        REQUIRE(nested_file_link != nullptr);
        
        REQUIRE(file_link->getName() == "file_link");
        REQUIRE(dir_link->getName() == "dir_link");
        REQUIRE(nested_file_link->getName() == "nested_file_link");
        builder.endBuildDirectory();
    }
    
    SECTION("Build multiple links to same target - only first should succeed") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        builder.startBuildDirectory("root");
        
        auto result1 = builder.buildLink("first_link", test_mockup.test_file);
        auto result2 = builder.buildLink("second_link", test_mockup.test_file);
        auto result3 = builder.buildLink("third_link", test_mockup.test_file);
        
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
        REQUIRE(result3 == nullptr);
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto first_link = tree->getChild("first_link");
        auto second_link = tree->getChild("second_link");
        auto third_link = tree->getChild("third_link");
        
        REQUIRE(first_link != nullptr);
        REQUIRE(second_link == nullptr);
        REQUIRE(third_link == nullptr);
        
        REQUIRE(first_link->getName() == "first_link");
        builder.endBuildDirectory();
    }
}

TEST_CASE("LinkFollowBuilder - Directory building behavior", "[LinkFollowBuilder]") {
    SECTION("Directory link creates empty directory structure") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        builder.startBuildDirectory("root");
        
        auto result = builder.buildLink("target_dir", test_mockup.target_dir);
        
        REQUIRE(result != nullptr);
        REQUIRE(result->getName() == "target_dir");
        
        auto tree = builder.getTree();
        auto linked_dir = tree->getChild("target_dir");
        REQUIRE(linked_dir != nullptr);
        
        REQUIRE(linked_dir->getName() == "target_dir");
        builder.endBuildDirectory();
    }
}
