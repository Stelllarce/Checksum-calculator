#pragma once
#include "DirectoryIterationVisitor.hpp"
#include <cstddef>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

class File;
class Directory;
class Link;

/**
 * @class ReportWriter
 * @brief Visitor that prints an inventory of the directory tree and keeps simple statistics.
 */
class ReportWriter : public DirectoryIterationVisitor {
public:
    explicit ReportWriter(std::ostream& os);

    void visitFile(File& file) override;
    void visitDirectory(Directory& dir) override;
    void visitLink(Link& link) override;

    void writeSummary();
    void reset();

private:
    static std::string indentForPath(const std::filesystem::path& p);

    std::size_t _file_count = 0;
    std::size_t _dir_count = 0;
    std::size_t _link_count = 0;
    std::uintmax_t _total_bytes = 0;
};