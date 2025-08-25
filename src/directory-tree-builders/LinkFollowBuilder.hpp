#pragma once
#include <unordered_set>
#include "CycleDetector.hpp"
#include "BaseBuilder.hpp"

class LinkFollowBuilder : public BaseBuilder {
public:
    LinkFollowBuilder(std::unique_ptr<CycleDetector> tracker);
    
    Directory* buildLink(const std::filesystem::path& name, const std::filesystem::path& target) override;

private:
    std::unique_ptr<DetectionStrategy> _cycle_tracker;
};