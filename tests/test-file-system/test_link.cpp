#include "file-system-composite/Link.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include "file-system-composite/FileObject.hpp"

#include <catch2/catch_all.hpp>
#include <memory>
#include <filesystem>

TEST_CASE("Link constructor", "[Link]") {
    SECTION("Link with valid directory owner and relative target") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        
        REQUIRE_NOTHROW(Link("link1", "target.txt", root_dir.get()));
        
        Link link("link1", "target.txt", root_dir.get());
        REQUIRE(link.getName() == "link1");
        REQUIRE(link.getPath() == "root/link1");
        REQUIRE(link.getTarget() == "target.txt");
        REQUIRE(link.getOwner() == root_dir.get());
    }
    
    SECTION("Link with absolute target path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        
        Link link("absolute_link", "/usr/bin/executable", root_dir.get());
        REQUIRE(link.getName() == "absolute_link");
        REQUIRE(link.getTarget() == "/usr/bin/executable");
    }
    
    SECTION("Link with null directory owner") {
        // Root link (no parent directory)
        REQUIRE_NOTHROW(Link("root_link", "some_target", nullptr));
        
        Link root_link("root_link", "some_target", nullptr);
        REQUIRE(root_link.getName() == "root_link");
        REQUIRE(root_link.getPath() == "root_link");
        REQUIRE(root_link.getOwner() == nullptr);
    }
    
    SECTION("Link with non-directory owner") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        FileObject* file = root_dir->createFile("parent.txt");
        
        // Should throw when trying to create link with file as parent
        REQUIRE_THROWS_AS(Link("child_link", "target", file), std::runtime_error);
    }
}

TEST_CASE("Link getName functionality", "[Link]") {
    SECTION("Get name from simple link") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("documents");
        Link link("shortcut", "document.pdf", root_dir.get());
        
        REQUIRE(link.getName() == "shortcut");
    }
    
    SECTION("Get name with extension") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("workspace");
        Link link("report.lnk", "report.docx", root_dir.get());
        
        REQUIRE(link.getName() == "report.lnk");
    }
    
    SECTION("Get name without extension") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("bin");
        Link link("executable_link", "/usr/bin/executable", root_dir.get());
        
        REQUIRE(link.getName() == "executable_link");
    }
    
    SECTION("Get name with special characters") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("temp");
        Link link("link-with_special.chars", "target", root_dir.get());
        
        REQUIRE(link.getName() == "link-with_special.chars");
    }
}

TEST_CASE("Link getPath functionality", "[Link]") {
    SECTION("Get full path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("home");
        Link link("config_link", "config.ini", root_dir.get());
        
        std::string path = link.getPath();
        REQUIRE_FALSE(path.empty());
        REQUIRE(path == "home/config_link");
    }
    
    SECTION("Path consistency") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("projects");
        Link link("main_link", "main.cpp", root_dir.get());
        
        std::string path_1 = link.getPath();
        std::string path_2 = link.getPath();
        
        REQUIRE(path_1 == path_2);
        REQUIRE(path_1 == "projects/main_link");
    }
    
    SECTION("Nested directory path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("home");
        FileObject* sub_dir = root_dir->createSubdirectory("user");
        Link link("settings_link", "settings.conf", sub_dir);
        
        REQUIRE(link.getPath() == "home/user/settings_link");
    }
}

TEST_CASE("Link target functionality", "[Link]") {
    SECTION("Get target path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("mylink", "documents/file.txt", root_dir.get());
        
        REQUIRE(link.getTarget() == "documents/file.txt");
    }
    
    SECTION("Target path consistency") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("mylink", "../other/file.txt", root_dir.get());
        
        auto target1 = link.getTarget();
        auto target2 = link.getTarget();
        REQUIRE(target1 == target2);
        REQUIRE(target1 == "../other/file.txt");
    }
    
    SECTION("Empty target path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("empty_link", "", root_dir.get());
        
        REQUIRE(link.getTarget().empty());
    }
}

