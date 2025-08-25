#include "directory-tree-builders/NonFollowLinkBuilder.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/Link.hpp"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>

namespace {
    class NonFollowLinkBuilderTestMockup {
    public:
        const std::filesystem::path base_path;
        const std::filesystem::path source_dir;
        const std::filesystem::path target_dir;
        const std::filesystem::path test_file;
        const std::filesystem::path nested_file;
        const std::filesystem::path link_to_file;
        const std::filesystem::path link_to_dir;
        const std::filesystem::path broken_link;
        const std::filesystem::path relative_path;
        const std::filesystem::path absolute_path;
        
        NonFollowLinkBuilderTestMockup() 
            : base_path(std::filesystem::temp_directory_path() / "non_follow_link_builder_test")
            , source_dir(base_path / "source")
            , target_dir(base_path / "target")
            , test_file(target_dir / "test_file.txt")
            , nested_file(target_dir / "nested" / "nested_file.txt")
            , link_to_file(source_dir / "link_to_file")
            , link_to_dir(source_dir / "link_to_dir")
            , broken_link(source_dir / "broken_link")
            , relative_path("../target/test_file.txt")
            , absolute_path(test_file)
        {
            setup();
        }
        
        ~NonFollowLinkBuilderTestMockup() {
            cleanup();
        }
        
        NonFollowLinkBuilderTestMockup(const NonFollowLinkBuilderTestMockup&) = delete;
        NonFollowLinkBuilderTestMockup& operator=(const NonFollowLinkBuilderTestMockup&) = delete;
        
    private:
        void setup() {
            std::filesystem::remove_all(base_path);
            
            std::filesystem::create_directories(source_dir);
            std::filesystem::create_directories(target_dir);
            std::filesystem::create_directories(target_dir / "nested");
            
            std::ofstream(test_file) << "test file content";
            std::ofstream(nested_file) << "nested file content";
            
            std::error_code ec;
            std::filesystem::create_symlink(test_file, link_to_file, ec);
            std::filesystem::create_symlink(target_dir, link_to_dir, ec);
            std::filesystem::create_symlink(base_path / "non_existent", broken_link, ec);
        }
        
        void cleanup() {
            std::filesystem::remove_all(base_path);
        }
    };
    
    static NonFollowLinkBuilderTestMockup test_mockup;
}

TEST_CASE("NonFollowLinkBuilder - Constructor and basic setup", "[NonFollowLinkBuilder]") {
    SECTION("Default constructor creates valid builder") {
        NonFollowLinkBuilder builder;
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        REQUIRE(tree->getName() == "(virtual_root)");
    }
}

TEST_CASE("NonFollowLinkBuilder - Creating links to files", "[NonFollowLinkBuilder]") {
    SECTION("Create link to regular file") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("file_link", test_mockup.test_file);
        
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto link_object = tree->getChild("file_link");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "file_link");
        
        REQUIRE(link_object->getTarget() == test_mockup.test_file);
    }
    
    SECTION("Create link with relative path") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("relative_link", test_mockup.relative_path);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("relative_link");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "relative_link");
        REQUIRE(link_object->getTarget() == test_mockup.relative_path);
    }
    
    SECTION("Create link with absolute path") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("absolute_link", test_mockup.absolute_path);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("absolute_link");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "absolute_link");
        REQUIRE(link_object->getTarget() == test_mockup.absolute_path);
    }
}

TEST_CASE("NonFollowLinkBuilder - Creating links to directories", "[NonFollowLinkBuilder]") {
    SECTION("Create link to directory") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("dir_link", test_mockup.target_dir);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("dir_link");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "dir_link");
        REQUIRE(link_object->getTarget() == test_mockup.target_dir);
    }
}

TEST_CASE("NonFollowLinkBuilder - Creating links to symbolic links", "[NonFollowLinkBuilder]") {
    SECTION("Create link to symbolic link pointing to file") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("symlink_to_file", test_mockup.link_to_file);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("symlink_to_file");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "symlink_to_file");
        REQUIRE(link_object->getTarget() == test_mockup.link_to_file);
    }
    
    SECTION("Create link to symbolic link pointing to directory") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("symlink_to_dir", test_mockup.link_to_dir);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("symlink_to_dir");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "symlink_to_dir");
        REQUIRE(link_object->getTarget() == test_mockup.link_to_dir);
    }
}

TEST_CASE("NonFollowLinkBuilder - Handling non-existent targets", "[NonFollowLinkBuilder]") {
    SECTION("Create link to non-existent file") {
        NonFollowLinkBuilder builder;
        
        std::filesystem::path non_existent = test_mockup.base_path / "does_not_exist.txt";
        auto result = builder.buildLink("broken_link", non_existent);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("broken_link");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "broken_link");
        REQUIRE(link_object->getTarget() == non_existent);
        
    }
    
    SECTION("Create link to broken symbolic link") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("link_to_broken", test_mockup.broken_link);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("link_to_broken");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "link_to_broken");
        REQUIRE(link_object->getTarget() == test_mockup.broken_link);
    }
}

