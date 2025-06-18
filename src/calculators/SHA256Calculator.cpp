#include "SHA256Calculator.hpp"

SHA256 SHA256Calculator::sha256 = SHA256();

std::string SHA256Calculator::calculate(const std::string& data) noexcept {
    sha256.reset();
    sha256(data);
    return sha256.getHash();
}