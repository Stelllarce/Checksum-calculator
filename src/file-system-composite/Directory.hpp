#pragma once

#include "FileObject.hpp"
#include <map>
#include <memory>

/**
 * @class child class, that represents the "Composite" in the Composite pattern.
 * 
 * It represents a directory in the file system.
 */
class Directory: public FileObject {
public:
    Directory(const std::string& name, FileObject* owner = nullptr);

    /**multiple
     * @brief Factory method to create a subdirectory and add it to this directory
     * @param name - name of the new subdirectory
     * @return raw pointer to the created directory (ownership transferred to this directory)
     */
    Directory* createSubdirectory(const std::string& name) override;

    /**
     * @brief Factory method to create a file and add it to this directory
     * @param name - name of the new file
     * @return raw pointer to the created file (ownership transferred to this directory)
     */
    class File* createFile(const std::string& name) override;

    /**
     * @brief Add a Component to the Composite collection
     * @return true if the fileObject was added, false otherwise
     */
    bool add(std::unique_ptr<FileObject>) override;

    /**
     * @brief Remove a Component from the Composite collection
     * @return true if the fileObject was removed, false otherwise
     */
    bool remove(const std::string&) override;

    /**
     * @brief Get a child Component from the Composite collection
     * @param index - index of the child to get
     * @return pointer to the child FileObject, or nullptr if not found
     */
    FileObject* getChild(const std::string& name) noexcept override;
    const FileObject* getChild(const std::string& name) const noexcept override;

    std::string getName() const override ;

    std::string getPath() const override;

    size_t getSize() override;
private:
    /**
     * @brief Calculate the size of the whole 
     * directory by summing all children's sizes
     */
    size_t calculateSize() const;

    /**
     * @brief name of file - pointer to object
     */
    std::map<std::string, std::unique_ptr<FileObject>> _children;
};