#pragma once
#include <string>
#include "progress-indicator-observers/Observable.hpp"

/**
 * @interface ChecksumCalculator
 * @brief Strategy interface for checksum algorithms. Observable to report progress.
 */
class ChecksumCalculator : public Observable {
public:
    /// Calculate checksum for given data
    virtual std::string calculate(const std::string& data) noexcept = 0;
    virtual std::string getAlgorithmName() const noexcept = 0;
    virtual ~ChecksumCalculator() = default;
};