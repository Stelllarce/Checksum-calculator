#include "Md5Calculator.hpp"
#include "progress-indicator-observers/Message.hpp"

MD5 Md5Calculator::md5 = MD5();

std::string Md5Calculator::calculate(const std::string& data) noexcept {
    md5.reset();

    constexpr std::size_t CHUNK = 1024; // 1 KiB ticks
    std::size_t processed = 0;

    const char* ptr = data.data();
    std::size_t remaining = data.size();

    while (remaining > 0) {
        std::size_t take = remaining < CHUNK ? remaining : CHUNK;
        md5.add(ptr, take);
        ptr += take;
        remaining -= take;
        processed += take;
        notify(*this, BytesReadMessage(static_cast<std::uint64_t>(processed)));
    }

    return md5.getHash();
}