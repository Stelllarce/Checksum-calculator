#include "BaseBuilder.hpp"

BaseBuilder::BaseBuilder() {
    _root = std::make_unique<Directory>("(virtual_root)", nullptr);
    _build_stack.push_back(_root.get());
}

void BaseBuilder::startBuildDirectory(const std::filesystem::path& name) {
    Directory* dir_ptr = _build_stack.back()->createSubdirectory(name);
    _build_stack.push_back(dir_ptr);
}

void BaseBuilder::endBuildDirectory() {
    if (_build_stack.size() > 1) {
        _build_stack.pop_back();
    }
}

void BaseBuilder::buildFile(const std::filesystem::path& name) {
    _build_stack.back()->createFile(name);
}

// std::unique_ptr<Directory> BaseBuilder::getTree() {
//     return std::move(_root);
// }

Directory* BaseBuilder::getTree() const {
    return _root.get();
}