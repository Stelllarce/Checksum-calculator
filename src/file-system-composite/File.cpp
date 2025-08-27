#include <stdexcept>
#include <cassert>
#include <fstream>
#include <vector>
#include "File.hpp"
#include "directory-iteration-visitors/DirectoryIterationVisitor.hpp"

File::File(const std::filesystem::path& name, FileObject* owner) 
    : FileObject(name, owner) {
        if (!owner) {
            throw std::logic_error("File without owning directory is invalid");
        }
    }

std::string File::getName() const {
    if (_filepath.has_filename()) {
        return _filepath.filename().string();
    }
    throw std::logic_error("File without owning directory is invalid");
}

size_t File::getSize() {
    try {
        // Get the actual file size from the filesystem
        if (std::filesystem::exists(_filepath)) {
            _size = std::filesystem::file_size(_filepath);
        }
    } catch (const std::filesystem::filesystem_error&) {
        // If we can't get the file size, keep the current _size value
    }
    return _size;
}

bool File::setSize(size_t size) {
    if (size <= 0) {
        return false;
    }
    _size = size;
    return true;
}

std::vector<char> File::read() const {
    std::ifstream file_stream(_filepath, std::ios::binary);
    if (!file_stream.is_open()) {
        throw std::ios_base::failure("Error: Could not open file for reading: " + _filepath.string());
    }
    
    file_stream.seekg(0, std::ios::end);
    size_t file_size = file_stream.tellg();
    file_stream.seekg(0, std::ios::beg);
    
    std::vector<char> contents(file_size);
    file_stream.read(contents.data(), file_size);
    
    if (file_stream.fail() && !file_stream.eof()) {
        throw std::ios_base::failure("Error: Failed to read data from file: " + _filepath.string());
    }
    
    file_stream.close();
    return contents;
}

void File::accept(DirectoryIterationVisitor& visitor) {
    visitor.visitFile(*this);
}
