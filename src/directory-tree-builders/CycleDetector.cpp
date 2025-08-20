#include "CycleDetector.hpp"

bool CycleDetector::check(const std::filesystem::path& path) {
    
    std::filesystem::path canonical;
    
    try {
        canonical = std::filesystem::canonical(path);
    } 
    catch (std::filesystem::filesystem_error& e) {
        std::cerr << "Path does not exist" << std::endl;
        return false;
    }
    catch (...) {
        std::cerr << "Memory allocation error when calling canonical" << std::endl;
        return false;
    }
    
    if (_visited_paths.count(canonical)) {
        return true;
    }

    _visited_paths.insert(canonical);
    return false;
}
