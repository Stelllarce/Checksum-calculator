#include "FileObject.hpp"
#include "Directory.hpp"

FileObject::FileObject(const std::string& name, FileObject* owner) 
    : _owner(owner) {
    // If there is an owner (not nullptr)
    if (owner) {
        // Check type of object pointer
        auto owner_d = dynamic_cast<Directory*>(owner);

        // If owner is anything but a directory
        if (!owner_d) {
            throw std::runtime_error("Owner object can only be of type Directory*");
        }
        
        // If there is a path already
        if (!owner_d->_filepath.empty()) {
            _filepath = owner_d->_filepath + PATH_SEPARATOR + name;
        } else { 
            throw std::logic_error("This should not be reached."
                 "Owning directory has no saved path for some reason, even its own name..");
        }
    } else { // Root folder
        _filepath = name;
    } 
}
