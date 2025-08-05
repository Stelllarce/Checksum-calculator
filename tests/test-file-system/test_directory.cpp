#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include "file-system-composite/FileObject.hpp"

#include <catch2/catch_all.hpp>
#include <memory>
#include <sstream>

TEST_CASE("Directory constructor", "[Directory]")
{
    SECTION("Root directory with no owner")
    {
        REQUIRE_NOTHROW(Directory("root"));

        Directory root("root");
        REQUIRE(root.getName() == "root");
        REQUIRE(root.getPath() == "root");
        REQUIRE(root.getOwner() == nullptr);
    }

    SECTION("Subdirectory with parent owner")
    {
        std::unique_ptr<FileObject> root = std::make_unique<Directory>("root");
        Directory *subdir = root->createSubdirectory("subdir");

        REQUIRE(subdir != nullptr);
        REQUIRE(subdir->getName() == "subdir");
        REQUIRE(subdir->getPath() == "root/subdir");
        REQUIRE(subdir->getOwner() == root.get());
    }

    SECTION("Directory with empty name")
    {
        REQUIRE_THROWS_AS(Directory(""), std::runtime_error);
    }
}

TEST_CASE("Directory getName functionality", "[Directory]")
{
    SECTION("Simple directory name")
    {
        Directory dir("documents");
        REQUIRE(dir.getName() == "documents");
    }

    SECTION("Directory name with special characters")
    {
        Directory dir("my-folder_2024");
        REQUIRE(dir.getName() == "my-folder_2024");
    }

    SECTION("Directory name consistency after operations")
    {
        Directory dir("workspace");
        dir.createFile("test.txt");
        dir.createSubdirectory("subdir");

        // Name should remain unchanged after adding children
        REQUIRE(dir.getName() == "workspace");
    }
}

TEST_CASE("Directory getPath functionality", "[Directory]")
{
    SECTION("Root directory path")
    {
        Directory root("root");
        REQUIRE(root.getPath() == "root");
    }

    SECTION("Nested directory path")
    {
        std::unique_ptr<FileObject> root = std::make_unique<Directory>("home");
        Directory *user_dir = root->createSubdirectory("user");
        Directory *docs_dir = user_dir->createSubdirectory("documents");

        REQUIRE(user_dir->getPath() == "home/user");
        REQUIRE(docs_dir->getPath() == "home/user/documents");
    }

    SECTION("Path consistency")
    {
        Directory dir("project");
        std::string original_path = dir.getPath();

        // Adding children shouldn't change the directory's own path
        dir.createFile("readme.txt");
        REQUIRE(dir.getPath() == original_path);
    }
}

TEST_CASE("Directory createSubdirectory functionality", "[Directory]")
{
    SECTION("Create single subdirectory")
    {
        Directory parent("parent");
        Directory *child = parent.createSubdirectory("child");

        REQUIRE(child != nullptr);
        REQUIRE(child->getName() == "child");
        REQUIRE(child->getPath() == "parent/child");
        REQUIRE(child->getOwner() == &parent);
    }

    SECTION("Create multiple subdirectories")
    {
        Directory parent("root");
        Directory *dir1 = parent.createSubdirectory("dir1");
        Directory *dir2 = parent.createSubdirectory("dir2");
        Directory *dir3 = parent.createSubdirectory("dir3");

        REQUIRE(dir1 != nullptr);
        REQUIRE(dir2 != nullptr);
        REQUIRE(dir3 != nullptr);
        REQUIRE(dir1->getName() == "dir1");
        REQUIRE(dir2->getName() == "dir2");
        REQUIRE(dir3->getName() == "dir3");
    }

    SECTION("Create subdirectory with duplicate name should handle appropriately")
    {
        Directory parent("root");
        REQUIRE_NOTHROW(parent.createSubdirectory("duplicate"));
        REQUIRE_THROWS_AS(parent.createSubdirectory("duplicate"), std::runtime_error);
    }

    SECTION("Create nested subdirectories")
    {
        Directory root("root");
        Directory *level1 = root.createSubdirectory("level1");
        Directory *level2 = level1->createSubdirectory("level2");
        Directory *level3 = level2->createSubdirectory("level3");

        REQUIRE(level3->getPath() == "root/level1/level2/level3");
        REQUIRE(level3->getOwner() == level2);
    }
}

