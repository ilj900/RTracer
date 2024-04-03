#include "vk_context.h"
#include "vk_debug.h"
#include "vk_functions.h"
#include "common_defines.h"
#include "common_structures.h"
#include "utils.h"

#include "vk_shader_compiler.h"

#include "task_material_sort_compute_prefix_sums_up_sweep.h"

#include <random>

FComputePrefixSumsUpSweepTask::FComputePrefixSumsUpSweepTask(uint32_t WidthIn, uint32_t HeightIn, FVulkanContext* Context, int NumberOfSimultaneousSubmits, VkDevice LogicalDevice) :
        FExecutableTask(WidthIn, HeightIn, Context, NumberOfSimultaneousSubmits, LogicalDevice)
{
    Name = "Material sort compute prefix sums up-sweep pipeline";

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    DescriptorSetManager->AddDescriptorLayout(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_BUFFER_A,
                                              {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,  VK_SHADER_STAGE_COMPUTE_BIT});

    std::random_device Dev;
    std::mt19937 RNG(Dev());
    RNG.seed(0);
    std::uniform_int_distribution<std::mt19937::result_type> Dist32(0,31);

    std::vector<uint32_t> TestData(TOTAL_MATERIALS * 8192, 0);
    for (int i = 0; i < TestData.size(); ++i)
    {
        if (i % 8192 < (8100))
        {
            TestData[i] = Dist32(RNG);
        }
        else
        {
            TestData[i] = 0;
        }
    }

    std::vector<uint32_t> TotalSum(TOTAL_MATERIALS, 0);
    for (int i = 0; i < TotalSum.size(); ++i)
    {
        for (int j = 0; j < 8192; ++j)
        {
            TotalSum[i] += TestData[i * 8192 + j];
        }
    }

    uint32_t GroupCount = CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE);
    uint32_t TotalGroupCount = 2 << Log2(GroupCount);
    uint32_t BufferSize = TotalGroupCount * TOTAL_MATERIALS * sizeof(uint32_t);
    FBuffer BufferA = Context->ResourceAllocator->CreateBuffer(BufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT| VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, "BufferA");
    Context->ResourceAllocator->RegisterBuffer(BufferA, "BufferA");

    Context->ResourceAllocator->LoadDataToBuffer(BufferA, {TestData.size() * sizeof(uint32_t)}, {0}, {TestData.data()});

    VkPushConstantRange PushConstantRange{VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums)};
    DescriptorSetManager->CreateDescriptorSetLayout({PushConstantRange}, Name);

    CreateSyncObjects();
}

FComputePrefixSumsUpSweepTask::~FComputePrefixSumsUpSweepTask()
{
    FreeSyncObjects();
    Context->ResourceAllocator->UnregisterAndDestroyBuffer("BufferA");
}

void FComputePrefixSumsUpSweepTask::Init()
{
    Context->TimingManager->RegisterTiming(Name, NumberOfSimultaneousSubmits);

    auto& DescriptorSetManager = Context->DescriptorSetManager;

    auto MaterialCountShader = FShader("../../../src/shaders/material_sort_compute_prefix_sums_up_sweep.comp");

    PipelineLayout = DescriptorSetManager->GetPipelineLayout(Name);
    Pipeline = Context->CreateComputePipeline(MaterialCountShader(), PipelineLayout);

    /// Reserve descriptor sets that will be bound once per frame and once for each renderable objects
    DescriptorSetManager->ReserveDescriptorSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, NumberOfSimultaneousSubmits);

    DescriptorSetManager->ReserveDescriptorPool(Name);

    DescriptorSetManager->AllocateAllDescriptorSets(Name);
};

void FComputePrefixSumsUpSweepTask::UpdateDescriptorSets()
{
    for (size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        UpdateDescriptorSet(MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_BUFFER_A, i, Context->ResourceAllocator->GetBuffer("BufferA"));
    }
};

void FComputePrefixSumsUpSweepTask::RecordCommands()
{
    CommandBuffers.resize(NumberOfSimultaneousSubmits);

    for (std::size_t i = 0; i < NumberOfSimultaneousSubmits; ++i)
    {
        CommandBuffers[i] = GetContext().CommandBufferManager->RecordCommand([&, this](VkCommandBuffer CommandBuffer)
        {
            Context->TimingManager->TimestampStart(Name, CommandBuffer, i);

            vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline);
            auto ComputeDescriptorSet = Context->DescriptorSetManager->GetSet(Name, MATERIAL_SORT_COMPUTE_PREFIX_SUMS_UP_SWEEP_LAYOUT_INDEX, i);
            vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                    0, 1, &ComputeDescriptorSet, 0, nullptr);

            uint32_t GroupCount = CalculateGroupCount(Width * Height, BASIC_CHUNK_SIZE);
            uint32_t DMax = Log2(GroupCount);
            uint32_t TotalGroupCount = 2 << Log2(GroupCount);
            FPushConstantsPrefixSums PushConstantsPrefixSums = {0, GroupCount};

            for (int D = 0; D <= DMax; ++D)
            {
                PushConstantsPrefixSums.D = D;
                vkCmdPushConstants(CommandBuffer, Context->DescriptorSetManager->GetPipelineLayout(Name),
                                   VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(FPushConstantsPrefixSums), &PushConstantsPrefixSums);
                TotalGroupCount /= 2;
                vkCmdDispatch(CommandBuffer, TotalGroupCount, 1, 1);

                VkMemoryBarrier MemoryBarrier = {};
                MemoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                MemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
                MemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &MemoryBarrier, 0, nullptr, 0, nullptr);
            }

            Context->TimingManager->TimestampEnd(Name, CommandBuffer, i);
        });

        V::SetName(LogicalDevice, CommandBuffers[i], "V::MaterialSort_Compute_Prefix_Sums_Up_Sweep_Command_Buffer");
    }
};

void FComputePrefixSumsUpSweepTask::Cleanup()
{
    Inputs.clear();
    Outputs.clear();

    for (auto& CommandBuffer : CommandBuffers)
    {
        Context->CommandBufferManager->FreeCommandBuffer(CommandBuffer);
    }

    Context->DescriptorSetManager->DestroyPipelineLayout(Name);
    vkDestroyPipeline(LogicalDevice, Pipeline, nullptr);

    Context->DescriptorSetManager->Reset(Name);
};

VkSemaphore FComputePrefixSumsUpSweepTask::Submit(VkQueue Queue, VkSemaphore WaitSemaphore, VkFence WaitFence, VkFence SignalFence, int IterationIndex)
{
    return FExecutableTask::Submit(Queue, WaitSemaphore, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, WaitFence, SignalFence, IterationIndex);
};