TEST_CASE("Link target resolution functionality", "[Link]") {
    SECTION("Resolve link to file") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        FileObject* target_file = root_dir->createFile("target.txt");
        target_file->setSize(100);
        
        Link link("link_to_file", "target.txt", root_dir.get());
        
        // Initially unresolved
        REQUIRE(link.getResolvedTarget() == nullptr);
        REQUIRE(link.getSize() == 0);
        
        // Create a copy of the target for resolution (since we can't move the original)
        auto target_copy = std::make_unique<File>("target.txt", root_dir.get());
        target_copy->setSize(100);
        
        // Resolve the link
        bool resolved = link.setResolveTarget(std::move(target_copy));
        REQUIRE(resolved == true);
        
        // Check resolution worked
        REQUIRE(link.getResolvedTarget() != nullptr);
        REQUIRE(link.getResolvedTarget()->getName() == "target.txt");
        REQUIRE(link.getSize() == 100); // Should return target's size
    }
    
    SECTION("Resolve link to directory") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        
        Link link("link_to_dir", "subdir", root_dir.get());
        
        // Create target directory with some content
        auto target_dir = std::make_unique<Directory>("subdir", root_dir.get());
        FileObject* file_in_target = target_dir->createFile("inside.txt");
        file_in_target->setSize(50);
        
        // Resolve the link
        bool resolved = link.setResolveTarget(std::move(target_dir));
        REQUIRE(resolved == true);
        
        // Check resolution
        REQUIRE(link.getResolvedTarget() != nullptr);
        REQUIRE(link.getResolvedTarget()->getName() == "subdir");
        REQUIRE(link.getSize() == 50); // Should return target directory's size
    }
    
    SECTION("Attempt to resolve with null target") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("broken_link", "nonexistent", root_dir.get());
        
        bool resolved = link.setResolveTarget(nullptr);
        REQUIRE(resolved == false);
        REQUIRE(link.getResolvedTarget() == nullptr);
        REQUIRE(link.getSize() == 0);
    }
    
    SECTION("Re-resolve link with different target") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("changeable_link", "first_target", root_dir.get());
        
        // First resolution
        auto first_target = std::make_unique<File>("first.txt", root_dir.get());
        first_target->setSize(100);
        link.setResolveTarget(std::move(first_target));
        
        REQUIRE(link.getSize() == 100);
        REQUIRE(link.getTarget() == "first.txt"); // Target name should be updated
        
        // Second resolution (overwrites first)
        auto second_target = std::make_unique<File>("second.txt", root_dir.get());
        second_target->setSize(200);
        bool resolved = link.setResolveTarget(std::move(second_target));
        
        REQUIRE(resolved == true);
        REQUIRE(link.getSize() == 200);
        REQUIRE(link.getTarget() == "second.txt"); // Target name should be updated
    }
}

TEST_CASE("Link size management", "[Link]") {
    SECTION("Unresolved link has zero size") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("unresolved", "nowhere", root_dir.get());
        
        REQUIRE(link.getSize() == 0);
    }
    
    SECTION("Resolved link returns target size") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("resolved", "target", root_dir.get());
        
        auto target = std::make_unique<File>("target.txt", root_dir.get());
        target->setSize(500);
        
        link.setResolveTarget(std::move(target));
        REQUIRE(link.getSize() == 500);
    }
    
    SECTION("Link size changes with target size") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        Link link("dynamic", "target", root_dir.get());
        
        auto target = std::make_unique<File>("target.txt", root_dir.get());
        target->setSize(100);
        link.setResolveTarget(std::move(target));
        
        REQUIRE(link.getSize() == 100);
        
        // Modify target size through the resolved target
        FileObject* resolved = link.getResolvedTarget();
        resolved->setSize(300);
        
        REQUIRE(link.getSize() == 300);
    }
}

TEST_CASE("Link composite pattern behavior", "[Link]") {
    SECTION("Link cannot add children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        Link link("mylink", "target", root_dir.get());
        
        std::unique_ptr<FileObject> child_dir = std::make_unique<Directory>("child");
        REQUIRE(link.add(std::move(child_dir)) == false);
    }
    
    SECTION("Link cannot remove children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        Link link("mylink", "target", root_dir.get());
        
        REQUIRE(link.remove("anything") == false);
    }
    
    SECTION("Link has no children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        Link link("mylink", "target", root_dir.get());
        
        REQUIRE(link.getChild("nonexistent") == nullptr);
        REQUIRE(link.getChild("anything") == nullptr);
    }
    
    SECTION("Link cannot create children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        Link link("mylink", "target", root_dir.get());
        
        REQUIRE(link.createFile("child.txt") == nullptr);
        REQUIRE(link.createSubdirectory("subdir") == nullptr);
    }
}

TEST_CASE("Link polymorphic behavior", "[Link]") {
    SECTION("Link as FileObject pointer") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("polymorphism");
        std::unique_ptr<FileObject> link_obj = std::make_unique<Link>("poly_link", "target", root_dir.get());
        
        REQUIRE_NOTHROW(link_obj->getName());
        REQUIRE_NOTHROW(link_obj->getPath());
        REQUIRE(link_obj->getSize() == 0);
        
        // Composite methods should return default values
        std::unique_ptr<FileObject> dummy_child = std::make_unique<Directory>("dummy");
        std::string to_remove = dummy_child->getName();
        REQUIRE(link_obj->add(std::move(dummy_child)) == false);
        REQUIRE(link_obj->remove(to_remove) == false);
        REQUIRE(link_obj->getChild("anything") == nullptr);
        
        // Factory methods should return nullptr for links
        REQUIRE(link_obj->createFile("child.txt") == nullptr);
        REQUIRE(link_obj->createSubdirectory("subdir") == nullptr);
        
        // Link-specific methods should work
        REQUIRE(link_obj->getTarget() == "target");
        REQUIRE(link_obj->getResolvedTarget() == nullptr);
    }
}

