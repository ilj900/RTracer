#include "timing_manager.h"
#include "vk_context.h"

#include <chrono>

FTimingManager::FTimingManager(VkDevice LogicalDeviceIn) : LogicalDevice(LogicalDeviceIn)
{
    Time = std::chrono::high_resolution_clock::now();
    PreviousTime = Time;
}

FTimingManager::~FTimingManager()
{
    for (auto Entry : NameToQueryPool)
    {
        vkDestroyQueryPool(LogicalDevice, Entry.second, nullptr);
    }

    NameToQueryPool.clear();
}

void FTimingManager::RegisterTiming(const std::string& TimingName, int NumberOfSimultaneousSubmitsIn)
{
    if (NameToQueryPool.find(TimingName) != NameToQueryPool.end())
    {
        return;
    }
    
    NameToQueryPool[TimingName] = VK_NULL_HANDLE;
    TimingHistory[TimingName] = std::vector<float>(NumberOfSimultaneousSubmitsIn * 2);

    VkQueryPoolCreateInfo QueryPoolCreateInfo{};
    QueryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    QueryPoolCreateInfo.queryCount = NumberOfSimultaneousSubmitsIn * 2;
    QueryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;

    if (vkCreateQueryPool(LogicalDevice, &QueryPoolCreateInfo, nullptr, &NameToQueryPool[TimingName]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create query pool!");
    }
}

float FTimingManager::GetDeltaTime(const std::string& TimingName, int FrameIndex)
{
    std::vector<uint64_t> TimeStamps(2);
    vkGetQueryPoolResults(LogicalDevice, NameToQueryPool[TimingName], FrameIndex * 2, 2, sizeof(uint64_t) * 2, TimeStamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    float Delta = float((TimeStamps[1] - TimeStamps[0]) * GetContext().TimestampPeriod) / 1000000000.f;
    return Delta;
}

void FTimingManager::TimestampStart(const std::string& TimingName, VkCommandBuffer CommandBuffer, int FrameIndex)
{
    vkCmdResetQueryPool(CommandBuffer, NameToQueryPool[TimingName], FrameIndex * 2, 2);
    vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, NameToQueryPool[TimingName], FrameIndex * 2);
}

void FTimingManager::TimestampEnd(const std::string& TimingName, VkCommandBuffer CommandBuffer, int FrameIndex)
{
    vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, NameToQueryPool[TimingName], FrameIndex * 2 + 1);
}

void FTimingManager::NewTime()
{
    PreviousTime = Time;
    Time = std::chrono::high_resolution_clock::now();
}

float FTimingManager::GetDeltaTime()
{
    return std::chrono::duration<float, std::chrono::milliseconds ::period>(Time - PreviousTime).count();
}

void FTimingManager::GetAllTimings(std::vector<std::string>& Names, std::vector<float>& Timings, float& FrameTime, int FrameIndex)
{
    Names.clear();
    Timings.clear();

    FrameTime = GetDeltaTime();

    for (auto& Task : NameToQueryPool)
    {
        Names.push_back(Task.first);
        Timings.push_back(GetDeltaTime(Task.first, FrameIndex));
    }
}
