#pragma once

#include <memory>
#include <string>
#include <filesystem>
#include <initializer_list>
#include <optional>
#include "../file-system-composite/FileObject.hpp"
#include "../file-system-composite/Directory.hpp"
#include "../file-system-composite/Link.hpp"
#include "../file-system-composite/File.hpp"


/**
 * @class DirectoryStructureBuilder
 * @brief Abstract builder class for creating in-memory directory structures
 */
class DirectoryStructureBuilder {
public:
    virtual ~DirectoryStructureBuilder() = default;
    
    virtual void startBuildDirectory(const std::filesystem::path& name) {}
    
    virtual void endBuildDirectory() {}

    virtual Directory* buildLink(const std::filesystem::path& name, const std::filesystem::path& target) { return nullptr; }

    virtual void buildFile(const std::filesystem::path& name) {}

    // virtual std::unique_ptr<Directory> getTree() const { return nullptr; }

    virtual Directory* getTree() const { return nullptr; }

protected:
    DirectoryStructureBuilder() = default;
};