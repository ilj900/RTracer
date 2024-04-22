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

    void RegisterTiming(const std::string& TimingName, uint32_t SizeXIn, uint32_t SizeYIn);
    /// Returns the delta time for a task in ms
    float GetDeltaTime(const std::string& TimingName, uint32_t Y);
	void TimestampReset(const std::string& TimingName, VkCommandBuffer CommandBuffer, uint32_t Y);
    void TimestampStart(const std::string& TimingName, VkCommandBuffer CommandBuffer, uint32_t X, uint32_t Y);
    void TimestampEnd(const std::string& TimingName, VkCommandBuffer CommandBuffer, uint32_t X, uint32_t Y);
    void NewTime();
    float GetDeltaTime();
    void GetAllTimings(std::vector<std::string>& Names, std::vector<float>& Timings, float& FrameTime, uint32_t FrameIndex);

private:
    std::chrono::time_point<std::chrono::steady_clock> Time;
    std::chrono::time_point<std::chrono::steady_clock> PreviousTime;
    std::unordered_map<std::string, VkQueryPool> NameToQueryPool;
	std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> TimingSizes;
};

FTimingManager* GetTimingManager();
void FreeTimingManager();

#define TIMING_MANAGER() GetTimingManager()
#define FREE_TIMING_MANAGER() FreeTimingManager()