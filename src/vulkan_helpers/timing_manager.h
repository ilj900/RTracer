#pragma once

#include "vulkan/vulkan.h"

#include <chrono>
#include <string>
#include <unordered_map>

class FTimingManager
{
public:
    FTimingManager();
    ~FTimingManager();

    void RegisterTiming(const std::string& TimingName, int NumberOfSimultaneousSubmitsIn);
    /// Returns the delta time for a task in ms
    float GetDeltaTime(const std::string& TimingName, int FrameIndex);
    void TimestampStart(const std::string& TimingName, VkCommandBuffer CommandBuffer, int FrameIndex);
    void TimestampEnd(const std::string& TimingName, VkCommandBuffer CommandBuffer, int FrameIndex);
    void NewTime();
    float GetDeltaTime();
    void GetAllTimings(std::vector<std::string>& Names, std::vector<float>& Timings, float& FrameTime, int FrameIndex);

private:
    std::chrono::time_point<std::chrono::steady_clock> Time;
    std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
    std::unordered_map<std::string, VkQueryPool> NameToQueryPool;
    std::unordered_map<std::string, std::vector<float>> TimingHistory;
};

FTimingManager* GetTimingManager();
void FreeTimingManager();

#define TIMING_MANAGER() GetTimingManager()
#define FREE_TIMING_MANAGER() FreeTimingManager()