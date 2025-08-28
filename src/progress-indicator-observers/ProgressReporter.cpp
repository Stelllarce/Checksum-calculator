#include "ProgressReporter.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

explicit ProgressReporter::ProgressReporter(std::uint64_t totalExpectedBytes, std::ostream& os = std::cout)
        : _os(os), _totalExpected(totalExpectedBytes) {}

void ProgressReporter::start() {
    _bytesTotalProcessed = 0;
    _currentBytes = 0;
    _start = std::chrono::steady_clock::now();
}

void ProgressReporter::update(Observable& sender, const Message& m) {
    if (m.type == Message::Type::NewFile) {
        const auto& msg = static_cast<const NewFileMessage&>(m);
        _os << '\n';
        _currentPath = msg.path;
        _currentBytes = 0;
        refreshDisplay();
    } else if (m.type == Message::Type::BytesRead) {
        const auto& msg = static_cast<const BytesReadMessage&>(m);
        std::uint64_t delta = 0;
        if (msg.bytesRead >= _currentBytes) delta = msg.bytesRead - _currentBytes;
        _currentBytes = msg.bytesRead;
        _bytesTotalProcessed += delta;
        refreshDisplay();
    }
}

static std::string humanizeBytes(std::uint64_t n) {
    const char* units[] = {"B","KiB","MiB","GiB","TiB"};
    double d = static_cast<double>(n);
    int u = 0;
    while (d >= 1024.0 && u < 4) { d /= 1024.0; ++u; }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(u==0?0:1) << d << ' ' << units[u];
    return oss.str();
}

void ProgressReporter::refreshDisplay() {
    using namespace std::chrono;
    auto now = steady_clock::now();
    auto elapsed = duration_cast<seconds>(now - _start).count();
    double speed = elapsed > 0 ? static_cast<double>(_bytesTotalProcessed) / elapsed : 0.0;
    double percent = _totalExpected > 0 ? (100.0 * _bytesTotalProcessed / static_cast<double>(_totalExpected)) : 0.0;
    std::uint64_t etaSecs = 0;
    if (speed > 0.0 && _totalExpected > _bytesTotalProcessed) {
        etaSecs = static_cast<std::uint64_t>((_totalExpected - _bytesTotalProcessed) / speed);
    }

    _os << '\r'
        << "Processing " << _currentPath
        << "... " << _currentBytes << " byte(s) read"
        << " | total " << std::fixed << std::setprecision(1) << percent << '%'
        << " | " << humanizeBytes(static_cast<std::uint64_t>(speed)) << "/s"
        << " | ETA " << etaSecs << "s";
    _os.flush();
}