#include "Directory.hpp"
#include "File.hpp"

size_t Directory::calculateSize() const {
    size_t totalSize = 0;
    for (const auto& [path, child] : _children) {
        totalSize += child->getSize();
    }
    return totalSize;
}

Directory::Directory(const std::filesystem::path& name, FileObject* owner) 
    : FileObject(name, owner) {}

const std::string& Directory::getName() const {
    static thread_local std::string filename;
    filename = _filepath.filename().string();
    return filename;
}

size_t Directory::getSize() {
    return calculateSize();
}

bool Directory::add(std::unique_ptr<FileObject> obj) {
    if (obj == nullptr) {
        return false;
    }
    auto name = std::filesystem::path(obj->getName());
    // Same name of a FileObject cannot be added to the Directory
    if (_children.find(name) != _children.end()) {
        return false; 
    }

    _children[name] = std::move(obj);
    _children[name]->setOwner(*this);
    return true;
}

bool Directory::remove(const std::filesystem::path& path) {
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

FileObject* Directory::getChild(const std::filesystem::path& name) noexcept {
    auto it = _children.find(name);
    if (it != _children.end()) {
        return it->second.get();
    }
    return nullptr;
}

const FileObject* Directory::getChild(const std::filesystem::path& name) const noexcept {
    auto it = _children.find(name);
    if (it != _children.end()) {
        return it->second.get();
    }
    return nullptr;
}

Directory* Directory::createSubdirectory(const std::filesystem::path& name) {
    auto subdir = std::make_unique<Directory>(name, this);
    Directory* subdirPtr = subdir.get();
    
    if (!add(std::move(subdir))) {
        throw std::runtime_error("Failed to add subdirectory '" + name.string() + "' to parent directory");
    }
    
    return subdirPtr;
}

File* Directory::createFile(const std::filesystem::path& name) {
    auto file = std::make_unique<File>(name, this);
    File* filePtr = file.get();

    if (!add(std::move(file))) {
        throw std::runtime_error("Failed to add file '" + name.string() + "' to parent directory");
    }
    
    return filePtr;
}