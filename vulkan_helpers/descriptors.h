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
};

class FDescriptorSetLayout
{
public:
    FDescriptorSetLayout() = default;

    void AddDescriptorLayout(const std::string& DescriptorLayoutName, const FDescriptor& Descriptor);
    VkDescriptorSetLayout CreateDescriptorSetLayout(VkDevice LogicalDevice);
    void Reset();

private:
    std::map<std::string, FDescriptor> Content;
};