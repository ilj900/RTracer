#include "descriptors.h"

#include "vk_debug.h"

#include <stdexcept>

FDescriptorSetLayout::FDescriptorSetLayout()
{
}

FDescriptorSetManager::FDescriptorSetManager(VkDevice LogicalDevice):
        LogicalDevice(LogicalDevice)
{
};

void FDescriptorSetManager::AddDescriptorLayout(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, const FDescriptor& Descriptor)
{
    auto SetIterator = DescriptorSetLayouts.find(DescriptorSetLayoutIndex);
    if (SetIterator != DescriptorSetLayouts.end())
    {
        auto DescriptorIterator = SetIterator->second.find(DescriptorLayoutIndex);
        if (DescriptorIterator != SetIterator->second.end())
        {
            throw std::runtime_error("You are rewriting existing layout in set: " + std::to_string(DescriptorSetLayoutIndex) + " at index: " + std::to_string(DescriptorLayoutIndex));
        }
    }
    DescriptorSetLayouts[DescriptorSetLayoutIndex][DescriptorLayoutIndex] = Descriptor;
}

void FDescriptorSetManager::CreateDescriptorSetLayouts()
{
    for (auto Entry : DescriptorSetLayouts)
    {
        auto SetIndex = Entry.first;
        auto Layouts = Entry.second;

        std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;

        for (auto Layout : Layouts)
        {
            VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding;
            DescriptorSetLayoutBinding.binding = Layout.first;
            DescriptorSetLayoutBinding.descriptorType = Layout.second.Type;
            DescriptorSetLayoutBinding.descriptorCount = 1;
            DescriptorSetLayoutBinding.stageFlags = Layout.second.StageFlags;
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

        VkDescriptorSetLayouts[SetIndex] = DescriptorSetLayout;
    }
}

VkDescriptorSetLayout FDescriptorSetManager::GetVkDescriptorSetLayout(uint32_t DescriptorSetLayoutIndex)
{
    return VkDescriptorSetLayouts[DescriptorSetLayoutIndex];
}

void FDescriptorSetManager::DestroyDescriptorSetLayout(uint32_t DescriptorSetLayoutIndex)
{
    vkDestroyDescriptorSetLayout(LogicalDevice, VkDescriptorSetLayouts[DescriptorSetLayoutIndex], nullptr);
}


void FDescriptorSetManager::AddDescriptorSet(uint32_t DescriptorSetLayoutIndex, uint32_t Count)
{
    Sets[DescriptorSetLayoutIndex] += Count;
}

void FDescriptorSetManager::ReserveDescriptorPool()
{
    /// Count all types and how much of it we need
    std::map<VkDescriptorType, uint32_t> TypeCount{};
    for (auto Set : Sets)
    {
        auto Count = Set.second;
        auto SetIndex = Set.first;
        auto DescriptorSetLayout = DescriptorSetLayouts[SetIndex];
        for (auto Descriptor : DescriptorSetLayout)
        {
            TypeCount[Descriptor.second.Type] += Count;
        }
    }

    /// Fill it pool sizes
    std::vector<VkDescriptorPoolSize> PoolSizes{};
    uint32_t MaxSets = 0;
    for (auto Type : TypeCount)
    {
        PoolSizes.push_back({Type.first, Type.second});
        MaxSets += Type.second;
    }

    VkDescriptorPoolCreateInfo PoolInfo{};
    PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    PoolInfo.poolSizeCount = static_cast<uint32_t>(PoolSizes.size());
    PoolInfo.pPoolSizes = PoolSizes.data();
    PoolInfo.maxSets = static_cast<uint32_t>(MaxSets);

    if (vkCreateDescriptorPool(LogicalDevice, &PoolInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create descriptor pool!");
    }

    V::SetName(LogicalDevice, DescriptorPool, "V_MainDescriptorPool");
}

void FDescriptorSetManager::FreeDescriptorPool()
{
    vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

void FDescriptorSetManager::Reset()
{
    if (DescriptorPool)
    {
        FreeDescriptorPool();
    }

    Sets.clear();
    DescriptorSets.clear();
}

void FDescriptorSetManager::CreateAllDescriptorSets()
{
    for (auto Set : Sets)
    {
        auto SetIndex = Set.first;
        auto Count = Set.second;
        auto DescriptorSetLayout = VkDescriptorSetLayouts[SetIndex];

        std::vector<VkDescriptorSetLayout> DescriptorSetLayoutsData(Count, DescriptorSetLayout);

        VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo{};
        DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;
        DescriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(DescriptorSetLayoutsData.size());
        DescriptorSetAllocateInfo.pSetLayouts = DescriptorSetLayoutsData.data();

        DescriptorSets[SetIndex] = std::vector<VkDescriptorSet>(Count);

        if (vkAllocateDescriptorSets(LogicalDevice, &DescriptorSetAllocateInfo, DescriptorSets[SetIndex].data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
    }
}

void FDescriptorSetManager::UpdateDescriptorSetInfo(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorBufferInfo& BufferInfo)
{
    auto& DescriptorBinding = DescriptorSetLayouts[DescriptorSetLayoutIndex][DescriptorLayoutIndex];

    VkWriteDescriptorSet DescriptorWrites{};
    DescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorWrites.dstSet = GetSet(DescriptorSetLayoutIndex, Index);
    DescriptorWrites.dstBinding = Index;
    DescriptorWrites.dstArrayElement = 0;
    DescriptorWrites.descriptorType = DescriptorBinding.Type;
    DescriptorWrites.descriptorCount = 1;
    DescriptorWrites.pBufferInfo = &BufferInfo;

    vkUpdateDescriptorSets(LogicalDevice, 1, &DescriptorWrites, 0, nullptr);
}

void FDescriptorSetManager::UpdateDescriptorSetInfo(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorImageInfo& ImageInfo)
{
    auto& DescriptorBinding = DescriptorSetLayouts[DescriptorSetLayoutIndex][DescriptorLayoutIndex];

    VkWriteDescriptorSet DescriptorWrites{};
    DescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorWrites.dstSet = GetSet(DescriptorSetLayoutIndex, Index);
    DescriptorWrites.dstBinding = Index;
    DescriptorWrites.dstArrayElement = 0;
    DescriptorWrites.descriptorType = DescriptorBinding.Type;
    DescriptorWrites.descriptorCount = 1;
    DescriptorWrites.pImageInfo = &ImageInfo;

    vkUpdateDescriptorSets(LogicalDevice, 1, &DescriptorWrites, 0, nullptr);
}

VkDescriptorSet& FDescriptorSetManager::GetSet(uint32_t SetIndex, uint32_t Index)
{
    if (DescriptorSets.find(SetIndex) == DescriptorSets.end())
    {
        throw std::runtime_error("Descriptor set with index: \"" + std::to_string(SetIndex) + "\" not registered.\n");
    }
    if (DescriptorSets[SetIndex].size() < Index)
    {
        throw std::runtime_error("Descriptor set: \"" + std::to_string(SetIndex) + "\" has only " + std::to_string(DescriptorSets[SetIndex].size())
                                 + " sets allocated. You requested for " + std::to_string(Index) + "\n");
    }
    return DescriptorSets[SetIndex][Index];
}