TEST_CASE("NonFollowLinkBuilder - Multiple links in same tree", "[NonFollowLinkBuilder]") {
    SECTION("Create multiple different links") {
        NonFollowLinkBuilder builder;
        
        auto result1 = builder.buildLink("file_link", test_mockup.test_file);
        auto result2 = builder.buildLink("dir_link", test_mockup.target_dir);
        auto result3 = builder.buildLink("symlink_link", test_mockup.link_to_file);
        auto result4 = builder.buildLink("relative_link", test_mockup.relative_path);
        
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
        REQUIRE(result3 == nullptr);
        REQUIRE(result4 == nullptr);
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto file_link = tree->getChild("file_link");
        auto dir_link = tree->getChild("dir_link");
        auto symlink_link = tree->getChild("symlink_link");
        auto relative_link = tree->getChild("relative_link");
        
        REQUIRE(file_link != nullptr);
        REQUIRE(dir_link != nullptr);
        REQUIRE(symlink_link != nullptr);
        REQUIRE(relative_link != nullptr);
        
        REQUIRE(file_link->getName() == "file_link");
        REQUIRE(dir_link->getName() == "dir_link");
        REQUIRE(symlink_link->getName() == "symlink_link");
        REQUIRE(relative_link->getName() == "relative_link");
        
        REQUIRE(file_link->getTarget() == test_mockup.test_file);
        REQUIRE(dir_link->getTarget() == test_mockup.target_dir);
        REQUIRE(symlink_link->getTarget() == test_mockup.link_to_file);
        REQUIRE(relative_link->getTarget() == test_mockup.relative_path);
    }
}

TEST_CASE("NonFollowLinkBuilder - Links within directory structure", "[NonFollowLinkBuilder]") {
    SECTION("Create links within nested directories") {
        NonFollowLinkBuilder builder;
        
        builder.startBuildDirectory("parent");
        auto result1 = builder.buildLink("child_link", test_mockup.test_file);
        REQUIRE(result1 == nullptr);
        
        builder.startBuildDirectory("nested");
        auto result2 = builder.buildLink("nested_link", test_mockup.target_dir);
        REQUIRE(result2 == nullptr);
        
        builder.endBuildDirectory();
        builder.endBuildDirectory();
        
        auto tree = builder.getTree();
        auto parent = tree->getChild("parent");
        REQUIRE(parent != nullptr);
        
        auto child_link = parent->getChild("child_link");
        REQUIRE(child_link != nullptr);
        REQUIRE(child_link->getName() == "child_link");
        REQUIRE(child_link->getTarget() == test_mockup.test_file);
        
        auto nested = parent->getChild("nested");
        REQUIRE(nested != nullptr);
        
        auto nested_link = nested->getChild("nested_link");
        REQUIRE(nested_link != nullptr);
        REQUIRE(nested_link->getName() == "nested_link");
        REQUIRE(nested_link->getTarget() == test_mockup.target_dir);
    }
}

TEST_CASE("NonFollowLinkBuilder - Mixed content in directories", "[NonFollowLinkBuilder]") {
    SECTION("Mix files, directories, and links") {
        NonFollowLinkBuilder builder;
        
        builder.buildFile("regular_file.txt");
        auto result1 = builder.buildLink("file_link", test_mockup.test_file);
        REQUIRE(result1 == nullptr);
        
        builder.startBuildDirectory("subdir");
        auto result2 = builder.buildLink("nested_link", test_mockup.relative_path);
        REQUIRE(result2 == nullptr);
        
        builder.buildFile("nested_file.txt");
        builder.endBuildDirectory();
        
        auto result3 = builder.buildLink("dir_link", test_mockup.target_dir);
        REQUIRE(result3 == nullptr);
        
        auto tree = builder.getTree();
        
        auto regularFile = tree->getChild("regular_file.txt");
        auto file_link = tree->getChild("file_link");
        auto subdir = tree->getChild("subdir");
        auto dir_link = tree->getChild("dir_link");
        
        REQUIRE(regularFile != nullptr);
        REQUIRE(file_link != nullptr);
        REQUIRE(subdir != nullptr);
        REQUIRE(dir_link != nullptr);
        
        REQUIRE(file_link->getTarget() == test_mockup.test_file);
        REQUIRE(dir_link->getTarget() == test_mockup.target_dir);
        
        auto nested_link = subdir->getChild("nested_link");
        auto nestedFile = subdir->getChild("nested_file.txt");
        REQUIRE(nested_link != nullptr);
        REQUIRE(nestedFile != nullptr);
        REQUIRE(nested_link->getTarget() == test_mockup.relative_path);
    }
}

TEST_CASE("NonFollowLinkBuilder - Link object behavior", "[NonFollowLinkBuilder]") {
    SECTION("Verify Link objects have correct properties") {
        NonFollowLinkBuilder builder;
        
        auto result = builder.buildLink("test_link", test_mockup.test_file);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("test_link");
        REQUIRE(link_object != nullptr);
        
        REQUIRE(link_object->getName() == "test_link");
        REQUIRE(link_object->getTarget() == test_mockup.test_file);
        
        REQUIRE(link_object->getResolvedTarget() == nullptr);
        
    }
}

TEST_CASE("NonFollowLinkBuilder - Edge cases", "[NonFollowLinkBuilder]") {
    SECTION("Empty path as target") {
        NonFollowLinkBuilder builder;
        
        std::filesystem::path emptyPath;
        auto result = builder.buildLink("empty_target", emptyPath);
        REQUIRE(result == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("empty_target");
        REQUIRE(link_object != nullptr);
        REQUIRE(link_object->getName() == "empty_target");
        REQUIRE(link_object->getTarget() == emptyPath);
    }
    
    SECTION("Same name for multiple links - last one wins") {
        NonFollowLinkBuilder builder;
        
        auto result1 = builder.buildLink("duplicate", test_mockup.test_file);
        auto result2 = builder.buildLink("duplicate", test_mockup.target_dir);
        
        REQUIRE(result1 == nullptr);
        REQUIRE(result2 == nullptr);
        
        auto tree = builder.getTree();
        auto link_object = tree->getChild("duplicate");
        REQUIRE(link_object != nullptr);
        
        REQUIRE(link_object->getName() == "duplicate");
    }
}
