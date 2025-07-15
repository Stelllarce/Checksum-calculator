#include "Directory.hpp"
#include "File.hpp"

size_t Directory::calculateSize() const {
    size_t totalSize = 0;
    for (const auto& [path, child] : _children) {
        totalSize += child->getSize();
    }
    return totalSize;
}

Directory::Directory(const std::string& name, FileObject* owner) 
    : FileObject(name, owner) {
        // Directory construction complete - parent addition should be handled by client code
    }

std::string Directory::getName() const {
    size_t pos = _filepath.find_last_of(PATH_SEPARATOR);
    return (pos == std::string::npos) ? _filepath : _filepath.substr(pos + 1);
}

std::string Directory::getPath() const {
    return _filepath;
}

size_t Directory::getSize() {
    if (_size == 0) {
        _size = calculateSize();
    }
    return _size;
}

bool Directory::add(std::unique_ptr<FileObject>& obj) {
    if (obj == nullptr) {
        return false;
    }
    auto path = obj->getPath();
    // Same path of a FileObject cannot be added to the Directory
    if (_children.find(path) != _children.end()) {
        return false; 
    }

    _children[path] = std::move(obj);
    _children[path]->setOwner(*this);
    _size += _children[path]->getSize();
    return true;
}

bool Directory::remove(std::unique_ptr<FileObject>& obj) {
    if (obj == nullptr) {
        return false;
    }

    auto path = obj->getPath();
    auto it = _children.find(path);
    if (it != _children.end()) {
        _size -= it->second->getSize();
        _children.erase(it);
        return true;
    }
    return false;
}

FileObject* Directory::getChild(const std::string& name) noexcept {
    auto it = _children.find(name);
    if (it != _children.end()) {
        return it->second.get();
    }
    return nullptr;
}

const FileObject* Directory::getChild(const std::string& name) const noexcept {
    auto it = _children.find(name);
    if (it != _children.end()) {
        return it->second.get();
    }
    return nullptr;
}

Directory* Directory::createSubdirectory(const std::string& name) {
    auto subdir = std::make_unique<Directory>(name, this);
    Directory* subdirPtr = subdir.get();
    
    // Convert to base type for add function
    std::unique_ptr<FileObject> basePtr = std::move(subdir);
    if (!add(basePtr)) {
        throw std::runtime_error("Failed to add subdirectory '" + name + "' to parent directory");
    }
    
    return subdirPtr;
}

File* Directory::createFile(const std::string& name) {
    auto file = std::make_unique<File>(name, this);
    File* filePtr = file.get();
    
    // Convert to base type for add function
    std::unique_ptr<FileObject> basePtr = std::move(file);
    if (!add(basePtr)) {
        throw std::runtime_error("Failed to add file '" + name + "' to parent directory");
    }
    
    return filePtr;
}