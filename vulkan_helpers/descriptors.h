#pragma once

#include "vulkan/vulkan.h"

#include <map>
#include <string>
#include <vector>

struct FDescriptor
{
    uint32_t BindingIndex = 0;
    VkDescriptorType Type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkShaderStageFlags StageFlags = VK_SHADER_STAGE_VERTEX_BIT;

/// Operators
    friend bool operator==(const FDescriptor& A, const FDescriptor& B);
};

struct FDescriptorSetLayout
{
    FDescriptorSetLayout();
    std::map<std::string, FDescriptor> Descriptors;
};

class FDescriptorSetManager
{
public:
    FDescriptorSetManager(VkDevice LogicalDevice);

    /// Layout business
    void AddDescriptorLayout(const std::string& DescriptorSetLayoutName, uint32_t DescriptorSetLayoutIndex, const std::string& DescriptorLayoutName, const FDescriptor& Descriptor);
    void CreateDescriptorSetLayouts();
    VkDescriptorSetLayout GetVkDescriptorSetLayout(const std::string& DescriptorSetLayoutName);
    void DestroyDescriptorSetLayout(const std::string& DescriptorSetLayoutName);

    /// Descriptor set part
    void AddDescriptorSet(const std::string& DescriptorSetLayoutName, uint32_t Count);
    void CreateAllDescriptorSets();
    void UpdateDescriptorSetInfo(const std::string& DescriptorSetName, const std::string& DescriptorName, uint32_t Index, VkDescriptorBufferInfo& BufferInfo);
    void UpdateDescriptorSetInfo(const std::string& DescriptorSetName, const std::string& DescriptorName, uint32_t Index, VkDescriptorImageInfo& ImageInfo);
    VkDescriptorSet& GetSet(const std::string& Name, uint32_t Index);

    /// Pool
    void ReserveDescriptorPool();
    void FreeDescriptorPool();

private:
    VkDevice LogicalDevice = VK_NULL_HANDLE;

    VkDescriptorPool DescriptorPool;

    /// Key (string) - name of the descriptor set layout
    /// Value (pair) - first: descriptor set layout, second: index of that descriptor set layout
    std::map<std::string, std::pair<FDescriptorSetLayout, uint32_t>> DescriptorSetLayouts;

    std::map<std::string, VkDescriptorSetLayout> VkDescriptorSetLayouts;

    /// Key (string) - name of the descriptor set layout
    /// Value (uint32_t) - number of descriptor sets
    std::map<std::string, uint32_t> Sets;

    /// Key (string) - name of the descriptor set
    /// Value - vector of descriptor sets
    std::map<std::string, std::vector<VkDescriptorSet>> DescriptorSets;
};