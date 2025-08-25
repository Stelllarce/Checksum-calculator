#include "FileObject.hpp"
#include "File.hpp"

FileObject::FileObject(const std::filesystem::path& name, FileObject* owner) 
    : _owner(owner) {
    
    if(name.empty()) {
        throw std::runtime_error("File object must have a non-empty name");
    }
    
    buildPath(name, owner);
}

void FileObject::buildPath(const std::filesystem::path& name, FileObject* owner) {
    
    // If there is an owner (not nullptr)
    if (owner) {
        // Check type of object pointer
        auto owner_f = dynamic_cast<File*>(owner);

        // If owner is a file (not allowed - only directories can own other objects)
        if (owner_f) {
            throw std::runtime_error("Owner object cannot be of type File*");
        }
        
        // Owner must be a Directory (or Link, but we'll treat it as a generic FileObject)
        // If there is a path already
        if (!owner->_filepath.empty()) {
            _filepath = owner->_filepath / name;
        } else { 
            throw std::logic_error("This should not be reached."
                 "Owning directory has no saved path for some reason, even its own name..");
        }
    } else { // Root folder
        _filepath = name;
    }
}