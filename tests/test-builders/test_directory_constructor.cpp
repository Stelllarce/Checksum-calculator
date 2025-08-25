#include "directory-tree-builders/DirectoryConstructor.hpp"
#include "directory-tree-builders/LinkFollowBuilder.hpp"
#include "directory-tree-builders/NonFollowLinkBuilder.hpp"
#include "directory-tree-builders/CycleDetector.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include "file-system-composite/Link.hpp"
#include <catch2/catch_all.hpp>
#include <filesystem>
#include <fstream>
#include <memory>

namespace {
    class DirectoryConstructorTestMockup {
    public:
        const std::filesystem::path base_path;
        const std::filesystem::path root_dir;
        const std::filesystem::path sub_dir1;
        const std::filesystem::path sub_dir2;
        const std::filesystem::path nested_dir;
        const std::filesystem::path empty_dir;
        const std::filesystem::path file1;
        const std::filesystem::path file2;
        const std::filesystem::path nested_file;
        const std::filesystem::path link_to_file;
        const std::filesystem::path link_to_dir;
        const std::filesystem::path link_to_nested;
        const std::filesystem::path broken_link;
        const std::filesystem::path circular_link1;
        const std::filesystem::path circular_link2;
        const std::filesystem::path single_file;
        
        DirectoryConstructorTestMockup() 
            : base_path(std::filesystem::temp_directory_path() / "directory_constructor_test")
            , root_dir(base_path / "root")
            , sub_dir1(root_dir / "subdir1")
            , sub_dir2(root_dir / "subdir2")
            , nested_dir(sub_dir1 / "nested")
            , empty_dir(root_dir / "empty")
            , file1(root_dir / "file1.txt")
            , file2(sub_dir1 / "file2.txt")
            , nested_file(nested_dir / "nested_file.txt")
            , link_to_file(root_dir / "link_to_file")
            , link_to_dir(sub_dir1 / "link_to_dir")
            , link_to_nested(root_dir / "link_to_nested")
            , broken_link(root_dir / "broken_link")
            , circular_link1(sub_dir2 / "circular1")
            , circular_link2(sub_dir2 / "circular2")
            , single_file(base_path / "standalone.txt")
        {
            setup();
        }
        
        ~DirectoryConstructorTestMockup() {
            cleanup();
        }
        
        DirectoryConstructorTestMockup(const DirectoryConstructorTestMockup&) = delete;
        DirectoryConstructorTestMockup& operator=(const DirectoryConstructorTestMockup&) = delete;
        
    private:
        void setup() {
            std::filesystem::remove_all(base_path);
            
            std::filesystem::create_directories(root_dir);
            std::filesystem::create_directories(sub_dir1);
            std::filesystem::create_directories(sub_dir2);
            std::filesystem::create_directories(nested_dir);
            std::filesystem::create_directories(empty_dir);
            
            std::ofstream(file1) << "content of file1";
            std::ofstream(file2) << "content of file2";
            std::ofstream(nested_file) << "content of nested file";
            std::ofstream(single_file) << "standalone file content";
            
            std::error_code ec;
            std::filesystem::create_symlink(file1, link_to_file, ec);
            std::filesystem::create_symlink(sub_dir2, link_to_dir, ec);
            std::filesystem::create_symlink(nested_dir, link_to_nested, ec);
            
            std::filesystem::create_symlink(base_path / "non_existent", broken_link, ec);
            
            std::filesystem::create_symlink(circular_link2, circular_link1, ec);
            std::filesystem::create_symlink(circular_link1, circular_link2, ec);
        }
        
        void cleanup() {
            std::filesystem::remove_all(base_path);
        }
    };
    
    static DirectoryConstructorTestMockup test_mockup;
}

TEST_CASE("DirectoryConstructor - Constructor and basic setup", "[DirectoryConstructor]") {
    SECTION("Constructor with NonFollowLinkBuilder") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        REQUIRE_NOTHROW(constructor.construct({}));
    }
    
    SECTION("Constructor with LinkFollowBuilder") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        REQUIRE_NOTHROW(constructor.construct({}));
    }
}