TEST_CASE("Directory createFile functionality", "[Directory]")
{
    SECTION("Create single file")
    {
        Directory parent("documents");
        File *file = parent.createFile("readme.txt");

        REQUIRE(file != nullptr);
        REQUIRE(file->getName() == "readme.txt");
        REQUIRE(file->getPath() == "documents/readme.txt");
        REQUIRE(file->getOwner() == &parent);
    }

    SECTION("Create multiple files")
    {
        Directory parent("project");
        File *file1 = parent.createFile("main.cpp");
        File *file2 = parent.createFile("header.h");
        File *file3 = parent.createFile("makefile");

        REQUIRE(file1 != nullptr);
        REQUIRE(file2 != nullptr);
        REQUIRE(file3 != nullptr);
        REQUIRE(file1->getName() == "main.cpp");
        REQUIRE(file2->getName() == "header.h");
        REQUIRE(file3->getName() == "makefile");
    }

    SECTION("Create file with various extensions")
    {
        Directory parent("media");
        File *txt_file = parent.createFile("document.txt");
        File *img_file = parent.createFile("photo.jpg");
        File *no_ext_file = parent.createFile("README");

        REQUIRE(txt_file->getName() == "document.txt");
        REQUIRE(img_file->getName() == "photo.jpg");
        REQUIRE(no_ext_file->getName() == "README");
    }
}

TEST_CASE("Directory add functionality", "[Directory]")
{
    SECTION("Add file object to directory")
    {
        Directory parent("root");
        std::unique_ptr<FileObject> child_dir = std::make_unique<Directory>("child");

        bool result = parent.add(std::move(child_dir));
        REQUIRE(result == true);
        REQUIRE(child_dir == nullptr); // Ownership should be transferred
    }

    SECTION("Add multiple objects")
    {
        Directory parent("root");
        std::unique_ptr<FileObject> dir1 = std::make_unique<Directory>("dir1");
        std::unique_ptr<FileObject> dir2 = std::make_unique<Directory>("dir2");

        REQUIRE(parent.add(std::move(dir1)) == true);
        REQUIRE(parent.add(std::move(dir2)) == true);
    }

    SECTION("Add object with duplicate name behavior")
    {
        Directory parent("root");
        std::unique_ptr<FileObject> first = std::make_unique<Directory>("same_name");
        std::unique_ptr<FileObject> second = std::make_unique<Directory>("same_name");

        bool first_result = parent.add(std::move(first));
        bool second_result = parent.add(std::move(second));

        REQUIRE(first_result == true);
        REQUIRE(second_result == false);
    }
}

TEST_CASE("Directory remove functionality", "[Directory]")
{
    SECTION("Remove existing subdirectory by name")
    {
        Directory parent("root");
        Directory *child = parent.createSubdirectory("to_remove");
        REQUIRE(child != nullptr);

        // Verify the child exists before removal
        FileObject *found_child = parent.getChild("to_remove");
        REQUIRE(found_child != nullptr);

        // Remove the child by name
        bool result = parent.remove("to_remove");
        REQUIRE(result == true);

        // Verify the child no longer exists
        FileObject *removed_child = parent.getChild("to_remove");
        REQUIRE(removed_child == nullptr);
    }

    SECTION("Remove existing file by name")
    {
        Directory parent("root");
        File *file = parent.createFile("file_to_remove.txt");
        REQUIRE(file != nullptr);

        // Verify the file exists before removal
        FileObject *found_file = parent.getChild("file_to_remove.txt");
        REQUIRE(found_file != nullptr);

        // Remove the file by name
        bool result = parent.remove("file_to_remove.txt");
        REQUIRE(result == true);

        // Verify the file no longer exists
        FileObject *removed_file = parent.getChild("file_to_remove.txt");
        REQUIRE(removed_file == nullptr);
    }

    SECTION("Remove non-existent child")
    {
        Directory parent("root");
        parent.createSubdirectory("existing_child");

        bool result = parent.remove("not_here");
        REQUIRE(result == false);

        // Verify existing child is still there
        FileObject *existing = parent.getChild("existing_child");
        REQUIRE(existing != nullptr);
    }

    SECTION("Remove from empty directory")
    {
        Directory empty_dir("empty");

        bool result = empty_dir.remove("anything");
        REQUIRE(result == false);
    }

    SECTION("Remove with empty string name")
    {
        Directory parent("root");
        parent.createFile("valid_file.txt");

        bool result = parent.remove("");
        REQUIRE(result == false);

        // Verify existing file is still there
        FileObject *existing = parent.getChild("valid_file.txt");
        REQUIRE(existing != nullptr);
    }

    SECTION("Remove multiple children")
    {
        Directory parent("root");
        parent.createSubdirectory("dir1");
        parent.createSubdirectory("dir2");
        parent.createFile("file1.txt");
        parent.createFile("file2.txt");

        // Remove some children
        REQUIRE(parent.remove("dir1") == true);
        REQUIRE(parent.remove("file1.txt") == true);

        // Verify removed children don't exist
        REQUIRE(parent.getChild("dir1") == nullptr);
        REQUIRE(parent.getChild("file1.txt") == nullptr);

        // Verify remaining children still exist
        REQUIRE(parent.getChild("dir2") != nullptr);
        REQUIRE(parent.getChild("file2.txt") != nullptr);
    }

    SECTION("Remove and re-add with same name")
    {
        Directory parent("root");
        parent.createSubdirectory("reusable_name");

        // Remove the directory
        REQUIRE(parent.remove("reusable_name") == true);
        REQUIRE(parent.getChild("reusable_name") == nullptr);

        // Create a new object with the same name
        Directory *new_child = parent.createSubdirectory("reusable_name");
        REQUIRE(new_child != nullptr);
        REQUIRE(parent.getChild("reusable_name") != nullptr);
    }
}

