#pragma once

#include "directory-iteration-visitors/DirectoryIterationVisitor.hpp"
#include "calculators/ChecksumCalculator.hpp"
#include <map>
#include <string>
#include <memory>

enum class VerificationStatus
{
    OK,
    MODIFIED,
    NEW,
    REMOVED
};

class VerificationVisitor : public DirectoryIterationVisitor
{
public:
    VerificationVisitor(
        std::map<std::string, std::string> expected_checksums);

    void visitFile(File &file) override;
    void visitDirectory(Directory &dir) override { }

    std::map<std::string, VerificationStatus> getResults();

private:
    std::map<std::string, std::string> _expected_checksums;
    std::unique_ptr<ChecksumCalculator> _calculator;
    std::map<std::string, VerificationStatus> _results;
};
