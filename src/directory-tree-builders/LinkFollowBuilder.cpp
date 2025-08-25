#include "LinkFollowBuilder.hpp"
#include "file-system-composite/File.hpp"

LinkFollowBuilder::LinkFollowBuilder(std::unique_ptr<CycleDetector> tracker) : BaseBuilder(), _cycle_tracker(std::move(tracker)) {}

Directory* LinkFollowBuilder::buildLink(const std::filesystem::path& name, const std::filesystem::path& target) {
    if (_cycle_tracker->check(target)) {
        std::cerr << "Circular dependancy detected" << '\n';
        return nullptr;
    }

    try {
        auto link = std::make_unique<Link>(name, target, _build_stack.back());
        if (std::filesystem::is_directory(target)) {
            auto dir_p = std::make_unique<Directory>(name, link.get());
            Directory* dir_internal = dir_p.get();
            link->setResolveTarget(std::move(dir_p));
            
            if (!_build_stack.back()->add(std::move(link))) {
                std::cerr << "Error while adding link to composite" << '\n';
                return nullptr;
            }
            
            _build_stack.push_back(dir_internal);
            return dir_internal;
        } else if (std::filesystem::is_regular_file(target)) {
            auto file_p = std::make_unique<File>(name, link.get());
            link->setResolveTarget(std::move(file_p));
            
            if (!_build_stack.back()->add(std::move(link))) {
                std::cerr << "Error while adding link to composite" << '\n';
                return nullptr;
            }
            
            return nullptr;
        }
    }
    catch(...) {
        std::cerr << "Exception thrown while building link:\n" << '\n';
        return nullptr;
    }
    
    return nullptr;
}
