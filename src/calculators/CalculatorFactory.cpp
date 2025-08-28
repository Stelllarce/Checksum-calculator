#include "calculators/CalculatorFactory.hpp"
#include "Md5Calculator.hpp"
#include "SHA1Calculator.hpp"
#include "SHA256Calculator.hpp"

std::unique_ptr<ChecksumCalculator> CalculatorFactory::create(const std::string& type) {
    if (type == "md5") {
        return std::make_unique<Md5Calculator>();
    } else if (type == "sha1") {
        return std::make_unique<SHA1Calculator>();
    } else if (type == "sha256") {
        return std::make_unique<SHA256Calculator>();
    }
    return nullptr;
}
