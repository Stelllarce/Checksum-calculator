#pragma once
#include "FileObject.hpp"
#include "Directory.hpp"
#include <vector>
#include <fstream>
#include <filesystem>

/**
 * @class File
 * @brief Leaf in the Composite pattern that represents a real file.
 *
 * Stores its full filesystem path and a cached size. Reading is done
 * in binary mode so arbitrary file contents can be handled.
 */
class File : public FileObject {
public:
    /**
     * @brief Construct a file object with a name within an owning directory.
     * @param name File name (no path separators expected).
     * @param owner Owning FileObject (typically a Directory); must not be nullptr.
     * @throws std::logic_error if owner is nullptr.
     */
    File(const std::filesystem::path& name, FileObject* owner);

    /// @return The file name component.
    std::string getName() const override;

    /**
     * @brief Return the file size.
     *
     * If a size was previously set explicitly via setSize, that cached value
     * is returned. Otherwise, if the path exists on the filesystem, the size
     * is fetched via std::filesystem::file_size and cached.
     * If the file does not exist, returns 0.
     */
    size_t getSize() override;

    /**
     * @brief Set the cached size (useful for tests/mocks).
     * @return true (size is always cached successfully).
     */
    bool setSize(size_t) override;

    /**
     * @brief Read the file contents from disk in binary mode.
     * @throws std::ios_base::failure if the file cannot be opened or read.
     */
    std::vector<char> read() const override;

#ifdef DEBUG
    /**
     * @brief Read the file contents from a provided input stream (for testing).
     * @param stream Input stream to read from.
     * @throws std::ios_base::failure if the stream cannot be read.
     */
    std::vector<char> read(std::istream& stream) const;
#endif

    void accept(DirectoryIterationVisitor& visitor) override;

private:
    size_t _size = 0; 
};
