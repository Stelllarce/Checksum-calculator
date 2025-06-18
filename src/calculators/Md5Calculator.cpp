#include "Md5Calculator.hpp"

std::string Md5Calculator::calculate(const std::string& data) noexcept {
    md5.reset();
    md5(data);
    return md5.getHash();
}