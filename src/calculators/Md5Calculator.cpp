#include "Md5Calculator.hpp"

MD5 Md5Calculator::md5 = MD5();

std::string Md5Calculator::calculate(const std::string& data) noexcept {
    md5.reset();
    md5(data);
    return md5.getHash();
}