#pragma once
#include "FileObject.hpp"
#include "Directory.hpp"

/**
 * @class child class, that represents the "Leaf" in the Composite pattern.
 * It represents a file in the file system.
 */
class File : public FileObject {
public:
    File(const std::string& name, FileObject* owner);

    /**
     * @throws std::logic_error if no path separator symbol found
     */
    std::string getName() const override;

    std::string getPath() const override;
    
    size_t getSize() override;
    
    bool setSize(size_t) override;

    /**
     * @brief Writing contents to a file object, acts as a setter for @var _contents
     * @param from - stream to get the contents of the file
     * @throws std::ios_base::failure when bad stram is passed
     * @throws std::runtime_error when size is not set properly
     */
    void write(std::istream& from) override;
    std::string read() const override;
private:
    std::string _contents;

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
