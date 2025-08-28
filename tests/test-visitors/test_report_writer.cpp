#include "directory-iteration-visitors/ReportWriter.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include "file-system-composite/Link.hpp"
#include <catch2/catch_all.hpp>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <memory>

namespace {
    class ReportWriterTestMockup {
    public:
        const std::filesystem::path base_path;
        const std::filesystem::path test_file1;
        const std::filesystem::path test_file2;
        const std::filesystem::path empty_file;
        const std::filesystem::path subdir_path;
        const std::filesystem::path subdir_file;
        
        ReportWriterTestMockup() 
            : base_path(std::filesystem::temp_directory_path() / "report_writer_test")
            , test_file1(base_path / "test1.txt")
            , test_file2(base_path / "test2.txt")
            , empty_file(base_path / "empty.txt")
            , subdir_path(base_path / "subdir")
            , subdir_file(subdir_path / "nested.txt")
        {
            setup();
        }
        
        ~ReportWriterTestMockup() {
            cleanup();
        }
        
        ReportWriterTestMockup(const ReportWriterTestMockup&) = delete;
        ReportWriterTestMockup& operator=(const ReportWriterTestMockup&) = delete;
        
    private:
        void setup() {
            std::filesystem::remove_all(base_path);
            std::filesystem::create_directories(base_path);
            std::filesystem::create_directories(subdir_path);
            
            std::ofstream(test_file1) << "Hello World";
            std::ofstream(test_file2) << "Test content for file 2";
            std::ofstream(empty_file);
            std::ofstream(subdir_file) << "Nested file content";
        }
        
        void cleanup() {
            std::filesystem::remove_all(base_path);
        }
    };
    
    static ReportWriterTestMockup test_mockup;
}

TEST_CASE("ReportWriter - Constructor", "[ReportWriter]") {
    std::ostringstream output_stream;
    
    SECTION("Valid constructor") {
        REQUIRE_NOTHROW(ReportWriter(output_stream));
    }
}

TEST_CASE("ReportWriter - visitFile", "[ReportWriter]") {
    std::ostringstream output_stream;
    ReportWriter writer(output_stream);
    
    SECTION("Visit single file with content") {
        Directory root_dir(test_mockup.base_path);
        File test_file(test_mockup.test_file1, &root_dir);
        
        writer.visitFile(test_file);
        
        std::string output = output_stream.str();
        REQUIRE_FALSE(output.empty());
        REQUIRE(output.find("- ") != std::string::npos);
        REQUIRE(output.find(test_mockup.test_file1.string()) != std::string::npos);
        REQUIRE(output.find("(11 bytes)") != std::string::npos);
        REQUIRE(output.back() == '\n');
    }
    
    SECTION("Visit empty file") {
        Directory root_dir(test_mockup.base_path);
        File empty_test_file(test_mockup.empty_file, &root_dir);
        
        writer.visitFile(empty_test_file);
        
        std::string output = output_stream.str();
        REQUIRE_FALSE(output.empty());
        REQUIRE(output.find("- ") != std::string::npos);
        REQUIRE(output.find(test_mockup.empty_file.string()) != std::string::npos);
        REQUIRE(output.find("(0 bytes)") != std::string::npos);
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
                REQUIRE(line.find("- ") != std::string::npos);
                REQUIRE(line.find(test_mockup.test_file1.string()) != std::string::npos);
                REQUIRE(line.find("(11 bytes)") != std::string::npos);
            } else if (line_count == 2) {
                REQUIRE(line.find("- ") != std::string::npos);
                REQUIRE(line.find(test_mockup.test_file2.string()) != std::string::npos);
                REQUIRE(line.find("(23 bytes)") != std::string::npos);
            }
        }
        
        REQUIRE(line_count == 2);
    }
}

TEST_CASE("ReportWriter - visitDirectory", "[ReportWriter]") {
    std::ostringstream output_stream;
    ReportWriter writer(output_stream);
    
    SECTION("Visit directory should produce output with [DIR] prefix") {
        Directory test_dir(test_mockup.base_path);
        
        writer.visitDirectory(test_dir);
        
        std::string output = output_stream.str();
        REQUIRE_FALSE(output.empty());
        REQUIRE(output.find("[DIR]") != std::string::npos);
        REQUIRE(output.find(test_mockup.base_path.string()) != std::string::npos);
        REQUIRE(output.back() == '\n');
    }
    
    SECTION("Visit nested directory should have correct indentation") {
        Directory root_dir(test_mockup.base_path);
        Directory sub_dir(test_mockup.subdir_path);
        
        writer.visitDirectory(sub_dir);
        
        std::string output = output_stream.str();
        REQUIRE(output.find("[DIR]") != std::string::npos);
        REQUIRE(output.find(test_mockup.subdir_path.string()) != std::string::npos);
        
        size_t dir_pos = output.find("[DIR]");
        std::string indent = output.substr(0, dir_pos);
        REQUIRE(indent.find("  ") != std::string::npos);
    }
}

