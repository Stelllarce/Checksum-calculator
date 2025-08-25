#pragma once
#include "DirectoryStructureBuilder.hpp"
#include <vector>

class BaseBuilder : public DirectoryStructureBuilder {
public:

void startBuildDirectory(const std::filesystem::path& name) override;

void endBuildDirectory() override;

void buildFile(const std::filesystem::path& name) override;

// std::unique_ptr<Directory> getTree() override;

Directory* getTree() const override;

protected:
    BaseBuilder();    
    
    std::unique_ptr<Directory> _root;
    std::vector<Directory*> _build_stack;
};