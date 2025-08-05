#pragma once
#include <string>
#include <cstddef>
#include <memory>
#include <istream>

/**
 * File separator, system dependent.
 */
#if defined(_WIN32)
    const char PATH_SEPARATOR = '\\';
#else
    const char PATH_SEPARATOR = '/';
#endif

class Directory;

/**
 * @class abstract class, serves as the "Component" class 
 * in the Composite pattern. 
 * 
 * Client should interact with this class
 * 
 * Represents an abstraction for all objects in the file system
 */
class FileObject {
public:
    virtual ~FileObject() = default;

    /**
     * @return the string after the last slash in the path.
     */
    virtual std::string getName() const = 0;

    /**
     * @return _filepath - full path to the file or directory
     */
    virtual std::string getPath() const = 0;

    /**
     * @return the size in bytes of the file or directory.
     */
    virtual size_t getSize() = 0;

    virtual bool setSize(size_t size) { return false; }
    
    /**
     * @brief Add a Component to the Composite collection
     * @return true if the fileObject was added, false otherwise
     * Defaults to false so that it can not be overriden by the Leaf class.
     */
    virtual bool add(std::unique_ptr<FileObject>) { return false; }
    
    /**
     * @brief Remove a Component from the Composite collection
     * @return true if the fileObject was removed, false otherwise
     * Defaults to false so that it can not be overriden by the Leaf class.
     */
    virtual bool remove(const std::string&) { return false; }
    
    /**
     * @brief Get a child Component from the Composite collection
     * @param name - name of the child to get
     * @return pointer to the child FileObject, or nullptr if not found
     * Defaults to nullptr so that it can not be overriden by the Leaf class.
     */
    virtual FileObject* getChild(const std::string& name) noexcept { return nullptr; }
    virtual const FileObject* getChild(const std::string& name) const noexcept { return nullptr; } 

    /**
     * @brief Factory method to create a file and add it to this FileObject
     * @param name - name of the new file
     * @return raw pointer to the created file (ownership transferred to this FileObject)
     * Defaults to nullptr so that it can not be overriden by the Leaf class.
     */
    virtual class File* createFile(const std::string& name) { return nullptr; }

    /**
     * @brief Factory method to create a subdirectory and add it to this FileObject
     * @param name - name of the new subdirectory
     * @return raw pointer to the created directory (ownership transferred to this FileObject)
     * Defaults to nullptr so that it can not be overriden by the Leaf class.
     */
    virtual class Directory* createSubdirectory(const std::string& name) { return nullptr; }

    /**
     * @brief set owner of current object
     */
    void setOwner(FileObject& owner) {
        _owner = &owner;
    }
    
    /**
     * @return pointer to owner of current object
     */
    FileObject* getOwner() const {
        return _owner;
    }

    /**
     * @brief Writing contents to a file object
     * @param from - stream to get the contents of the file
     */
    virtual void write(std::istream& from) {}
    virtual std::string read() const { return ""; } 

protected:
    /**
     * @param name - name of the component
     * Appends to path if already exists, else, the name is the new starting point of path
     * 
     * @note This may need changes, having Directory explicitly stated
     * There is no other way to check without repeating code
     */
    FileObject(const std::string& name, FileObject* owner);


    std::string _filepath; ///< Whole path to object

    /**
     * Non-owning refrence to owner object
     * Should not be released
     */
    FileObject* _owner = nullptr;
};