TEST_CASE("Link edge cases and error handling", "[Link]") {
    SECTION("Empty link name") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        
        REQUIRE_THROWS_AS(Link("", "target", root_dir.get()), std::runtime_error);
    }
    
    SECTION("Very long link name") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        std::string long_name(100, 'a');
        long_name += ".lnk";
        
        Link link(long_name, "target", root_dir.get());
        
        REQUIRE(link.getName() == long_name);
        REQUIRE(link.getPath().find(long_name) != std::string::npos);
    }
    
    SECTION("Link name with special characters") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        std::string special_name = "link_with-special.chars@123.lnk";
        
        Link link(special_name, "target", root_dir.get());
        
        REQUIRE(link.getName() == special_name);
        REQUIRE(link.getPath() == "test/" + special_name);
    }
    
    SECTION("Very long target path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        std::string long_target(500, 'x');
        long_target = "/very/long/path/" + long_target + "/target.txt";
        
        Link link("link", long_target, root_dir.get());
        
        REQUIRE(link.getTarget() == long_target);
    }
}

TEST_CASE("Link memory and resource management", "[Link]") {
    SECTION("Multiple links with same name in different directories") {
        std::unique_ptr<FileObject> dir_1 = std::make_unique<Directory>("dir1");
        std::unique_ptr<FileObject> dir_2 = std::make_unique<Directory>("dir2");
        
        Link link_1("same_link", "target1", dir_1.get());
        Link link_2("same_link", "target2", dir_2.get());
        
        REQUIRE(link_1.getName() == link_2.getName());
        REQUIRE(link_1.getPath() != link_2.getPath());
        REQUIRE(link_1.getPath() == "dir1/same_link");
        REQUIRE(link_2.getPath() == "dir2/same_link");
        REQUIRE(link_1.getTarget() != link_2.getTarget());
    }
    
    SECTION("Link ownership and parent relationship") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("owner_test");
        Link link("owned_link", "target", root_dir.get());
        
        REQUIRE(link.getOwner() == root_dir.get());
    }
    
    SECTION("Link with circular reference prevention") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("circular_test");
        
        Link link1("link1", "link2", root_dir.get());
        Link link2("link2", "link1", root_dir.get());
        
        // Both links should exist but remain unresolved to prevent circular refs
        REQUIRE(link1.getTarget() == "link2");
        REQUIRE(link2.getTarget() == "link1");
        REQUIRE(link1.getResolvedTarget() == nullptr);
        REQUIRE(link2.getResolvedTarget() == nullptr);
    }
}

TEST_CASE("Link integration with directory operations", "[Link]") {
    SECTION("Link can be added to directory manually") {
        Directory parent("root");
        auto link = std::make_unique<Link>("manual_link", "target.txt", &parent);
        
        std::string link_name = link->getName();
        bool added = parent.add(std::move(link));
        REQUIRE(added == true);
        
        FileObject* found_link = parent.getChild(link_name);
        REQUIRE(found_link != nullptr);
        REQUIRE(found_link->getName() == link_name);
    }
    
    SECTION("Link can be removed from directory") {
        Directory parent("root");
        auto link = std::make_unique<Link>("removable_link", "target.txt", &parent);
        
        std::string link_name = link->getName();
        parent.add(std::move(link));
        
        // Verify link exists
        REQUIRE(parent.getChild(link_name) != nullptr);
        
        // Remove link
        bool removed = parent.remove(link_name);
        REQUIRE(removed == true);
        REQUIRE(parent.getChild(link_name) == nullptr);
    }
    
    SECTION("Directory size includes unresolved links as zero") {
        Directory parent("root");
        auto link = std::make_unique<Link>("unresolved_link", "nowhere", &parent);
        parent.add(std::move(link));
        
        FileObject* file = parent.createFile("actual_file.txt");
        file->setSize(100);
        
        // Directory size should only include the file, not the unresolved link
        REQUIRE(parent.getSize() == 100);
    }
    
    SECTION("Directory size includes resolved links") {
        Directory parent("root");
        
        // Create target file
        FileObject* target_file = parent.createFile("target.txt");
        target_file->setSize(200);
        
        // Create link and resolve it
        auto link = std::make_unique<Link>("resolved_link", "target.txt", &parent);
        auto target_copy = std::make_unique<File>("target.txt", &parent);
        target_copy->setSize(200);
        link->setResolveTarget(std::move(target_copy));
        
        parent.add(std::move(link));
        
        // Directory size should include both the original file and the resolved link
        REQUIRE(parent.getSize() == 400); // 200 + 200
    }
}
