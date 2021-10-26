#include "descriptors.h"

#include <stdexcept>

void FDescriptorSetLayout::AddDescriptorLayout(const std::string& DescriptorLayoutName, const FDescriptor& Descriptor)
{
    Content[DescriptorLayoutName] = Descriptor;
}

VkDescriptorSetLayout FDescriptorSetLayout::CreateDescriptorSetLayout(VkDevice LogicalDevice)
{
    std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;

    for (auto DescriptorLayout : Content)
    {
        VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding;
        DescriptorSetLayoutBinding.binding = DescriptorLayout.second.BindingIndex;
        DescriptorSetLayoutBinding.descriptorType = DescriptorLayout.second.Type;
        DescriptorSetLayoutBinding.descriptorCount = 1;
        DescriptorSetLayoutBinding.stageFlags = DescriptorLayout.second.StageFlags;
        DescriptorSetLayoutBinding.pImmutableSamplers = nullptr;

        DescriptorSetLayoutBindings.push_back(DescriptorSetLayoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo{};
    DescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DescriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(DescriptorSetLayoutBindings.size());
    DescriptorSetLayoutCreateInfo.pBindings = DescriptorSetLayoutBindings.data();

    VkDescriptorSetLayout DescriptorSetLayout;

    if (vkCreateDescriptorSetLayout(LogicalDevice, &DescriptorSetLayoutCreateInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    return DescriptorSetLayout;
}

void FDescriptorSetLayout::Reset()
{
    Content.clear();
}