TEST_CASE("ReportWriter - visitLink", "[ReportWriter]") {
    std::ostringstream output_stream;
    ReportWriter writer(output_stream);
    
    SECTION("Visit link to existing file should process target") {
        Directory root_dir(test_mockup.base_path);
        Link test_link("link_to_file", "target.txt", &root_dir);
        
        auto target_file = std::make_unique<File>(test_mockup.test_file1, &root_dir);
        test_link.setResolveTarget(std::move(target_file));
        
        writer.visitLink(test_link);
        
        std::string output = output_stream.str();
        REQUIRE_FALSE(output.empty());
        REQUIRE(output.find("[LINK]") != std::string::npos);
        REQUIRE(output.find(" -> ") != std::string::npos);
        REQUIRE(output.find(test_mockup.test_file1.string()) != std::string::npos);
        
        std::istringstream line_stream(output);
        std::string line;
        int line_count = 0;
        while (std::getline(line_stream, line)) {
            line_count++;
            if (line_count == 1) {
                REQUIRE(line.find("[LINK]") != std::string::npos);
                REQUIRE(line.find(" -> ") != std::string::npos);
            } else if (line_count == 2) {
                REQUIRE(line.find("- ") != std::string::npos);
                REQUIRE(line.find("(11 bytes)") != std::string::npos);
            }
        }
        REQUIRE(line_count == 2);
    }
    
    SECTION("Visit link with no resolved target should show unresolved") {
        Directory root_dir(test_mockup.base_path);
        Link broken_test_link("broken_link", "non_existent_target", &root_dir);
        
        writer.visitLink(broken_test_link);
        
        std::string output = output_stream.str();
        REQUIRE_FALSE(output.empty());
        REQUIRE(output.find("[LINK]") != std::string::npos);
        REQUIRE(output.find("(unresolved)") != std::string::npos);
    }
}

TEST_CASE("ReportWriter - Output format", "[ReportWriter]") {
    std::ostringstream output_stream;
    ReportWriter writer(output_stream);
    
    SECTION("File output format should be correct with indentation") {
        Directory root_dir("root", nullptr);
        File test_file("test.txt", &root_dir);
        test_file.setSize(11);
        
        writer.visitFile(test_file);
        
        std::string output = output_stream.str();
        std::istringstream line_stream(output);
        std::string line;
        
        REQUIRE(std::getline(line_stream, line));
        REQUIRE(line.find("- ") != std::string::npos);
        REQUIRE(line.find(" (11 bytes)") != std::string::npos);
        REQUIRE(line.find("root/test.txt") != std::string::npos);
        REQUIRE_FALSE(std::getline(line_stream, line));
    }
    
    SECTION("Directory output format should be correct") {
        Directory test_dir("mydir", nullptr);
        
        writer.visitDirectory(test_dir);
        
        std::string output = output_stream.str();
        std::istringstream line_stream(output);
        std::string line;
        
        REQUIRE(std::getline(line_stream, line));
        REQUIRE(line.find("[DIR]  ") != std::string::npos);
        REQUIRE(line.find("mydir") != std::string::npos);
        REQUIRE_FALSE(std::getline(line_stream, line));
    }
}

