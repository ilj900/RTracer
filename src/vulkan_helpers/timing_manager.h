#pragma once

#include "vulkan/vulkan.h"

#include <string>
#include <unordered_map>

class TimingManager
{
public:
    TimingManager();

    void RegisterTiming(const std::string& TimingName);
    float GetTiming(const std::string& TimingName);
    void NewTime();
    float GetDeltaTime();

private:
    std::unordered_map<std::string, VkQueryPool> NameToQueryPool;
    std::unordered_map<std::string, float, float> TimingHistory;
};