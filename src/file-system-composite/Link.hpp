#pragma once
#include "FileObject.hpp"

class Link : public FileObject {
public:
    Link(const std::filesystem::path& name, const std::filesystem::path& target_path, FileObject* owner);

    // Pure virtual methods from FileObject
    const std::string& getName() const override;
    size_t getSize() override;

    // Link-specific methods
    const std::filesystem::path& getTarget() const override;
    bool setResolveTarget(std::unique_ptr<FileObject> t) override;
    FileObject* getResolvedTarget() const override;
private:
    /**
     * @note this could be either relative or full path
     */
    std::filesystem::path _target_name;
    /**
     * @brief the real target the link resolves to
     */
    std::unique_ptr<FileObject> _resolved_target;
};