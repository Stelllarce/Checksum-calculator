#pragma once
#include <filesystem>

class DetectionStrategy {
public:
    virtual bool check(const std::filesystem::path& path) = 0;
};