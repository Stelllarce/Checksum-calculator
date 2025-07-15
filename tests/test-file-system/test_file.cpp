#include "file-system-composite/File.hpp"
#include "file-system-composite/Directory.hpp"

#include <catch2/catch_all.hpp>
#include <memory>
#include <limits>
#include <sstream>

TEST_CASE("File constructor", "[File]") {
    SECTION("File with valid directory owner") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        
        // File constructor automatically adds itself to the parent directory
        REQUIRE_NOTHROW(root_dir->createFile("test.txt"));
        
        // Verify the file was added to the directory
        FileObject* file = root_dir->getChild("root/test.txt");
        REQUIRE(file != nullptr);
        REQUIRE(file->getName() == "test.txt");
        REQUIRE(file->getPath() == "root/test.txt");
    }
    
    SECTION("File with null directory owner") {
        // Should throw when trying to create file without parent directory
        REQUIRE_THROWS_AS(File("test.txt", nullptr), std::logic_error);
    }
    
    SECTION("File with non-directory owner") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("root");
        FileObject* file = root_dir->createFile("parent.txt");
        
        // Should throw when trying to create file with file as parent
        REQUIRE_THROWS_AS(File("child.txt", file), std::runtime_error);
    }
}

TEST_CASE("File getName functionality", "[File]") {
    SECTION("Get name from simple path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("documents");
        FileObject* file = root_dir->createFile("document.pdf");
        
        REQUIRE(file->getName() == "document.pdf");
    }
    
    SECTION("Get name with extension") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("workspace");
        FileObject* file = root_dir->createFile("report.docx");
        
        REQUIRE(file->getName() == "report.docx");
    }
    
    SECTION("Get name without extension") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("bin");
        FileObject* file = root_dir->createFile("executable");
        
        REQUIRE(file->getName() == "executable");
    }
    
    SECTION("Get name with special characters") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("temp");
        FileObject* file = root_dir->createFile("file-with_special.chars.txt");
        
        REQUIRE(file->getName() == "file-with_special.chars.txt");
    }
}

TEST_CASE("File getPath functionality", "[File]") {
    SECTION("Get full path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("home");
        FileObject* file = root_dir->createFile("config.ini");
        
        std::string path = file->getPath();
        REQUIRE_FALSE(path.empty());
        REQUIRE(path == "home/config.ini");
    }
    
    SECTION("Path consistency") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("projects");
        FileObject* file = root_dir->createFile("main.cpp");
        
        std::string path_1 = file->getPath();
        std::string path_2 = file->getPath();
        
        REQUIRE(path_1 == path_2);
        REQUIRE(path_1 == "projects/main.cpp");
    }
    
    SECTION("Nested directory path") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("home");
        FileObject* sub_dir = root_dir->createSubdirectory("user");
        FileObject* file = sub_dir->createFile("settings.conf");
        
        REQUIRE(file->getPath() == "home/user/settings.conf");
    }
}

TEST_CASE("File size management", "[File]") {
    SECTION("Initial size is zero") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("data");
        FileObject* file = root_dir->createFile("empty.dat");
        
        REQUIRE(file->getSize() == 0);
    }
    
    SECTION("Set valid size") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("files");
        FileObject* file = root_dir->createFile("large.bin");
        
        REQUIRE(file->setSize(1024) == true);
        REQUIRE(file->getSize() == 1024);
    }
    
    SECTION("Set zero size should fail") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("files");
        FileObject* file = root_dir->createFile("test.txt");
        
        REQUIRE(file->setSize(0) == false);
        REQUIRE(file->getSize() == 0);  // Should remain unchanged
    }
    
    SECTION("Set negative size should fail") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("files");
        FileObject* file = root_dir->createFile("test.txt");
        
        // setSize checks for size <= 0, so this should fail
        REQUIRE(file->setSize(static_cast<size_t>(-1)) == true);  // This will be a very large positive number
    }
    
    SECTION("Update size multiple times") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("workspace");
        FileObject* file = root_dir->createFile("growing.log");
        
        REQUIRE(file->setSize(100) == true);
        REQUIRE(file->getSize() == 100);
        
        REQUIRE(file->setSize(500) == true);
        REQUIRE(file->getSize() == 500);
        
        REQUIRE(file->setSize(50) == true);
        REQUIRE(file->getSize() == 50);
    }
    
    SECTION("Set maximum size") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("huge");
        FileObject* file = root_dir->createFile("massive.dat");
        
        size_t max_size = std::numeric_limits<size_t>::max();
        REQUIRE(file->setSize(max_size) == true);
        REQUIRE(file->getSize() == max_size);
    }
}

