#include "utils/VerificationResultPrinter.hpp"
#include "directory-iteration-visitors/VerificationVisitor.hpp"
#include <catch2/catch_all.hpp>
#include <sstream>
#include <map>
#include <string>

TEST_CASE("VerificationResultPrinter - Empty results", "[VerificationResultPrinter]") {
    VerificationResultPrinter printer;
    std::ostringstream output;
    std::map<std::string, VerificationStatus> empty_results;
    
    printer.printResults(empty_results, output);
    
    REQUIRE(output.str().empty());
}

TEST_CASE("VerificationResultPrinter - Single result per status", "[VerificationResultPrinter]") {
    VerificationResultPrinter printer;
    std::ostringstream output;
    
    SECTION("OK status") {
        std::map<std::string, VerificationStatus> results;
        results["/path/to/file.txt"] = VerificationStatus::OK;
        
        printer.printResults(results, output);
        
        std::string result = output.str();
        REQUIRE(result.find("/path/to/file.txt: OK") != std::string::npos);
        REQUIRE(result.back() == '\n');
    }
    
    SECTION("MODIFIED status") {
        std::map<std::string, VerificationStatus> results;
        results["/path/to/modified.txt"] = VerificationStatus::MODIFIED;
        
        printer.printResults(results, output);
        
        std::string result = output.str();
        REQUIRE(result.find("/path/to/modified.txt: MODIFIED") != std::string::npos);
        REQUIRE(result.back() == '\n');
    }
    
    SECTION("NEW status") {
        std::map<std::string, VerificationStatus> results;
        results["/path/to/new.txt"] = VerificationStatus::NEW;
        
        printer.printResults(results, output);
        
        std::string result = output.str();
        REQUIRE(result.find("/path/to/new.txt: NEW") != std::string::npos);
        REQUIRE(result.back() == '\n');
    }
    
    SECTION("REMOVED status") {
        std::map<std::string, VerificationStatus> results;
        results["/path/to/removed.txt"] = VerificationStatus::REMOVED;
        
        printer.printResults(results, output);
        
        std::string result = output.str();
        REQUIRE(result.find("/path/to/removed.txt: REMOVED") != std::string::npos);
        REQUIRE(result.back() == '\n');
    }
}

TEST_CASE("VerificationResultPrinter - Multiple results", "[VerificationResultPrinter]") {
    VerificationResultPrinter printer;
    std::ostringstream output;
    std::map<std::string, VerificationStatus> results;
    
    results["/file1.txt"] = VerificationStatus::OK;
    results["/file2.txt"] = VerificationStatus::MODIFIED;
    results["/file3.txt"] = VerificationStatus::NEW;
    results["/file4.txt"] = VerificationStatus::REMOVED;
    
    printer.printResults(results, output);
    
    std::string result = output.str();
    
    REQUIRE(result.find("/file1.txt: OK") != std::string::npos);
    REQUIRE(result.find("/file2.txt: MODIFIED") != std::string::npos);
    REQUIRE(result.find("/file3.txt: NEW") != std::string::npos);
    REQUIRE(result.find("/file4.txt: REMOVED") != std::string::npos);
    
    std::istringstream line_stream(result);
    std::string line;
    int line_count = 0;
    while (std::getline(line_stream, line)) {
        line_count++;
        REQUIRE(!line.empty());
        REQUIRE(line.find(": ") != std::string::npos);
    }
    REQUIRE(line_count == 4);
}

TEST_CASE("VerificationResultPrinter - Output format", "[VerificationResultPrinter]") {
    VerificationResultPrinter printer;
    std::ostringstream output;
    std::map<std::string, VerificationStatus> results;
    results["test.txt"] = VerificationStatus::OK;
    
    printer.printResults(results, output);
    
    std::string result = output.str();
    REQUIRE(result == "test.txt: OK\n");
}

TEST_CASE("VerificationResultPrinter - Default cout parameter", "[VerificationResultPrinter]") {
    VerificationResultPrinter printer;
    std::map<std::string, VerificationStatus> results;
    results["test.txt"] = VerificationStatus::OK;
    
    REQUIRE_NOTHROW(printer.printResults(results));
}
