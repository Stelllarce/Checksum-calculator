#include "SHA1Calculator.hpp"

SHA1 SHA1Calculator::sha1 = SHA1();

std::string SHA1Calculator::calculate(const std::string& data) noexcept {
    sha1.reset();
    sha1(data);
    return sha1.getHash();
}