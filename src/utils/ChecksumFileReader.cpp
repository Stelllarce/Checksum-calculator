#include "ChecksumFileReader.hpp"
#include <fstream>
#include <sstream>

std::map<std::string, std::string> ChecksumFileReader::readChecksums(const std::string &file_path)
{
    std::map<std::string, std::string> checksums;
    std::ifstream file(file_path);
    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string algorithm;
        std::string checksum;
        std::string file_path_str;

        if (iss >> algorithm >> checksum >> file_path_str)
        {
            checksums[file_path_str] = algorithm + " " + checksum;
        }
    }

    return checksums;
}
