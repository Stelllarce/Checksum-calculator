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
    : FileObject(name, owner) {}

std::string Directory::getName() const {
    size_t pos = _filepath.find_last_of(PATH_SEPARATOR);
    return (pos == std::string::npos) ? _filepath : _filepath.substr(pos + 1);
}

std::string Directory::getPath() const {
    return _filepath;
}

size_t Directory::getSize() {
    return calculateSize();
}

bool Directory::add(std::unique_ptr<FileObject> obj) {
    if (obj == nullptr) {
        return false;
    }
    auto name = obj->getName();
    // Same name of a FileObject cannot be added to the Directory
    if (_children.find(name) != _children.end()) {
        return false; 
    }

    _children[name] = std::move(obj);
    _children[name]->setOwner(*this);
    return true;
}

bool Directory::remove(const std::string& path) {
    if (path.empty()) {
        return false;
    }
    auto it = _children.find(path);
    if (it != _children.end()) {
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
    
    if (!add(std::move(subdir))) {
        throw std::runtime_error("Failed to add subdirectory '" + name + "' to parent directory");
    }
    
    return subdirPtr;
}

File* Directory::createFile(const std::string& name) {
    auto file = std::make_unique<File>(name, this);
    File* filePtr = file.get();

    if (!add(std::move(file))) {
        throw std::runtime_error("Failed to add file '" + name + "' to parent directory");
    }
    
    return filePtr;
}