TEST_CASE("Directory getChild functionality", "[Directory]")
{
    SECTION("Get existing child by name")
    {
        Directory parent("root");
        parent.createSubdirectory("child_dir");
        parent.createFile("child_file.txt");

        FileObject *dir_child = parent.getChild("child_dir");
        FileObject *file_child = parent.getChild("child_file.txt");

        REQUIRE(dir_child != nullptr);
        REQUIRE(file_child != nullptr);
        REQUIRE(dir_child->getName() == "child_dir");
        REQUIRE(file_child->getName() == "child_file.txt");
    }

    SECTION("Get non-existent child")
    {
        Directory parent("root");
        parent.createFile("existing.txt");

        FileObject *non_existent = parent.getChild("not_here.txt");
        REQUIRE(non_existent == nullptr);
    }

    SECTION("Get child from empty directory")
    {
        Directory empty_dir("empty");

        FileObject *child = empty_dir.getChild("anything");
        REQUIRE(child == nullptr);
    }

    SECTION("Get child with full path vs name")
    {
        Directory parent("root");
        parent.createFile("test.txt");

        // Should work with just the name
        FileObject *by_name = parent.getChild("test.txt");
        REQUIRE(by_name != nullptr);

        // Behavior with full path depends on implementation
        FileObject *by_full_path = parent.getChild("root/test.txt");
        // Implementation may or may not support full paths
    }

    SECTION("Const version of getChild")
    {
        Directory parent("root");
        parent.createFile("readonly.txt");

        const Directory &const_parent = parent;
        const FileObject *child = const_parent.getChild("readonly.txt");

        REQUIRE(child != nullptr);
        REQUIRE(child->getName() == "readonly.txt");
    }
}

TEST_CASE("Directory getSize functionality", "[Directory]")
{
    SECTION("Empty directory size")
    {
        Directory empty_dir("empty");
        REQUIRE(empty_dir.getSize() == 0);
    }

    SECTION("Directory with files of known sizes")
    {
        Directory parent("root");
        File *file1 = parent.createFile("file1.txt");
        File *file2 = parent.createFile("file2.txt");

        // Set file sizes
        file1->setSize(100);
        file2->setSize(200);

        // Directory size should be sum of children
        REQUIRE(parent.getSize() == 300);
    }

    SECTION("Directory with subdirectories")
    {
        Directory root("root");
        Directory *sub1 = root.createSubdirectory("sub1");
        Directory *sub2 = root.createSubdirectory("sub2");

        File *file1 = sub1->createFile("file1.txt");
        File *file2 = sub2->createFile("file2.txt");

        file1->setSize(50);
        file2->setSize(75);

        // Root should include sizes from subdirectories
        REQUIRE(root.getSize() == 125);
        REQUIRE(sub1->getSize() == 50);
        REQUIRE(sub2->getSize() == 75);
    }

    SECTION("Nested directory structure size calculation")
    {
        Directory root("root");
        Directory *level1 = root.createSubdirectory("level1");
        Directory *level2 = level1->createSubdirectory("level2");

        File *root_file = root.createFile("root.txt");
        File *l1_file = level1->createFile("l1.txt");
        File *l2_file = level2->createFile("l2.txt");

        root_file->setSize(10);
        l1_file->setSize(20);
        l2_file->setSize(30);

        // Each directory should report correct cumulative size
        REQUIRE(level2->getSize() == 30);
        REQUIRE(level1->getSize() == 50); // 20 + 30
        REQUIRE(root.getSize() == 60);    // 10 + 50
    }

    SECTION("Size calculation after adding/removing children")
    {
        Directory parent("root");
        File *file = parent.createFile("test.txt");
        file->setSize(100);

        REQUIRE(parent.getSize() == 100);

        // After adding more files
        File *file2 = parent.createFile("test2.txt");
        file2->setSize(50);

        REQUIRE(parent.getSize() == 150);

        // After removing a file
        bool removed = parent.remove("test.txt");
        REQUIRE(removed == true);
        REQUIRE(parent.getSize() == 50);

        // After removing the last file
        removed = parent.remove("test2.txt");
        REQUIRE(removed == true);
        REQUIRE(parent.getSize() == 0);
    }
}

