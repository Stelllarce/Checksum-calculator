#pragma once
#include "FileObject.hpp"
#include "Directory.hpp"
#include <vector>
#include <fstream>

/**
 * @class child class, that represents the "Leaf" in the Composite pattern.
 * 
 * It represents a file in the file system.
 */
class File : public FileObject {
public:
    File(const std::filesystem::path& name, FileObject* owner);

    /**
     * @throws std::logic_error if no path separator symbol found
     */
    std::string getName() const override;
    
    size_t getSize() override;
    
    bool setSize(size_t) override;

    /**
     * @brief Reading binary data from a file
     * @return vector of binary data read from the file
     * @throws std::ios_base::failure when file cannot be opened or read
     */
    std::vector<char> read() const override;


    void accept(DirectoryIterationVisitor& visitor) override;
private:
    size_t _size = 0;

    /**
     * @todo Maybe add a struct with state flags?
     */
};
/**
 * Class can be either extended to a binary file by having a BinaryFile inherit it
 * or by making this a common interface and having a TextFile and BinaryFile classes
 * inherit it
 * Binary file - vector<char>, Text file - string
 */
