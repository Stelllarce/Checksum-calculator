#pragma once

#include <string>
#include <vector>
#include <map>

class ChecksumFileReader
{
public:
    std::map<std::string, std::string> readChecksums(const std::string &file_path);
};
