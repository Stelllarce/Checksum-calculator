#pragma once
#include "progress-indicator-observers/Observer.hpp"
#include "progress-indicator-observers/Observable.hpp"
#include "progress-indicator-observers/Message.hpp"
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

/**
 * @class ProgressReporter
 * @brief Prints per-file progress + overall percentage, speed, and ETA.
 */
class ProgressReporter : public Observer {
public:
    explicit ProgressReporter(std::uint64_t totalExpectedBytes, std::ostream& os = std::cout);

    void start();

    void update(Observable& sender, const Message& m) override;

private:
    void refreshDisplay();

    std::ostream& _os;
    std::string _currentPath = "";
    std::uint64_t _currentBytes = 0;          
    std::uint64_t _bytesTotalProcessed = 0;   
    std::uint64_t _totalExpected = 0;         
    std::chrono::steady_clock::time_point _start = std::chrono::steady_clock::now();
};