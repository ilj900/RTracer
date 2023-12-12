#pragma once

#include "vulkan/vulkan.h"

#include <map>
#include <string>
#include <vector>

struct FDescriptor
{
    VkDescriptorType Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkShaderStageFlags StageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    int DescriptorCount = 1;
};

struct FDescriptorSetLayout
{
    std::map<uint32_t , FDescriptor> DescriptorSets;
};

struct FPipelineDescriptorSetLayout
{
    void AddDescriptorLayout(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, const FDescriptor& Descriptor);
    void CreateDescriptorSetLayout(VkDevice LogicalDevice, const std::vector<VkPushConstantRange>& PushConstantRangeVector, const std::string& PipelineDebugName);
    VkDescriptorSetLayout GetVkDescriptorSetLayout(uint32_t DescriptorSetLayoutIndex);
    VkPipelineLayout GetPipelineLayout();

    void ReserveDescriptorSet(uint32_t DescriptorSetLayoutIndex, uint32_t Count);
    void ReserveDescriptorPool(VkDevice LogicalDevice);
    void AllocateAllDescriptorSets(VkDevice LogicalDevice);

    void UpdateDescriptorSetInfo(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorBufferInfo* BufferInfo);
    void UpdateDescriptorSetInfo(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorImageInfo* ImageInfo);
    void UpdateDescriptorSetInfo(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkAccelerationStructureKHR* ASInfo);
    VkDescriptorSet GetSet(uint32_t SetIndex, uint32_t Index);

    void DestroyDescriptorSetLayout(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex);
    void DestroyPipelineLayout(VkDevice LogicalDevice);

    void CleanAll(VkDevice LogicalDevice);

    void FreeDescriptorPool(VkDevice LogicalDevice);
    void Reset(VkDevice LogicalDevice);

    using SetIndex = uint32_t;

    std::map<SetIndex, FDescriptorSetLayout> PipelineDescriptorSets;
    std::map<SetIndex, VkDescriptorSetLayout> VkDescriptorSetLayouts;
    std::map<SetIndex, uint32_t> Sets;
    std::map<SetIndex, std::vector<VkDescriptorSet>> VkDescriptorSets;
    VkPipelineLayout PipelineLayout;
    VkDescriptorPool DescriptorPool;
};

class FDescriptorSetManager
{
public:
    FDescriptorSetManager(VkDevice LogicalDevice);
    ~FDescriptorSetManager();

    void AddDescriptorLayout(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, const FDescriptor& Descriptor);
    void CreateDescriptorSetLayout(const std::vector<VkPushConstantRange>& PushConstantRangeVector, const std::string& PipelineName);
    VkDescriptorSetLayout GetVkDescriptorSetLayout(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex);
    VkPipelineLayout GetPipelineLayout(const std::string& PipelineName);

    void ReserveDescriptorSet(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t Count);
    void ReserveDescriptorPool(const std::string& PipelineName);
    void AllocateAllDescriptorSets(const std::string& PipelineName);

    void UpdateDescriptorSetInfo(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorBufferInfo* BufferInfo);
    void UpdateDescriptorSetInfo(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorImageInfo* ImageInfo);
    void UpdateDescriptorSetInfo(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkAccelerationStructureKHR* ASInfo);
    VkDescriptorSet GetSet(const std::string& PipelineName, uint32_t SetIndex, uint32_t Index);

    void DestroyDescriptorSetLayout(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex);
    void DestroyPipelineLayout(const std::string& PipelineName);

    void FreeDescriptorPool(const std::string& PipelineName);
    void Reset(const std::string& PipelineName);
private:
    VkDevice LogicalDevice = VK_NULL_HANDLE;

    /// For every pipeline we have a set that has number of descriptors
    std::map<std::string, FPipelineDescriptorSetLayout> PipelineDescriptorSetLayouts;
};