TEST_CASE("ReportWriter - Statistics and Summary", "[ReportWriter]") {
    std::ostringstream output_stream;
    ReportWriter writer(output_stream);
    
    SECTION("Count files, directories, and links correctly") {
        Directory root_dir("root", nullptr);
        Directory sub_dir("subdir", &root_dir);
        File file1("file1.txt", &root_dir);
        File file2("file2.txt", &root_dir);
        File nested_file("nested.txt", &sub_dir);
        
        // Set explicit sizes
        file1.setSize(10);
        file2.setSize(20);
        nested_file.setSize(15);
        
        Link test_link("link1", "target", &root_dir);
        // Create a simple file for the link target
        auto target_file = std::make_unique<File>("target.txt", &root_dir);
        target_file->setSize(5);
        test_link.setResolveTarget(std::move(target_file));
        
        writer.visitDirectory(root_dir);
        writer.visitDirectory(sub_dir);
        writer.visitFile(file1);
        writer.visitFile(file2);
        writer.visitFile(nested_file);
        writer.visitLink(test_link);
        
        output_stream.str("");
        output_stream.clear();
        
        writer.writeSummary();
        
        std::string summary = output_stream.str();
        REQUIRE(summary.find("Summary: ") != std::string::npos);
        REQUIRE(summary.find("2 dir(s)") != std::string::npos);
        REQUIRE(summary.find("4 file(s)") != std::string::npos); // 3 original + 1 from link target
        REQUIRE(summary.find("1 link(s)") != std::string::npos);
        REQUIRE(summary.find("total 50 bytes") != std::string::npos); // 10+20+15+5 = 50
    }
    
    SECTION("Reset functionality") {
        Directory root_dir("root", nullptr);
        File test_file("test.txt", &root_dir);
        test_file.setSize(10);
        
        writer.visitDirectory(root_dir);
        writer.visitFile(test_file);
        
        writer.reset();
        
        output_stream.str("");
        output_stream.clear();
        
        writer.writeSummary();
        
        std::string summary = output_stream.str();
        REQUIRE(summary.find("0 dir(s)") != std::string::npos);
        REQUIRE(summary.find("0 file(s)") != std::string::npos);
        REQUIRE(summary.find("0 link(s)") != std::string::npos);
        REQUIRE(summary.find("total 0 bytes") != std::string::npos);
    }
}

TEST_CASE("ReportWriter - Indentation", "[ReportWriter]") {
    std::ostringstream output_stream;
    ReportWriter writer(output_stream);
    
    SECTION("Root level should have no indentation") {
        Directory root_dir("root");
        
        writer.visitDirectory(root_dir);
        
        std::string output = output_stream.str();
        std::istringstream line_stream(output);
        std::string line;
        
        REQUIRE(std::getline(line_stream, line));
        REQUIRE(line.find("[DIR]") == 0);
    }
    
    SECTION("Nested paths should have proper indentation") {
        std::filesystem::path nested_path = test_mockup.base_path / "level1" / "level2" / "file.txt";
        Directory level1(test_mockup.base_path / "level1");
        File nested_file(nested_path, &level1);
        
        writer.visitFile(nested_file);
        
        std::string output = output_stream.str();
        std::istringstream line_stream(output);
        std::string line;
        
        REQUIRE(std::getline(line_stream, line));
        size_t dash_pos = line.find("- ");
        std::string indent = line.substr(0, dash_pos);
        REQUIRE(indent.length() >= 4);
        REQUIRE(indent.find("    ") == 0);
    }
}

TEST_CASE("ReportWriter - Integration with mock file system", "[ReportWriter]") {
    std::ostringstream output_stream;
    ReportWriter writer(output_stream);
    
    SECTION("Process complete mock file system hierarchy") {
        Directory root_dir("root", nullptr);
        Directory sub_dir("subdir", &root_dir);
        
        File file1("file1.txt", &root_dir);
        File file2("file2.txt", &root_dir);
        File empty_file("empty.txt", &root_dir);
        File nested_file("nested.txt", &sub_dir);
        
        // Set explicit sizes to avoid filesystem dependency
        file1.setSize(10);
        file2.setSize(20);
        empty_file.setSize(0);
        nested_file.setSize(15);
        
        Link working_link("link_to_file1", "file1.txt", &root_dir);
        auto target_file1 = std::make_unique<File>("target1.txt", &root_dir);
        target_file1->setSize(10);
        working_link.setResolveTarget(std::move(target_file1));
        
        Link broken_link("broken_link", "nonexistent", &root_dir);
        
        writer.visitDirectory(root_dir);
        writer.visitDirectory(sub_dir);
        writer.visitFile(file1);
        writer.visitFile(file2);
        writer.visitFile(empty_file);
        writer.visitFile(nested_file);
        writer.visitLink(working_link);
        writer.visitLink(broken_link);
        
        std::string full_output = output_stream.str();
        
        REQUIRE(full_output.find("[DIR]") != std::string::npos);
        REQUIRE(full_output.find("- ") != std::string::npos);
        REQUIRE(full_output.find("[LINK]") != std::string::npos);
        REQUIRE(full_output.find("(unresolved)") != std::string::npos);
        REQUIRE(full_output.find(" -> ") != std::string::npos);
        
        output_stream.str("");
        output_stream.clear();
        
        writer.writeSummary();
        
        std::string summary = output_stream.str();
        REQUIRE(summary.find("2 dir(s)") != std::string::npos);
        REQUIRE(summary.find("5 file(s)") != std::string::npos); // 4 original + 1 from working link
        REQUIRE(summary.find("2 link(s)") != std::string::npos);
        REQUIRE(summary.find("total 55 bytes") != std::string::npos); // 10+20+0+15+10 = 55
    }
}