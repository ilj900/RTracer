#include "descriptors.h"

#include <stdexcept>

bool operator==(const FDescriptor& A, const FDescriptor& B)
{
    return (A.BindingIndex == B.BindingIndex);
}

FDescriptorSetLayout::FDescriptorSetLayout()
{
}

FDescriptorSetManager::FDescriptorSetManager(VkDevice LogicalDevice):
        LogicalDevice(LogicalDevice)
{
};

void FDescriptorSetManager::AddDescriptorLayout(const std::string& DescriptorSetLayoutName, uint32_t DescriptorSetLayoutIndex, const std::string& DescriptorLayoutName, const FDescriptor& Descriptor)
{
    auto Iterator = DescriptorSetLayouts.find(DescriptorSetLayoutName);

    if (Iterator == DescriptorSetLayouts.end())
    {
        DescriptorSetLayouts[DescriptorSetLayoutName] = {FDescriptorSetLayout(), DescriptorSetLayoutIndex};
        DescriptorSetLayouts[DescriptorSetLayoutName].first.Descriptors[DescriptorLayoutName] = Descriptor;
    }
    else
    {
        auto SetIndex = Iterator->second.second;

        if (SetIndex != DescriptorSetLayoutIndex)
        {
            throw std::runtime_error("For given descriptor set layout name: \"" + DescriptorSetLayoutName + "\" provided index: \"" + std::to_string(DescriptorSetLayoutIndex) +
                                     "\" doesn't match the one already assigned to it: \"" + std::to_string(SetIndex) + "\".\n");
        }

        auto& Descriptors = Iterator->second.first.Descriptors;

        auto InnerIterator = Descriptors.find(DescriptorLayoutName);

        if (InnerIterator == Descriptors.end())
        {
            Descriptors[DescriptorLayoutName] = Descriptor;
        }
        else
        {
            throw std::runtime_error("You are trying to rewrite already existing descriptor layout: \"" + InnerIterator->first + "\" with a new one: \""
            + DescriptorLayoutName + "\"\n");
        }
    }
}

