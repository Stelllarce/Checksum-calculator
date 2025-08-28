#include "ReportWriter.hpp"
#include "file-system-composite/File.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/Link.hpp"

ReportWriter::ReportWriter(std::ostream& os)
    : DirectoryIterationVisitor(os) {}

void ReportWriter::visitDirectory(Directory& dir) {
    ++_dir_count;
    std::filesystem::path p{dir.getPath().string()};
    _output << indentForPath(p) << "[DIR]  " << p.string() << '\n';
}

void ReportWriter::visitFile(File& file) {
    ++_file_count;
    std::filesystem::path p{file.getPath().string()};
    auto size = file.getSize();
    _total_bytes += size;
    _output << indentForPath(p) << "- " << p.string() << " (" << size << " bytes)" << '\n';
}

void ReportWriter::visitLink(Link& link) {
    ++_link_count;
    std::filesystem::path p{link.getPath().string()};
    _output << indentForPath(p) << "[LINK] " << p.string();
    if (auto* target = link.getResolvedTarget()) {
        _output << " -> " << target->getPath().string() << '\n';
        target->accept(*this);
    } else {
        _output << " (unresolved)" << '\n';
    }
}

void ReportWriter::writeSummary() {
    _output << "\nSummary: "
            << _dir_count << " dir(s), "
            << _file_count << " file(s), "
            << _link_count << " link(s), total "
            << _total_bytes << " bytes" << '\n';
}

void ReportWriter::reset() {
    _file_count = _dir_count = _link_count = 0;
    _total_bytes = 0;
}

std::string ReportWriter::indentForPath(const std::filesystem::path& p) {
    std::size_t depth = 0;
    for (const auto& _ : p) { (void)_; ++depth; }
    if (depth > 0) --depth;
    return std::string(depth * 2, ' ');
}