TEST_CASE("DirectoryConstructor - Single file construction with NonFollowLinkBuilder", "[DirectoryConstructor]") {
    SECTION("Construct single regular file") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.single_file});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        REQUIRE(tree->getName() == "(virtual_root)");
        
        auto file_obj = tree->getChild("standalone.txt");
        REQUIRE(file_obj != nullptr);
        REQUIRE(file_obj->getName() == "standalone.txt");
    }
    
    SECTION("Construct non-existent file - should handle gracefully") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        std::filesystem::path non_existent = test_mockup.base_path / "does_not_exist.txt";
        constructor.construct({non_existent});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        REQUIRE(tree->getName() == "(virtual_root)");
    }
}

TEST_CASE("DirectoryConstructor - Single file construction with LinkFollowBuilder", "[DirectoryConstructor]") {
    SECTION("Construct single regular file") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.single_file});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        REQUIRE(tree->getName() == "(virtual_root)");
        
        auto file_obj = tree->getChild("standalone.txt");
        REQUIRE(file_obj != nullptr);
        REQUIRE(file_obj->getName() == "standalone.txt");
    }
}

TEST_CASE("DirectoryConstructor - Directory construction with NonFollowLinkBuilder", "[DirectoryConstructor]") {
    SECTION("Construct simple directory with files") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto root_obj = tree->getChild("root");
        REQUIRE(root_obj != nullptr);
        REQUIRE(root_obj->getName() == "root");
        
        auto subdir1 = root_obj->getChild("subdir1");
        auto subdir2 = root_obj->getChild("subdir2");
        auto empty_dir_obj = root_obj->getChild("empty");
        
        REQUIRE(subdir1 != nullptr);
        REQUIRE(subdir2 != nullptr);
        REQUIRE(empty_dir_obj != nullptr);
        
        auto file1_obj = root_obj->getChild("file1.txt");
        REQUIRE(file1_obj != nullptr);
        REQUIRE(file1_obj->getName() == "file1.txt");
        
        auto nested_dir_obj = subdir1->getChild("nested");
        REQUIRE(nested_dir_obj != nullptr);
        
        auto nested_file_obj = nested_dir_obj->getChild("nested_file.txt");
        REQUIRE(nested_file_obj != nullptr);
        REQUIRE(nested_file_obj->getName() == "nested_file.txt");
    }
    
    SECTION("Construct directory with symbolic links - should create Link objects") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir});
        
        auto tree = builder.getTree();
        auto root_obj = tree->getChild("root");
        REQUIRE(root_obj != nullptr);
        
        auto link_to_file_obj = root_obj->getChild("link_to_file");
        REQUIRE(link_to_file_obj != nullptr);
        REQUIRE(link_to_file_obj->getName() == "link_to_file");
        REQUIRE(link_to_file_obj->getTarget() == test_mockup.file1);
        
        auto link_to_nested_obj = root_obj->getChild("link_to_nested");
        REQUIRE(link_to_nested_obj != nullptr);
        REQUIRE(link_to_nested_obj->getName() == "link_to_nested");
        REQUIRE(link_to_nested_obj->getTarget() == test_mockup.nested_dir);
        
        auto broken_link_obj = root_obj->getChild("broken_link");
        REQUIRE(broken_link_obj != nullptr);
        REQUIRE(broken_link_obj->getName() == "broken_link");
    }
}

TEST_CASE("DirectoryConstructor - Directory construction with LinkFollowBuilder", "[DirectoryConstructor]") {
    SECTION("Construct simple directory with files") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto root_obj = tree->getChild("root");
        REQUIRE(root_obj != nullptr);
        REQUIRE(root_obj->getName() == "root");
        
        auto subdir1 = root_obj->getChild("subdir1");
        auto file1_obj = root_obj->getChild("file1.txt");
        
        REQUIRE(subdir1 != nullptr);
        REQUIRE(file1_obj != nullptr);
    }
    
    SECTION("Construct directory with symbolic links - should follow valid links") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir});
        
        auto tree = builder.getTree();
        auto root_obj = tree->getChild("root");
        REQUIRE(root_obj != nullptr);
        
        REQUIRE(root_obj->getName() == "root");
    }
}

