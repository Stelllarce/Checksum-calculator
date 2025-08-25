#include "NonFollowLinkBuilder.hpp"
#include "file-system-composite/Link.hpp"
#include <iostream>

Directory* NonFollowLinkBuilder::buildLink(const std::filesystem::path& name, const std::filesystem::path& target) {
    auto link = std::make_unique<Link>(name, target, _build_stack.back());
    if (!_build_stack.back()->add(std::move(link))) {
        std::cerr << "Error while adding link to composite" << '\n';
    }
    return nullptr;
}
