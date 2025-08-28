#include "DirectoryConstructor.hpp"
#include <iostream>

DirectoryConstructor::DirectoryConstructor(DirectoryStructureBuilder& builder) : _builder(builder) {
    // if (!_builder) {
    //     throw std::invalid_argument("Builder provided to DirectoryConstructor cannot be null.");
    // }
}

void DirectoryConstructor::construct(std::initializer_list<std::filesystem::path> root_paths) {
    for (const auto& root_path : root_paths) {
        try {
            if (!std::filesystem::exists(root_path)) {
                std::cerr << "Warning: Path does not exist, skipping: " << root_path;
                continue;
            }

            if (std::filesystem::is_directory(root_path)) {
                _builder.startBuildDirectory(root_path);
                traverse(root_path);
                _builder.endBuildDirectory();
            }
            
            if (std::filesystem::is_symlink(root_path)) {
                auto target = std::filesystem::read_symlink(root_path);
                Directory* traversal_target = _builder.buildLink(root_path.filename(), target);
                if (traversal_target) {
                    traverse(traversal_target->getPath().string());
                    _builder.endBuildDirectory();
                }
                continue;
            }

            if (std::filesystem::is_regular_file(root_path)) {
                // For single files, create a parent directory to contain the file
                auto parent_path = root_path.parent_path();
                if (parent_path.empty()) {
                    parent_path = ".";
                }
                _builder.startBuildDirectory(parent_path);
                _builder.buildFile(root_path.filename());
                _builder.endBuildDirectory();
                continue;
            }

        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "Error processing root path: " << e.path1()
                      << ". Reason: " << e.what() << '\n';
        }
    }
}

void DirectoryConstructor::traverse(const std::filesystem::path& currentPath) {

    try {
        for (const auto& entry : std::filesystem::directory_iterator(currentPath)) {
            
            if (entry.is_symlink()) {
                try {
                    auto target = std::filesystem::read_symlink(entry.path());
                    auto traversal_target = _builder.buildLink(entry.path().filename(), target);

                    if (traversal_target) {
                        traverse(traversal_target->getPath().string());
                        _builder.endBuildDirectory();
                    }
                } catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << "Warning: Could not process symlink: " << e.path1()
                              << ". Reason: " << e.what() << '\n';
                }
            
            } else if (entry.is_directory()) {
                _builder.startBuildDirectory(entry.path().filename());
                traverse(entry.path());
                _builder.endBuildDirectory();
            
            } else if (entry.is_regular_file()) {
                _builder.buildFile(entry.path().filename());
            }

        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error accessing directory: " << e.path1()
                  << ". Reason: " << e.what() << '\n';
    }
}