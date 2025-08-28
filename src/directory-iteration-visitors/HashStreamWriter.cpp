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
    std::vector<char> content;
#ifdef DEBUG
    std::ifstream debug_stream(file.getPath().string(), std::ios::in | std::ios::binary);
    if (debug_stream.is_open()) {
        content = file.read(debug_stream);
        debug_stream.close();
    } else {
        // Fallback to regular read if debug stream fails
        content = file.read();
    }
#else
    content = file.read();
#endif
    std::string content_str(content.begin(), content.end());
    std::string checksum = _hash_strategy->calculate(content_str);
    _output << _hash_strategy->getAlgorithmName() << " " << checksum << " " << file.getPath().string() << '\n';
}

void HashStreamWriter::attach(Observer* observer) {
    Observable::attach(observer);
    if (_hash_strategy) {
        _hash_strategy->attach(observer);
    }
}

void HashStreamWriter::preProcess(File& file) {
    notify(*this, NewFileMessage(file.getPath().string()));
}