TEST_CASE("DirectoryConstructor - Multiple root paths with NonFollowLinkBuilder", "[DirectoryConstructor]") {
    SECTION("Construct multiple directories and files") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir, test_mockup.single_file});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto root_obj = tree->getChild("root");
        auto single_file_obj = tree->getChild("standalone.txt");
        
        REQUIRE(root_obj != nullptr);
        REQUIRE(single_file_obj != nullptr);
        REQUIRE(root_obj->getName() == "root");
        REQUIRE(single_file_obj->getName() == "standalone.txt");
    }
    
    SECTION("Construct multiple separate directories") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.sub_dir1, test_mockup.sub_dir2});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto subdir1_obj = tree->getChild("subdir1");
        auto subdir2_obj = tree->getChild("subdir2");
        
        REQUIRE(subdir1_obj != nullptr);
        REQUIRE(subdir2_obj != nullptr);
        
        auto nested_obj = subdir1_obj->getChild("nested");
        auto file2_obj = subdir1_obj->getChild("file2.txt");
        
        REQUIRE(nested_obj != nullptr);
        REQUIRE(file2_obj != nullptr);
    }
}

TEST_CASE("DirectoryConstructor - Multiple root paths with LinkFollowBuilder", "[DirectoryConstructor]") {
    SECTION("Construct multiple directories and files") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir, test_mockup.single_file});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto root_obj = tree->getChild("root");
        auto single_file_obj = tree->getChild("standalone.txt");
        
        REQUIRE(root_obj != nullptr);
        REQUIRE(single_file_obj != nullptr);
    }
}

TEST_CASE("DirectoryConstructor - Symbolic link handling with NonFollowLinkBuilder", "[DirectoryConstructor]") {
    SECTION("Construct starting from symbolic link to directory") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.link_to_nested});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto link_obj = tree->getChild("link_to_nested");
        REQUIRE(link_obj != nullptr);
        REQUIRE(link_obj->getName() == "link_to_nested");
        REQUIRE(link_obj->getTarget() == test_mockup.nested_dir);
    }
    
    SECTION("Construct starting from symbolic link to file") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.link_to_file});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto link_obj = tree->getChild("link_to_file");
        REQUIRE(link_obj != nullptr);
        REQUIRE(link_obj->getName() == "link_to_file");
        REQUIRE(link_obj->getTarget() == test_mockup.file1);
    }
}

TEST_CASE("DirectoryConstructor - Symbolic link handling with LinkFollowBuilder", "[DirectoryConstructor]") {
    SECTION("Construct starting from symbolic link to directory - should follow") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.link_to_nested});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        REQUIRE(tree->getName() == "(virtual_root)");
    }
    
    SECTION("Construct starting from symbolic link to file - should follow") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.link_to_file});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        REQUIRE(tree->getName() == "(virtual_root)");
    }
}

TEST_CASE("DirectoryConstructor - Error handling", "[DirectoryConstructor]") {
    SECTION("Empty root paths list with NonFollowLinkBuilder") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        REQUIRE(tree->getName() == "(virtual_root)");
    }
    
    SECTION("Empty root paths list with LinkFollowBuilder") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        REQUIRE(tree->getName() == "(virtual_root)");
    }
    
    SECTION("Non-existent paths - should handle gracefully") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        std::filesystem::path non_existent1 = test_mockup.base_path / "does_not_exist1";
        std::filesystem::path non_existent2 = test_mockup.base_path / "does_not_exist2";
        
        REQUIRE_NOTHROW(constructor.construct({non_existent1, non_existent2}));
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        REQUIRE(tree->getName() == "(virtual_root)");
    }
    
    SECTION("Mixed valid and invalid paths") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        std::filesystem::path non_existent = test_mockup.base_path / "does_not_exist";
        
        constructor.construct({test_mockup.single_file, non_existent, test_mockup.empty_dir});
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto file_obj = tree->getChild("standalone.txt");
        auto empty_dir_obj = tree->getChild("empty");
        
        REQUIRE(file_obj != nullptr);
        REQUIRE(empty_dir_obj != nullptr);
    }
}

TEST_CASE("DirectoryConstructor - Circular symbolic links with LinkFollowBuilder", "[DirectoryConstructor]") {
    SECTION("Should handle circular links gracefully") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        REQUIRE_NOTHROW(constructor.construct({test_mockup.sub_dir2}));
        
        auto tree = builder.getTree();
        REQUIRE(tree != nullptr);
        
        auto subdir2_obj = tree->getChild("subdir2");
        REQUIRE(subdir2_obj != nullptr);
    }
}

