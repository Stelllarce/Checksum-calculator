#include <stdexcept>
#include <cassert>
#include "File.hpp"

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
    return _size;
}

bool File::setSize(size_t size) {
    if (size <= 0) {
        return false;
    }
    _size = size;
    return true;
}

void File::write(std::istream& from) {
    if (!from.good()) {
        throw std::ios_base::failure("Error: Writing contents from bad stream");
    }
    from >> _contents;
    if (!setSize(read().size())) {
        throw std::runtime_error("Bad content size");
    }
    if (from.eof()) from.clear();
}

std::string File::read() const {
    return _contents;
}