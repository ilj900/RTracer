#pragma once

#include "vulkan/vulkan.h"

#include <map>
#include <string>
#include <vector>

struct FDescriptor
{
    VkDescriptorType Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkShaderStageFlags StageFlags = VK_SHADER_STAGE_VERTEX_BIT;
};

struct FDescriptorSetLayout
{
    FDescriptorSetLayout();
    std::map<uint32_t , FDescriptor> Descriptors;
};

class FDescriptorSetManager
{
public:
    FDescriptorSetManager(VkDevice LogicalDevice);

    /// Layout business
    void AddDescriptorLayout(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, const FDescriptor& Descriptor);
    void CreateDescriptorSetLayouts();
    VkDescriptorSetLayout GetVkDescriptorSetLayout(uint32_t DescriptorSetLayoutIndex);
    void DestroyDescriptorSetLayout(uint32_t DescriptorSetLayoutIndex);

    /// Descriptor set part
    void AddDescriptorSet(uint32_t DescriptorSetLayoutIndex, uint32_t Count);
    void CreateAllDescriptorSets();
    void UpdateDescriptorSetInfo(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorBufferInfo& BufferInfo);
    void UpdateDescriptorSetInfo(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorImageInfo& ImageInfo);
    VkDescriptorSet& GetSet(uint32_t SetIndex, uint32_t Index);

    /// Pool
    void ReserveDescriptorPool();
    void FreeDescriptorPool();
    void Reset();

private:
    VkDevice LogicalDevice = VK_NULL_HANDLE;

    VkDescriptorPool DescriptorPool;

    /// Key (uint32_t) - index of the descriptor set layout
    /// Value (pair) - first: index of a descriptor set layout, second: that descriptor set layout
    std::map<uint32_t , std::map<uint32_t , FDescriptor>> DescriptorSetLayouts;

    std::map<uint32_t, VkDescriptorSetLayout> VkDescriptorSetLayouts;

    /// Key (uinnt32_t) - index of the descriptor set layout
    /// Value (uint32_t) - number of descriptor sets
    std::map<uint32_t, uint32_t> Sets;

    /// Key (string) - name of the descriptor set
    /// Value - vector of descriptor sets
    std::map<uint32_t, std::vector<VkDescriptorSet>> DescriptorSets;
};