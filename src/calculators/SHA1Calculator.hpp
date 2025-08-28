#pragma once
#include "ChecksumCalculator.hpp"
#include "sha1.h"

/**
 * @class SHA1Calculator
 * @brief Class for calculating SHA1 checksums
 */
class SHA1Calculator : public ChecksumCalculator {
public:
    /**
     * @brief Calculate SHA1 checksum for given data
     * @param data - data to calculate checksum for
     * @return SHA1 checksum as a hexadecimal string
     */
    std::string calculate(const std::string& data) noexcept override;
    std::string getAlgorithmName() const noexcept override { return "sha1"; }
private:

    
    static SHA1 sha1;  ///< Shared SHA1 instance for checksum calculations
};