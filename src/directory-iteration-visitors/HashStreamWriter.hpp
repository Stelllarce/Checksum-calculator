#pragma once
#include "DirectoryIterationVisitor.hpp"
#include "calculators/ChecksumCalculator.hpp"
#include "progress-indicator-observers/Message.hpp"
#include "progress-indicator-observers/Observable.hpp"
#include <memory>
#include <iostream>

/**
* @class HashStreamWriter
* @brief Visitor that computes a checksum for each visited File and writes it to an output stream.
*
* Output format: `<hash><two spaces><path>\n`
*/
class HashStreamWriter : public DirectoryIterationVisitor, public Observable {
public:
    /**
    * @brief Construct a writer with a concrete checksum calculator and an output stream.
    * @param calc Ownership of a checksum calculator strategy (MD5/SHA1/SHA256, etc.)
    * @param os Output stream to write lines to.
    */
    HashStreamWriter(std::unique_ptr<ChecksumCalculator> calc, std::ostream& os);

    void visitFile(File& file) override;
    void visitDirectory(Directory& dir) override;
    void visitLink(Link& link) override;

    void attach(Observer* observer) override;
protected:
    void preProcess(File& file) override;
    void applyAlgorithm(File& file) override;
private:
    std::unique_ptr<ChecksumCalculator> _hash_strategy;
};