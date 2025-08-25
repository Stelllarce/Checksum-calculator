#pragma once
#include "DirectoryStructureBuilder.hpp"
#include <initializer_list>


class DirectoryConstructor {
public:
    explicit DirectoryConstructor(DirectoryStructureBuilder& builder);

    void construct(std::initializer_list<std::filesystem::path> rootPaths);

private:
    void traverse(const std::filesystem::path& path);

    DirectoryStructureBuilder& _builder;
};