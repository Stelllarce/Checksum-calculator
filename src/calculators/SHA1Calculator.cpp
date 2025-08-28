#include "SHA1Calculator.hpp"
#include "progress-indicator-observers/Message.hpp"

SHA1 SHA1Calculator::sha1 = SHA1();

std::string SHA1Calculator::calculate(const std::string& data) noexcept {
    sha1.reset();

    constexpr std::size_t CHUNK = 1024; // 1 KiB ticks
    std::size_t processed = 0;

    const char* ptr = data.data();
    std::size_t remaining = data.size();

    while (remaining > 0) {
        std::size_t take = remaining < CHUNK ? remaining : CHUNK;
        sha1.add(ptr, take);
        ptr += take;
        remaining -= take;
        processed += take;
        notify(*this, BytesReadMessage(static_cast<std::uint64_t>(processed)));
    }

    return sha1.getHash();
}