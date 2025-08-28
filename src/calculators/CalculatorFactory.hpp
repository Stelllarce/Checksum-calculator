#pragma once
#include "ChecksumCalculator.hpp"
#include <memory>
#include <string> 

class CalculatorFactory {
public:
    static std::unique_ptr<ChecksumCalculator> create(const std::string& type);
};