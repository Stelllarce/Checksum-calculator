#pragma once
#include <string>

/**
 * @interface for all checksum algorithms
 */
class ChecksumCalculator {
public:
    /**
     * @brief Calculate checksum for given data
     * @param data - data to calculate checksum for
     * @return checksum
     */
    virtual std::string calculate(const std::string& data) noexcept = 0;
    virtual ~ChecksumCalculator() = default;
};