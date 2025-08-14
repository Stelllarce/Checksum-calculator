#include "Link.hpp"

Link::Link(const std::filesystem::path& name, const std::filesystem::path& target_path, FileObject* owner) :
                FileObject(name, owner), _target_name(target_path) {}

const std::string& Link::getName() const {
    static thread_local std::string filename;
    if (_filepath.has_filename()) {
        filename = _filepath.filename().string();
        return filename;
    }
    throw std::logic_error("Link without valid path");
}

size_t Link::getSize() {
    if (_resolved_target) {
        return _resolved_target->getSize();
    }
    return 0; // Unresolved links have no size
}

const std::filesystem::path& Link::getTarget() const {
    return _target_name; 
}

bool Link::setResolveTarget(std::unique_ptr<FileObject> t) {
    if(!t) return false;
    _resolved_target = std::move(t);
    _target_name = _resolved_target->getName();
    return true;
}

FileObject* Link::getResolvedTarget() const {
    return _resolved_target.get();
}