#include "ChecksumCalculator.hpp"
#include "md5.h" // Include external md5 library

class Md5Calculator : public ChecksumCalculator {
public:
    /**
     * @brief Calculate MD5 checksum for given data
     * @param data - data to calculate checksum for
     * @return MD5 checksum as a hexadecimal string
     */
    std::string calculate(const std::string& data) noexcept override;

private:
    /// Shared MD5 instance for checksum calculations
    static MD5 md5;
};

MD5 Md5Calculator::md5 = MD5();