void FDescriptorSetManager::CreateDescriptorSetLayouts()
{
    for (auto Entry : DescriptorSetLayouts)
    {
        auto Name = Entry.first;
        auto Layouts = Entry.second.first;

        std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;

        for (auto Layout : Layouts.Descriptors)
        {
            VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding;
            DescriptorSetLayoutBinding.binding = Layout.second.BindingIndex;
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

        VkDescriptorSetLayouts[Name] = DescriptorSetLayout;
    }
}

VkDescriptorSetLayout FDescriptorSetManager::GetVkDescriptorSetLayout(const std::string& DescriptorSetLayoutName)
{
    return VkDescriptorSetLayouts[DescriptorSetLayoutName];
}

void FDescriptorSetManager::DestroyDescriptorSetLayout(const std::string& DescriptorSetLayoutName)
{
    vkDestroyDescriptorSetLayout(LogicalDevice, VkDescriptorSetLayouts[DescriptorSetLayoutName], nullptr);
}


void FDescriptorSetManager::AddDescriptorSet(const std::string& DescriptorSetLayoutName, uint32_t Count)
{
    Sets[DescriptorSetLayoutName] += Count;
}

void FDescriptorSetManager::ReserveDescriptorPool()
{
    /// Count all types and how much of it we need
    std::map<VkDescriptorType, uint32_t> TypeCount{};
    for (auto Set : Sets)
    {
        auto Count = Set.second;
        auto SetName = Set.first;
        auto DescriptorSetLayout = DescriptorSetLayouts[SetName].first.Descriptors;
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
}

//VkDescriptorSet FDescriptorSetManager::CreateDescriptorSets(const std::string& DescriptorSetLayoutName)
//{
//    if (VkDescriptorSetLayouts.find(DescriptorSetLayoutName) == VkDescriptorSetLayouts.end())
//    {
//        throw std::runtime_error("Descriptor set layout: \"" + DescriptorSetLayoutName + "\" not registered.\n");
//    }
//
//    VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo{};
//    DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;
//    DescriptorSetAllocateInfo.descriptorSetCount = 1;
//    DescriptorSetAllocateInfo.pSetLayouts = &VkDescriptorSetLayouts[DescriptorSetLayoutName];
//
//    VkDescriptorSet DescriptorSet;
//
//    if (vkAllocateDescriptorSets(LogicalDevice, &DescriptorSetAllocateInfo, &DescriptorSet) != VK_SUCCESS)
//    {
//        throw std::runtime_error("Failed to allocate descriptor sets!");
//    }
//
//    return DescriptorSet;
//}

void FDescriptorSetManager::FreeDescriptorPool()
{
    vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

void FDescriptorSetManager::CreateAllDescriptorSets()
{
    for (auto Set : Sets)
    {
        auto Name = Set.first;
        auto Count = Set.second;
        auto DescriptorSetLayout = VkDescriptorSetLayouts[Name];

        std::vector<VkDescriptorSetLayout> DescriptorSetLayoutsData(Count, DescriptorSetLayout);

        VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo{};
        DescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        DescriptorSetAllocateInfo.descriptorPool = DescriptorPool;
        DescriptorSetAllocateInfo.descriptorSetCount = static_cast<uint32_t>(DescriptorSetLayoutsData.size());
        DescriptorSetAllocateInfo.pSetLayouts = DescriptorSetLayoutsData.data();

        DescriptorSets[Name] = std::vector<VkDescriptorSet>(Count);

        if (vkAllocateDescriptorSets(LogicalDevice, &DescriptorSetAllocateInfo, DescriptorSets[Name].data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
    }
}

void FDescriptorSetManager::UpdateDescriptorSetInfo(const std::string& DescriptorSetName, const std::string& DescriptorName, uint32_t Index, VkDescriptorBufferInfo& BufferInfo)
{
    auto& Layout = DescriptorSetLayouts[DescriptorSetName];
    auto& DescriptorBinding = Layout.first.Descriptors[DescriptorName];

    VkWriteDescriptorSet DescriptorWrites{};
    DescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorWrites.dstSet = GetSet(DescriptorSetName, Index);
    DescriptorWrites.dstBinding = DescriptorBinding.BindingIndex;
    DescriptorWrites.dstArrayElement = 0;
    DescriptorWrites.descriptorType = DescriptorBinding.Type;
    DescriptorWrites.descriptorCount = 1;
    DescriptorWrites.pBufferInfo = &BufferInfo;

    vkUpdateDescriptorSets(LogicalDevice, 1, &DescriptorWrites, 0, nullptr);
}

void FDescriptorSetManager::UpdateDescriptorSetInfo(const std::string& DescriptorSetName, const std::string& DescriptorName, uint32_t Index, VkDescriptorImageInfo& ImageInfo)
{
    auto& Layout = DescriptorSetLayouts[DescriptorSetName];
    auto& DescriptorBinding = Layout.first.Descriptors[DescriptorName];

    VkWriteDescriptorSet DescriptorWrites{};
    DescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorWrites.dstSet = GetSet(DescriptorSetName, Index);
    DescriptorWrites.dstBinding = DescriptorBinding.BindingIndex;
    DescriptorWrites.dstArrayElement = 0;
    DescriptorWrites.descriptorType = DescriptorBinding.Type;
    DescriptorWrites.descriptorCount = 1;
    DescriptorWrites.pImageInfo = &ImageInfo;

    vkUpdateDescriptorSets(LogicalDevice, 1, &DescriptorWrites, 0, nullptr);
}

VkDescriptorSet& FDescriptorSetManager::GetSet(const std::string& Name, uint32_t Index)
{
    if (DescriptorSets.find(Name) == DescriptorSets.end())
    {
        throw std::runtime_error("Descriptor set: \"" + Name + "\" not registered.\n");
    }
    if (DescriptorSets[Name].size() < Index)
    {
        throw std::runtime_error("Descriptor set: \"" + Name + "\" has only " + std::to_string(DescriptorSets[Name].size())
                                 + " sets allocated. You requested for " + std::to_string(Index) + "\n");
    }
    return DescriptorSets[Name][Index];
}