TEST_CASE("Directory composite pattern behavior", "[Directory]")
{
    SECTION("Directory acts as FileObject")
    {
        std::unique_ptr<FileObject> obj = std::make_unique<Directory>("test");

        // Should be able to use FileObject interface
        REQUIRE(obj->getName() == "test");
        REQUIRE(obj->getPath() == "test");
        REQUIRE(obj->getSize() == 0);
    }

    SECTION("Directory supports composite operations unlike File")
    {
        std::unique_ptr<FileObject> dir = std::make_unique<Directory>("dir");
        std::unique_ptr<FileObject> child = std::make_unique<Directory>("child");

        // Directory should support add operation
        bool add_result = dir->add(std::move(child));
        REQUIRE(add_result == true);

        // Should be able to create children
        File *file = dir->createFile("test.txt");
        Directory *subdir = dir->createSubdirectory("subdir");

        REQUIRE(file != nullptr);
        REQUIRE(subdir != nullptr);
    }

    SECTION("Directory maintains parent-child relationships")
    {
        Directory root("root");
        Directory *child = root.createSubdirectory("child");
        File *file = child->createFile("grandchild.txt");

        // Verify ownership chain
        REQUIRE(child->getOwner() == &root);
        REQUIRE(file->getOwner() == child);

        // Verify path construction
        REQUIRE(file->getPath() == "root/child/grandchild.txt");
    }
}

TEST_CASE("Directory edge cases and error handling", "[Directory]")
{
    SECTION("Operations on directory with special characters in name")
    {
        Directory special("dir-with_special.chars");

        REQUIRE_NOTHROW(special.createFile("file.txt"));
        REQUIRE_NOTHROW(special.createSubdirectory("subdir"));
        REQUIRE(special.getName() == "dir-with_special.chars");
    }

    SECTION("Very long directory names")
    {
        std::string long_name(1000, 'a');
        Directory long_dir(long_name);

        REQUIRE(long_dir.getName() == long_name);
        REQUIRE_NOTHROW(long_dir.createFile("test.txt"));
    }

    SECTION("Directory operations are consistent")
    {
        Directory dir("consistent");

        // Multiple calls to getSize should return same value if no changes
        size_t size1 = dir.getSize();
        size_t size2 = dir.getSize();
        REQUIRE(size1 == size2);

        // Multiple calls to getName should return same value
        std::string name1 = dir.getName();
        std::string name2 = dir.getName();
        REQUIRE(name1 == name2);
    }

    SECTION("Directory handles null or invalid inputs gracefully")
    {
        Directory dir("test");

        // getChild with empty string
        FileObject *empty_name = dir.getChild("");
        // Should either return nullptr or handle gracefully

        // createFile/createSubdirectory with empty names
        REQUIRE_THROWS_AS(dir.createFile(""), std::runtime_error);
        REQUIRE_THROWS_AS(dir.createSubdirectory(""), std::runtime_error);
    }
}

TEST_CASE("Directory polymorphic behavior", "[Directory]")
{
    SECTION("Directory behaves correctly when used through FileObject pointer")
    {
        std::unique_ptr<FileObject> poly_dir = std::make_unique<Directory>("polymorphic");

        // Virtual function calls should work correctly
        REQUIRE(poly_dir->getName() == "polymorphic");
        REQUIRE(poly_dir->getPath() == "polymorphic");
        REQUIRE(poly_dir->getSize() == 0);

        // Composite-specific operations should work
        File *file = poly_dir->createFile("poly_file.txt");
        Directory *subdir = poly_dir->createSubdirectory("poly_subdir");

        REQUIRE(file != nullptr);
        REQUIRE(subdir != nullptr);
    }

    SECTION("Directory in heterogeneous collections")
    {
        Directory root("root");

        // Create mixed children
        File *file = root.createFile("file.txt");
        Directory *subdir = root.createSubdirectory("subdir");

        // Both should be accessible through parent
        FileObject *file_obj = root.getChild("file.txt");
        FileObject *dir_obj = root.getChild("subdir");

        REQUIRE(file_obj != nullptr);
        REQUIRE(dir_obj != nullptr);
        REQUIRE(file_obj->getName() == "file.txt");
        REQUIRE(dir_obj->getName() == "subdir");
    }
}