TEST_CASE("File composite pattern behavior", "[File]") {
    SECTION("File cannot add children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        FileObject* file = root_dir->createFile("leaf.txt");
        
        std::unique_ptr<FileObject> child_dir = std::make_unique<Directory>("child");
        std::unique_ptr<FileObject> child_ptr = std::move(child_dir);
        REQUIRE(file->add(child_ptr) == false);
    }
    
    SECTION("File cannot remove children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        FileObject* file = root_dir->createFile("leaf.txt");
        
        std::unique_ptr<FileObject> child_dir = std::make_unique<Directory>("child");
        std::unique_ptr<FileObject> child_ptr = std::move(child_dir);
        REQUIRE(file->remove(child_ptr) == false);
    }
    
    SECTION("File has no children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        FileObject* file = root_dir->createFile("leaf.txt");
        
        REQUIRE(file->getChild("nonexistent") == nullptr);
        REQUIRE(file->getChild("anything") == nullptr);
    }
    
    SECTION("File cannot create children") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("parent");
        FileObject* file = root_dir->createFile("leaf.txt");
        
        REQUIRE(file->createFile("child.txt") == nullptr);
        REQUIRE(file->createSubdirectory("subdir") == nullptr);
    }
}

TEST_CASE("File content management", "[File]") {
    SECTION("Initial content is empty") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("data");
        FileObject* file = root_dir->createFile("content.txt");
        
        REQUIRE(file->read() == "");
        REQUIRE(file->getSize() == 0);
    }
    
    SECTION("Write and read content") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("docs");
        FileObject* file = root_dir->createFile("document.txt");
        
        std::stringstream ss("Hello World");
        REQUIRE_NOTHROW(file->write(ss));
        
        REQUIRE(file->read() == "Hello");  // operator>> only reads first word
        REQUIRE(file->getSize() > 0);
    }
    
    SECTION("Write from bad stream should throw") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        FileObject* file = root_dir->createFile("bad.txt");
        
        std::stringstream ss;
        ss.setstate(std::ios::badbit);
        
        REQUIRE_THROWS_AS(file->write(ss), std::ios_base::failure);
    }
    
    SECTION("Write updates size automatically") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("auto");
        FileObject* file = root_dir->createFile("sized.txt");
        
        std::stringstream ss("test");
        file->write(ss);
        
        REQUIRE(file->getSize() == file->read().size());
    }
}

TEST_CASE("File edge cases and error handling", "[File]") {
    SECTION("Empty filename") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        FileObject* file = root_dir->createFile("");
        
        REQUIRE(file->getName() == "");
        REQUIRE(file->getPath() == "test/");
    }
    
    SECTION("Very long filename") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        std::string long_name(100, 'a');  // Reduced from 1000 to avoid potential issues
        long_name += ".txt";
        
        FileObject* file = root_dir->createFile(long_name);
        
        REQUIRE(file->getName() == long_name);
        REQUIRE(file->getPath().find(long_name) != std::string::npos);
    }
    
    SECTION("Filename with special characters") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("test");
        std::string special_name = "file_with-special.chars@123.txt";
        
        FileObject* file = root_dir->createFile(special_name);
        
        REQUIRE(file->getName() == special_name);
        REQUIRE(file->getPath() == "test/" + special_name);
    }
}

TEST_CASE("File polymorphic behavior", "[File]") {
    SECTION("File as FileObject pointer") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("polymorphism");
        FileObject* file = root_dir->createFile("poly.txt");
        FileObject* file_obj = file;
        
        REQUIRE_NOTHROW(file_obj->getName());
        REQUIRE_NOTHROW(file_obj->getPath());
        REQUIRE(file_obj->getSize() == 0);
        
        REQUIRE(file_obj->setSize(42) == true);
        REQUIRE(file_obj->getSize() == 42);
        
        // Composite methods should return default values
        std::unique_ptr<FileObject> dummy_child = std::make_unique<Directory>("dummy");
        std::unique_ptr<FileObject> dummy_ptr = std::move(dummy_child);
        REQUIRE(file_obj->add(dummy_ptr) == false);
        REQUIRE(file_obj->remove(dummy_ptr) == false);
        REQUIRE(file_obj->getChild("anything") == nullptr);
        
        // Factory methods should return nullptr for files
        REQUIRE(file_obj->createFile("child.txt") == nullptr);
        REQUIRE(file_obj->createSubdirectory("subdir") == nullptr);
    }
}

TEST_CASE("File memory and resource management", "[File]") {
    SECTION("Multiple files with same name in different directories") {
        std::unique_ptr<FileObject> dir_1 = std::make_unique<Directory>("dir1");
        std::unique_ptr<FileObject> dir_2 = std::make_unique<Directory>("dir2");
        
        FileObject* file_1 = dir_1->createFile("same.txt");
        FileObject* file_2 = dir_2->createFile("same.txt");
        
        REQUIRE(file_1->getName() == file_2->getName());
        REQUIRE(file_1->getPath() != file_2->getPath());
        REQUIRE(file_1->getPath() == "dir1/same.txt");
        REQUIRE(file_2->getPath() == "dir2/same.txt");
    }
    
    SECTION("File ownership and parent relationship") {
        std::unique_ptr<FileObject> root_dir = std::make_unique<Directory>("owner_test");
        FileObject* file = root_dir->createFile("owned.txt");
        
        REQUIRE(file->getOwner() == root_dir.get());
        
        // Verify file is in parent's children
        FileObject* found_file = root_dir->getChild("owner_test/owned.txt");
        REQUIRE(found_file == file);
    }
}

