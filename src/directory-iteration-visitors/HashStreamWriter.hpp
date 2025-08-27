#pragma once
#include "DirectoryIterationVisitor.hpp"
#include "calculators/ChecksumCalculator.hpp"
#include <memory>
#include <iostream>

class HashStreamWriter : public DirectoryIterationVisitor {
public:
    HashStreamWriter(std::unique_ptr<ChecksumCalculator> calc, std::ostream& os);

    void visitFile(File& file) override;

    void visitDirectory(Directory& dir) override;

    void visitLink(Link& link) override;
protected:
    void applyAlgorithm(File& file) override;
private:
    std::unique_ptr<ChecksumCalculator> _hash_strategy;
};