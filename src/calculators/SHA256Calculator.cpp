#include "SHA256Calculator.hpp"
#include "progress-indicator-observers/Message.hpp"

SHA256 SHA256Calculator::sha256 = SHA256();

std::string SHA256Calculator::calculate(const std::string& data) noexcept {
    sha256.reset();

    constexpr std::size_t CHUNK = 1024; // 1 KiB ticks
    std::size_t processed = 0;

    const char* ptr = data.data();
    std::size_t remaining = data.size();

    while (remaining > 0) {
        std::size_t take = remaining < CHUNK ? remaining : CHUNK;
        sha256.add(ptr, take);
        ptr += take;
        remaining -= take;
        processed += take;
        notify(*this, BytesReadMessage(static_cast<std::uint64_t>(processed)));
    }

    return sha256.getHash();
}