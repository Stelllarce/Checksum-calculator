#include "HashStreamWriter.hpp"
#include "file-system-composite/File.hpp"
#include "calculators/ChecksumCalculator.hpp"

HashStreamWriter::HashStreamWriter(std::unique_ptr<ChecksumCalculator> calc, std::ostream& os) 
    : DirectoryIterationVisitor(os), _hash_strategy(std::move(calc)) {
        if (!calc) {
            throw std::runtime_error("Calculator cant be null!");
        }
}


void HashStreamWriter::visitFile(File& file) {
    processFile(file);   
}

void HashStreamWriter::applyAlgorithm(File& file) {
    std::vector<char> content = file.read();
    std::string content_str(content.begin(), content.end());
    std::string checksum = _hash_strategy->calculate(content_str);
    _output << checksum;
}