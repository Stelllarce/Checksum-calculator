#include "HashStreamWriter.hpp"
#include "file-system-composite/File.hpp"
#include "file-system-composite/Directory.hpp"
#include "file-system-composite/Link.hpp"
#include "calculators/ChecksumCalculator.hpp"
#include <stdexcept>
#include <fstream>

HashStreamWriter::HashStreamWriter(std::unique_ptr<ChecksumCalculator> calc, std::ostream& os)
    : DirectoryIterationVisitor(os), _hash_strategy(std::move(calc)) {
    if (!_hash_strategy) {
    throw std::runtime_error("Checksum calculator cannot be null");
    }
}

void HashStreamWriter::visitFile(File& file) {
    processFile(file);
}

void HashStreamWriter::visitDirectory(Directory&) {}

void HashStreamWriter::visitLink(Link& link) {
    if (auto* target = link.getResolvedTarget()) {
    target->accept(*this);
    }
}

void HashStreamWriter::applyAlgorithm(File& file) {
#ifdef DEBUG
    std::ifstream debug_stream(file.getPath(), std::ios::in | std::ios::binary);
    if (debug_stream.is_open()) {
        std::vector<char> content = file.read(debug_stream);
        debug_stream.close();
        std::string content_str(content.begin(), content.end());
        std::string checksum = _hash_strategy->calculate(content_str);
        _output << checksum << " " << file.getPath() << '\n';
    } else {
        // Fallback to regular read if debug stream fails
        std::vector<char> content = file.read();
        std::string content_str(content.begin(), content.end());
        std::string checksum = _hash_strategy->calculate(content_str);
        _output << checksum << " " << file.getPath() << '\n';
    }
#else
    std::vector<char> content = file.read();
    std::string content_str(content.begin(), content.end());
    std::string checksum = _hash_strategy->calculate(content_str);
    _output << checksum << " " << file.getPath() << '\n';
#endif
}