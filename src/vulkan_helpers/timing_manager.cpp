#include "timing_manager.h"
#include "vk_context.h"

#include <chrono>

FTimingManager* TimingManager = nullptr;

FTimingManager* GetTimingManager()
{
    if (TimingManager == nullptr)
    {
        TimingManager = new FTimingManager();
    }

    return TimingManager;
}

void FreeTimingManager()
{
    if (TimingManager != nullptr)
    {
        delete TimingManager;
        TimingManager = nullptr;
    }
}

FTimingManager::FTimingManager()
{
    Time = std::chrono::high_resolution_clock::now();
    PreviousTime = Time;
}

FTimingManager::~FTimingManager()
{
    for (const auto& Entry : NameToQueryPool)
    {
        vkDestroyQueryPool(VK_CONTEXT()->LogicalDevice, Entry.second, nullptr);
    }

    NameToQueryPool.clear();
	TimingSizes.clear();
}

void FTimingManager::RegisterTiming(const std::string& TimingName, uint32_t SizeXIn, uint32_t SizeYIn)
{
    if (NameToQueryPool.find(TimingName) != NameToQueryPool.end())
    {
        return;
    }

    NameToQueryPool[TimingName] = VK_NULL_HANDLE;

    VkQueryPoolCreateInfo QueryPoolCreateInfo{};
    QueryPoolCreateInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    QueryPoolCreateInfo.queryCount = SizeXIn * SizeYIn * 2;
    QueryPoolCreateInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;

    if (vkCreateQueryPool(VK_CONTEXT()->LogicalDevice, &QueryPoolCreateInfo, nullptr, &NameToQueryPool[TimingName]) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create query pool!");
    }

	TimingSizes[TimingName] = {SizeXIn, SizeYIn};
}

float FTimingManager::GetDeltaTime(const std::string& TimingName, uint32_t Y)
{
    std::vector<uint64_t> TimeStamps(TimingSizes[TimingName].first * 2);
    vkGetQueryPoolResults(VK_CONTEXT()->LogicalDevice, NameToQueryPool[TimingName], Y * 2 * TimingSizes[TimingName].first, 2 * TimingSizes[TimingName].first, sizeof(uint64_t) * TimeStamps.size(), TimeStamps.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

	uint32_t DeltaUint = 0u;

	for (int i = 0; i < TimingSizes[TimingName].first; ++i)
	{
		DeltaUint += (TimeStamps[1] - TimeStamps[0]);
	}

    float Delta = float(DeltaUint * VK_CONTEXT()->TimestampPeriod) / 1000000000.f;

    return Delta;
}

void FTimingManager::TimestampReset(const std::string& TimingName, VkCommandBuffer CommandBuffer, uint32_t Y)
{
	vkCmdResetQueryPool(CommandBuffer, NameToQueryPool[TimingName], Y * 2 * TimingSizes[TimingName].first, 2 * TimingSizes[TimingName].first);
}

void FTimingManager::TimestampStart(const std::string& TimingName, VkCommandBuffer CommandBuffer, uint32_t X, uint32_t Y)
{
    vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, NameToQueryPool[TimingName], 2 * (Y * TimingSizes[TimingName].first + X));
}

void FTimingManager::TimestampEnd(const std::string& TimingName, VkCommandBuffer CommandBuffer, uint32_t X, uint32_t Y)
{
    vkCmdWriteTimestamp(CommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, NameToQueryPool[TimingName], 2 * (Y * TimingSizes[TimingName].first + X) + 1);
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

void FTimingManager::GetAllTimings(std::vector<std::string>& Names, std::vector<float>& Timings, float& FrameTime, uint32_t FrameIndex)
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
