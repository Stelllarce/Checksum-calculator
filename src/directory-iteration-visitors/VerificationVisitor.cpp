#include "VerificationVisitor.hpp"
#include "file-system-composite/File.hpp"
#include <iostream>
#include <sstream>
#include "calculators/Md5Calculator.hpp"
#include "calculators/SHA1Calculator.hpp"
#include "calculators/SHA256Calculator.hpp"

VerificationVisitor::VerificationVisitor(std::map<std::string, std::string> expected_checksums) 
        : DirectoryIterationVisitor(std::cout),
        _expected_checksums(std::move(expected_checksums)) {}

void VerificationVisitor::visitFile(File &file)
{
    std::string file_path = file.getPath();
    auto it = _expected_checksums.find(file_path);

    if (it == _expected_checksums.end())
    {
        _results[file_path] = VerificationStatus::NEW;
        return;
    }

    std::istringstream iss(it->second);
    std::string algorithm;
    std::string expected_checksum;
    iss >> algorithm >> expected_checksum;

    if (algorithm == "md5") {
        _calculator = std::make_unique<Md5Calculator>();
    } else if (algorithm == "sha1") {
        _calculator = std::make_unique<SHA1Calculator>();
    } else if (algorithm == "sha256") {
        _calculator = std::make_unique<SHA256Calculator>();
    } else {
        // Should not happen with controlled input
        _results[file_path] = VerificationStatus::MODIFIED;
        _expected_checksums.erase(it);
        return;
    }

    std::string actual_checksum = _calculator->calculate(file.getPath());

    if (expected_checksum == actual_checksum)
    {
        _results[file_path] = VerificationStatus::OK;
    }
    else
    {
        _results[file_path] = VerificationStatus::MODIFIED;
    }
    _expected_checksums.erase(it);
}

std::map<std::string, VerificationStatus> VerificationVisitor::getResults()
{
    for (const auto &pair : _expected_checksums)
    {
        _results[pair.first] = VerificationStatus::REMOVED;
    }
    return _results;
}
