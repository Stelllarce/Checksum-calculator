#include <stdexcept>
#include <fstream>
#include <vector>
#include <filesystem>
#include "File.hpp"
#include "directory-iteration-visitors/DirectoryIterationVisitor.hpp"

File::File(const std::filesystem::path& name, FileObject* owner)
    : FileObject(name, owner)
{
    if (!owner) {
        throw std::logic_error("File without owning directory is invalid");
    }
}

std::string File::getName() const {
    if (_filepath.has_filename()) {
        return _filepath.filename().string();
    }
    return std::string();
}

size_t File::getSize() {
    if (_size != 0) {
        return _size;
    }
    std::error_code ec;
    if (std::filesystem::exists(_filepath, ec) && std::filesystem::is_regular_file(_filepath, ec)) {
        auto sz = std::filesystem::file_size(_filepath, ec);
        if (!ec) {
            _size = static_cast<size_t>(sz);
            return _size;
        }
    }
    return 0;
}

bool File::setSize(size_t size) {
    _size = size;
    return true;
}

std::vector<char> File::read() const {
    std::ifstream file_stream(_filepath, std::ios::in | std::ios::binary);
    if (!file_stream.is_open()) {
        throw std::ios_base::failure("Error: Failed to open file for reading: " + _filepath.string());
    }

    std::error_code ec;
    size_t file_size = 0;
    if (std::filesystem::exists(_filepath, ec) && std::filesystem::is_regular_file(_filepath, ec)) {
        auto sz = std::filesystem::file_size(_filepath, ec);
        if (ec) {
            file_stream.close();
            throw std::ios_base::failure("Error: Failed to get file size: " + _filepath.string());
        }
        file_size = static_cast<size_t>(sz);
    } else {
        file_stream.close();
        throw std::ios_base::failure("Error: File does not exist: " + _filepath.string());
    }

    std::vector<char> contents(file_size);
    if (file_size > 0) {
        file_stream.read(contents.data(), static_cast<std::streamsize>(file_size));
        if (file_stream.fail() && !file_stream.eof()) {
            file_stream.close();
            throw std::ios_base::failure("Error: Failed to read data from file: " + _filepath.string());
        }
    }

    file_stream.close();
    return contents;
}

#ifdef DEBUG
std::vector<char> File::read(std::istream& stream) const {
    stream.seekg(0, std::ios::end);
    if (stream.fail()) {
        throw std::ios_base::failure("Error: Failed to seek to end of stream");
    }
    
    size_t stream_size = static_cast<size_t>(stream.tellg());
    stream.seekg(0, std::ios::beg);
    if (stream.fail()) {
        throw std::ios_base::failure("Error: Failed to seek to beginning of stream");
    }

    std::vector<char> contents(stream_size);
    if (stream_size > 0) {
        stream.read(contents.data(), static_cast<std::streamsize>(stream_size));
        if (stream.fail() && !stream.eof()) {
            throw std::ios_base::failure("Error: Failed to read data from stream");
        }
    }

    return contents;
}
#endif

void File::accept(DirectoryIterationVisitor& visitor) {
    visitor.visitFile(*this);
}
