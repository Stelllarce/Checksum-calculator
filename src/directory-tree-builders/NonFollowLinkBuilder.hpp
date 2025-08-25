#pragma once
#include "BaseBuilder.hpp"

class NonFollowLinkBuilder : public BaseBuilder {
public:
    Directory* buildLink(const std::filesystem::path& name, const std::filesystem::path& target) override;
}; 