#include "DirectoryIterationVisitor.hpp"

DirectoryIterationVisitor::DirectoryIterationVisitor(std::ostream& os) : _output(os) {}


void DirectoryIterationVisitor::processFile(File& file) {
    preProcess(file);
    applyAlgorithm(file);
    postProcess(file);
}