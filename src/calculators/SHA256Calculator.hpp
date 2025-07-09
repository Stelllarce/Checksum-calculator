#pragma once
#include "ChecksumCalculator.hpp"
#include "sha256.h"

/**
 * @class SHA256Calculator
 * @brief Class for calculating SHA256 checksums
 */
class SHA256Calculator : public ChecksumCalculator {
public:
    /**
     * @brief Calculate SHA256 checksum for given data
     * @param data - data to calculate checksum for
     * @return SHA256 checksum as a hexadecimal string
     */
    std::string calculate(const std::string& data) noexcept override;
private:
    
    static SHA256 sha256; ///< Shared SHA256 instance for checksum calculations
};