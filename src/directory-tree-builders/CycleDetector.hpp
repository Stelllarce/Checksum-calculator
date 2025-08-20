#pragma once
#include <unordered_set>
#include <filesystem>
#include <stdexcept>
#include <iostream>

/**
 * @brief Class for tracking circular dependencies between links
 */
class CycleDetector {
public:
    bool check(const std::filesystem::path& path);

private:
    std::unordered_set<std::filesystem::path> _visited_paths;
};