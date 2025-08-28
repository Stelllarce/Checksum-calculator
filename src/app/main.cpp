#include <tclap/CmdLine.h>
#include <iostream>
#include <filesystem>
#include <memory>

// Include project headers
#include "calculators/CalculatorFactory.hpp"
#include "calculators/ChecksumCalculator.hpp"
#include "directory-tree-builders/DirectoryConstructor.hpp"
#include "directory-tree-builders/LinkFollowBuilder.hpp"
#include "directory-tree-builders/NonFollowLinkBuilder.hpp"
#include "directory-tree-builders/CycleDetector.hpp"
#include "directory-iteration-visitors/HashStreamWriter.hpp"
#include "directory-iteration-visitors/VerificationVisitor.hpp"
#include "directory-iteration-visitors/ReportWriter.hpp"
#include "utils/ChecksumFileReader.hpp"
#include "utils/VerificationResultPrinter.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/File.hpp"
#include "progress-indicator-observers/ProgressReporter.hpp"

// Helper function to calculate total size of directory tree
std::uint64_t calculateTotalSize(Directory* root) {
    if (!root) return 0;
    return static_cast<std::uint64_t>(root->getSize());
}

int main(int argc, char** argv) {
    try {
        // Create command line object
        TCLAP::CmdLine cmd("Checksum calculator - Calculate and verify file checksums", ' ', "0.1");
        
        // Add command line arguments
        TCLAP::ValueArg<std::string> path_arg("p", "path", 
            "Target file or directory to analyze (default: current directory)", 
            false, ".", "path");
        cmd.add(path_arg);
        
        TCLAP::ValueArg<std::string> algorithm_arg("a", "algorithm", 
            "Checksum algorithm to use (md5, sha1, sha256)", 
            false, "md5", "algorithm");
        cmd.add(algorithm_arg);
        
        TCLAP::ValueArg<std::string> checksums_arg("c", "checksums", 
            "File containing checksums for verification mode", 
            false, "", "checksum_file");
        cmd.add(checksums_arg);
        
        TCLAP::ValueArg<std::string> format_arg("f", "format", 
            "Output format (text, xml, json, markdown, html)", 
            false, "text", "format");
        cmd.add(format_arg);
        
        TCLAP::SwitchArg follow_links_arg("l", "follow-links", 
            "Follow symbolic links instead of calculating checksum of link itself", 
            cmd, false);
        
        TCLAP::SwitchArg report_arg("r", "report", 
            "Display a report of files to be traversed and their sizes", 
            cmd, false);
        
        // Parse command line
        cmd.parse(argc, argv);
        
        // Get values
        std::string target_path = path_arg.getValue();
        std::string algorithm = algorithm_arg.getValue();
        std::string checksums_file = checksums_arg.getValue();
        std::string output_format = format_arg.getValue();
        bool follow_symbolic_links = follow_links_arg.getValue();
        bool show_report = report_arg.getValue();
        
        // Validate arguments
        if (!std::filesystem::exists(target_path)) {
            std::cerr << "Error: Target path '" << target_path << "' does not exist." << std::endl;
            return 1;
        }
        
        // Create checksum calculator
        auto calculator = CalculatorFactory::create(algorithm);
        if (!calculator) {
            std::cerr << "Error: Unsupported algorithm '" << algorithm << "'. "
                      << "Supported algorithms: md5, sha1, sha256" << std::endl;
            return 1;
        }
        
        // Choose appropriate directory structure builder based on link handling preference
        std::unique_ptr<DirectoryStructureBuilder> builder;
        if (follow_symbolic_links) {
            builder = std::make_unique<LinkFollowBuilder>(std::make_unique<CycleDetector>());
        } else {
            builder = std::make_unique<NonFollowLinkBuilder>();
        }
        
        // Build directory structure
        DirectoryConstructor constructor(*builder);
        constructor.construct({target_path});
        Directory* root = builder->getTree();
        
        if (!root) {
            std::cerr << "Error: Failed to build directory structure for '" << target_path << "'." << std::endl;
            return 1;
        }
        
        // Determine mode of operation
        if (show_report) {
            // Report mode - show files to be traversed and their sizes
            try {
                ReportWriter report_writer(std::cout);
                root->accept(report_writer);
                report_writer.writeSummary();
                
                // Continue with other operations if verification or explicit algorithm was requested
                if (checksums_file.empty() && algorithm_arg.isSet() == false) {
                    // User only wanted the report, no checksums
                    return 0;
                }
                
                // Otherwise, add a separator before continuing with other operations
                std::cout << "\n" << std::string(50, '-') << "\n" << std::endl;
                
            } catch (const std::exception& e) {
                std::cerr << "Error during report generation: " << e.what() << std::endl;
                return 1;
            }
        }
        
        if (!checksums_file.empty()) {
            // Verification mode
            if (!std::filesystem::exists(checksums_file)) {
                std::cerr << "Error: Checksums file '" << checksums_file << "' does not exist." << std::endl;
                return 1;
            }
            
            try {
                ChecksumFileReader reader;
                auto expected_checksums = reader.readChecksums(checksums_file);
                
                VerificationVisitor verification_visitor(expected_checksums);
                root->accept(verification_visitor);
                
                auto results = verification_visitor.getResults();
                VerificationResultPrinter printer;
                printer.printResults(results, std::cout);
                
            } catch (const std::exception& e) {
                std::cerr << "Error during verification: " << e.what() << std::endl;
                return 1;
            }
            
        } else {
            // Checksum calculation mode
            try {
                // Note: For now using text format only, other formats can be implemented later
                if (output_format != "text") {
                    std::cerr << "Warning: Only text format is currently implemented. Using text format." << std::endl;
                }
                
                // Calculate total size for progress reporting
                std::uint64_t total_size = calculateTotalSize(root);
                
                HashStreamWriter hash_writer(std::move(calculator), std::cout);
                
                // Create and register progress reporter if total size is significant
                std::unique_ptr<ProgressReporter> progress_reporter;
                if (total_size > 1024 * 1024) { // Only show progress for files > 1MB
                    progress_reporter = std::make_unique<ProgressReporter>(total_size, std::cerr);
                    progress_reporter->start();
                    hash_writer.attach(progress_reporter.get());
                }
                
                root->accept(hash_writer);
                
                // Ensure final newline after progress display
                if (progress_reporter) {
                    std::cerr << std::endl;
                }
                
            } catch (const std::exception& e) {
                std::cerr << "Error during checksum calculation: " << e.what() << std::endl;
                return 1;
            }
        }
        
    } catch (TCLAP::ArgException& e) {
        std::cerr << "Error: " << e.error() << " for arg " << e.argId() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}