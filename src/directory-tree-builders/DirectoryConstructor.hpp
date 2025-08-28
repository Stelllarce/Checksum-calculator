#pragma once
#include "DirectoryStructureBuilder.hpp"
#include <initializer_list>

/**
 * @brief Director class in the Builder structure
 * Creates the in-memory file structure
 */
class DirectoryConstructor {
public:
    explicit DirectoryConstructor(DirectoryStructureBuilder& builder);

    void construct(std::initializer_list<std::filesystem::path> rootPaths);

private:
    void traverse(const std::filesystem::path& path);

    DirectoryStructureBuilder& _builder;
};