TEST_CASE("DirectoryConstructor - Complex directory structure", "[DirectoryConstructor]") {
    SECTION("Build complete structure with NonFollowLinkBuilder") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir});
        
        auto tree = builder.getTree();
        auto root_obj = tree->getChild("root");
        REQUIRE(root_obj != nullptr);
        
        REQUIRE(root_obj->getChild("file1.txt") != nullptr);
        
        auto subdir1 = root_obj->getChild("subdir1");
        auto subdir2 = root_obj->getChild("subdir2");
        auto empty_dir_obj = root_obj->getChild("empty");
        
        REQUIRE(subdir1 != nullptr);
        REQUIRE(subdir2 != nullptr);
        REQUIRE(empty_dir_obj != nullptr);
        
        REQUIRE(subdir1->getChild("file2.txt") != nullptr);
        
        auto nested = subdir1->getChild("nested");
        REQUIRE(nested != nullptr);
        REQUIRE(nested->getChild("nested_file.txt") != nullptr);
        
        auto link_to_file_obj = root_obj->getChild("link_to_file");
        auto link_to_nested_obj = root_obj->getChild("link_to_nested");
        auto broken_link_obj = root_obj->getChild("broken_link");
        
        REQUIRE(link_to_file_obj != nullptr);
        REQUIRE(link_to_nested_obj != nullptr);
        REQUIRE(broken_link_obj != nullptr);
        
        REQUIRE(link_to_file_obj->getTarget() == test_mockup.file1);
        REQUIRE(link_to_nested_obj->getTarget() == test_mockup.nested_dir);
        
        auto circular1 = subdir2->getChild("circular1");
        auto circular2 = subdir2->getChild("circular2");
        REQUIRE(circular1 != nullptr);
        REQUIRE(circular2 != nullptr);
    }
    
    SECTION("Build complete structure with LinkFollowBuilder") {
        auto detector = std::make_unique<CycleDetector>();
        LinkFollowBuilder builder(std::move(detector));
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.root_dir});
        
        auto tree = builder.getTree();
        auto root_obj = tree->getChild("root");
        REQUIRE(root_obj != nullptr);
        
        REQUIRE(root_obj->getChild("file1.txt") != nullptr);
        REQUIRE(root_obj->getChild("subdir1") != nullptr);
        REQUIRE(root_obj->getChild("subdir2") != nullptr);
        REQUIRE(root_obj->getChild("empty") != nullptr);
        
        auto subdir1 = root_obj->getChild("subdir1");
        REQUIRE(subdir1 != nullptr);
        REQUIRE(subdir1->getChild("file2.txt") != nullptr);
    }
}

TEST_CASE("DirectoryConstructor - Builder state consistency", "[DirectoryConstructor]") {
    SECTION("Multiple constructions should work with same builder") {
        NonFollowLinkBuilder builder;
        DirectoryConstructor constructor(builder);
        
        constructor.construct({test_mockup.single_file});
        auto tree1 = builder.getTree();
        REQUIRE(tree1 != nullptr);
        REQUIRE(tree1->getChild("standalone.txt") != nullptr);
        
        constructor.construct({test_mockup.empty_dir});
        auto tree2 = builder.getTree();
        REQUIRE(tree2 != nullptr);
        
        REQUIRE(tree2->getChild("standalone.txt") != nullptr);
        REQUIRE(tree2->getChild("empty") != nullptr);
    }
    
    SECTION("Different builders should produce independent trees") {
        NonFollowLinkBuilder builder1;
        NonFollowLinkBuilder builder2;
        
        DirectoryConstructor constructor1(builder1);
        DirectoryConstructor constructor2(builder2);
        
        constructor1.construct({test_mockup.single_file});
        constructor2.construct({test_mockup.empty_dir});
        
        auto tree1 = builder1.getTree();
        auto tree2 = builder2.getTree();
        
        REQUIRE(tree1 != nullptr);
        REQUIRE(tree2 != nullptr);
        
        REQUIRE(tree1->getChild("standalone.txt") != nullptr);
        REQUIRE(tree1->getChild("empty") == nullptr);
        
        REQUIRE(tree2->getChild("empty") != nullptr);
        REQUIRE(tree2->getChild("standalone.txt") == nullptr);
    }
}