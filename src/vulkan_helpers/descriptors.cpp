#include "descriptors.h"
#include "vk_context.h"

#include "vk_debug.h"

#include <stdexcept>

void FPipelineDescriptorSetLayout::AddDescriptorLayout(uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, const FDescriptor& Descriptor)
{
    auto SetIterator = PipelineDescriptorSets.find(DescriptorSetLayoutIndex);

    if (SetIterator != PipelineDescriptorSets.end())
    {
        auto DescriptorIterator = SetIterator->second.DescriptorSets.find(DescriptorLayoutIndex);
        if (DescriptorIterator != SetIterator->second.DescriptorSets.end())
        {
            throw std::runtime_error("You are rewriting existing layout in set: " + std::to_string(DescriptorSetLayoutIndex) + " at index: " + std::to_string(DescriptorLayoutIndex));
        }
    }

    PipelineDescriptorSets[DescriptorSetLayoutIndex].DescriptorSets[DescriptorLayoutIndex] = Descriptor;
}

void FPipelineDescriptorSetLayout::CreateDescriptorSetLayout(VkDevice LogicalDevice, const std::vector<VkPushConstantRange>& PushConstantRangeVector, const std::string& PipelineDebugName)
{
    for (const auto& Entry : PipelineDescriptorSets)
    {
        auto SetIndex = Entry.first;
        auto Layouts = Entry.second;

        std::vector<VkDescriptorSetLayoutBinding> DescriptorSetLayoutBindings;

        for (auto Layout : Layouts.DescriptorSets)
        {
            VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding;
            DescriptorSetLayoutBinding.binding = Layout.first;
            DescriptorSetLayoutBinding.descriptorType = Layout.second.Type;
            DescriptorSetLayoutBinding.descriptorCount = Layout.second.DescriptorCount;
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

        V::SetName(LogicalDevice, DescriptorSetLayout, PipelineDebugName);

        VkDescriptorSetLayouts[SetIndex] = DescriptorSetLayout;
    }

    std::vector<VkDescriptorSetLayout> DescriptorSetLayouts;

    DescriptorSetLayouts.reserve(VkDescriptorSetLayouts.size());

    for  (auto Entry : VkDescriptorSetLayouts)
    {
        DescriptorSetLayouts.push_back(Entry.second);
    }

    VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(DescriptorSetLayouts.size());
    PipelineLayoutInfo.pSetLayouts = DescriptorSetLayouts.data();
    PipelineLayoutInfo.pushConstantRangeCount = PushConstantRangeVector.size();
    PipelineLayoutInfo.pPushConstantRanges = (PushConstantRangeVector.empty()) ? nullptr : PushConstantRangeVector.data();

    if (vkCreatePipelineLayout(LogicalDevice, &PipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

VkDescriptorSetLayout FPipelineDescriptorSetLayout::GetVkDescriptorSetLayout(uint32_t DescriptorSetLayoutIndex)
{
    return VkDescriptorSetLayouts[DescriptorSetLayoutIndex];
}

VkPipelineLayout FPipelineDescriptorSetLayout::GetPipelineLayout() const
{
    return PipelineLayout;
}

void FPipelineDescriptorSetLayout::DestroyDescriptorSetLayout(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex)
{
    vkDestroyDescriptorSetLayout(LogicalDevice, VkDescriptorSetLayouts[DescriptorSetLayoutIndex], nullptr);
}

void FPipelineDescriptorSetLayout::DestroyPipelineLayout(VkDevice LogicalDevice) const
{
    vkDestroyPipelineLayout(LogicalDevice, PipelineLayout, nullptr);
}

void FPipelineDescriptorSetLayout::CleanAll(VkDevice LogicalDevice)
{
    for (auto Entry : VkDescriptorSetLayouts)
    {
        vkDestroyDescriptorSetLayout(LogicalDevice, Entry.second, nullptr);
    }

    VkDescriptorSetLayouts.clear();
}

void FPipelineDescriptorSetLayout::ReserveDescriptorSet(uint32_t DescriptorSetLayoutIndex, uint32_t Count)
{
    Sets[DescriptorSetLayoutIndex] += Count;
}

void FPipelineDescriptorSetLayout::ReserveDescriptorPool(VkDevice LogicalDevice)
{
    /// Count all types and how much of it we need
    std::map<VkDescriptorType, uint32_t> TypeCount{};
    for (auto Set : Sets)
    {
        auto Count = Set.second;
        auto SetIndex = Set.first;
        auto DescriptorSetLayout = PipelineDescriptorSets[SetIndex];
        for (auto Descriptor : DescriptorSetLayout.DescriptorSets)
        {
            TypeCount[Descriptor.second.Type] += Count;
        }
    }

    DescriptorPool = VK_CONTEXT()->CreateDescriptorPool(TypeCount, LogicalDevice, "TODO: Give me name!");
}

void FPipelineDescriptorSetLayout::AllocateAllDescriptorSets(VkDevice LogicalDevice)
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

        VkDescriptorSets[SetIndex] = std::vector<VkDescriptorSet>(Count);

        if (vkAllocateDescriptorSets(LogicalDevice, &DescriptorSetAllocateInfo, VkDescriptorSets[SetIndex].data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }

        for (int i = 0; i < VkDescriptorSets[SetIndex].size(); ++i)
        {
            V::SetName(LogicalDevice, VkDescriptorSets[SetIndex][i], "Descriptor_Set_" + std::to_string(SetIndex) + "_Number_" + std::to_string(i));
        }
    }
}

void FPipelineDescriptorSetLayout::UpdateDescriptorSetInfo(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorBufferInfo* BufferInfo)
{
    auto& Descriptor = PipelineDescriptorSets[DescriptorSetLayoutIndex].DescriptorSets[DescriptorLayoutIndex];

    VkWriteDescriptorSet DescriptorWrites{};
    DescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorWrites.dstSet = GetSet(DescriptorSetLayoutIndex, Index);
    DescriptorWrites.dstBinding = DescriptorLayoutIndex;
    DescriptorWrites.dstArrayElement = 0;
    DescriptorWrites.descriptorType = Descriptor.Type;
    DescriptorWrites.descriptorCount = 1;
    DescriptorWrites.pBufferInfo = BufferInfo;

    vkUpdateDescriptorSets(LogicalDevice, 1, &DescriptorWrites, 0, nullptr);
}

void FPipelineDescriptorSetLayout::UpdateDescriptorSetInfo(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorImageInfo* ImageInfo)
{
    auto& Descriptor = PipelineDescriptorSets[DescriptorSetLayoutIndex].DescriptorSets[DescriptorLayoutIndex];

    VkWriteDescriptorSet DescriptorWrites{};
    DescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorWrites.dstSet = GetSet(DescriptorSetLayoutIndex, Index);
    DescriptorWrites.dstBinding = DescriptorLayoutIndex;
    DescriptorWrites.dstArrayElement = 0;
    DescriptorWrites.descriptorType = Descriptor.Type;
    DescriptorWrites.descriptorCount = Descriptor.DescriptorCount;
    DescriptorWrites.pImageInfo = ImageInfo;

    vkUpdateDescriptorSets(LogicalDevice, 1, &DescriptorWrites, 0, nullptr);
}

void FPipelineDescriptorSetLayout::UpdateDescriptorSetInfo(VkDevice LogicalDevice, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkAccelerationStructureKHR* ASInfo)
{
    VkWriteDescriptorSetAccelerationStructureKHR WriteDescriptorSetAccelerationStructureKHR{};
    WriteDescriptorSetAccelerationStructureKHR.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    WriteDescriptorSetAccelerationStructureKHR.pAccelerationStructures = ASInfo;
    WriteDescriptorSetAccelerationStructureKHR.accelerationStructureCount = 1;

    VkWriteDescriptorSet DescriptorWrites{};
    DescriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorWrites.pNext = &WriteDescriptorSetAccelerationStructureKHR;
    DescriptorWrites.dstSet = GetSet(DescriptorSetLayoutIndex, Index);
    DescriptorWrites.dstBinding = DescriptorLayoutIndex;
    DescriptorWrites.dstArrayElement = 0;
    DescriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    DescriptorWrites.descriptorCount = 1;

    vkUpdateDescriptorSets(LogicalDevice, 1, &DescriptorWrites, 0, nullptr);
}

VkDescriptorSet FPipelineDescriptorSetLayout::GetSet(uint32_t SetIndex, uint32_t Index)
{
    if (VkDescriptorSets.find(SetIndex) == VkDescriptorSets.end())
    {
        throw std::runtime_error("Descriptor set with index: \"" + std::to_string(SetIndex) + "\" not registered.\n");
    }
    if (VkDescriptorSets[SetIndex].size() < Index)
    {
        throw std::runtime_error("Descriptor set: \"" + std::to_string(SetIndex) + "\" has only " + std::to_string(VkDescriptorSets[SetIndex].size())
                                 + " sets allocated. You requested for " + std::to_string(Index) + "\n");
    }
    return VkDescriptorSets[SetIndex][Index];
}

void FPipelineDescriptorSetLayout::FreeDescriptorPool(VkDevice LogicalDevice) const
{
    vkDestroyDescriptorPool(LogicalDevice, DescriptorPool, nullptr);
}

void FPipelineDescriptorSetLayout::Reset(VkDevice LogicalDevice)
{
    CleanAll(LogicalDevice);
    if (DescriptorPool)
    {
        FreeDescriptorPool(LogicalDevice);
    }

    Sets.clear();
    VkDescriptorSets.clear();
    PipelineDescriptorSets.clear();
}

FDescriptorSetManager::FDescriptorSetManager(VkDevice LogicalDevice):
        LogicalDevice(LogicalDevice)
{
};

FDescriptorSetManager::~FDescriptorSetManager()
{
    for (auto Entry : PipelineDescriptorSetLayouts)
    {
        Entry.second.CleanAll(LogicalDevice);
    }
}

void FDescriptorSetManager::AddDescriptorLayout(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, const FDescriptor& Descriptor)
{
    PipelineDescriptorSetLayouts[PipelineName].AddDescriptorLayout(DescriptorSetLayoutIndex, DescriptorLayoutIndex, Descriptor);
}

void FDescriptorSetManager::CreateDescriptorSetLayout(const std::vector<VkPushConstantRange>& PushConstantRangeVector, const std::string& PipelineName)
{
    PipelineDescriptorSetLayouts[PipelineName].CreateDescriptorSetLayout(LogicalDevice, PushConstantRangeVector, PipelineName);
}

VkDescriptorSetLayout FDescriptorSetManager::GetVkDescriptorSetLayout(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex)
{
    return PipelineDescriptorSetLayouts[PipelineName].GetVkDescriptorSetLayout(DescriptorSetLayoutIndex);
}

VkPipelineLayout FDescriptorSetManager::GetPipelineLayout(const std::string& PipelineName)
{
    return PipelineDescriptorSetLayouts[PipelineName].GetPipelineLayout();
}


void FDescriptorSetManager::ReserveDescriptorSet(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t Count)
{
    PipelineDescriptorSetLayouts[PipelineName].ReserveDescriptorSet(DescriptorSetLayoutIndex, Count);
}

void FDescriptorSetManager::ReserveDescriptorPool(const std::string& PipelineName)
{
    PipelineDescriptorSetLayouts[PipelineName].ReserveDescriptorPool(LogicalDevice);
}

void FDescriptorSetManager::AllocateAllDescriptorSets(const std::string& PipelineName)
{
    PipelineDescriptorSetLayouts[PipelineName].AllocateAllDescriptorSets(LogicalDevice);
}


void FDescriptorSetManager::UpdateDescriptorSetInfo(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorBufferInfo* BufferInfo)
{
    PipelineDescriptorSetLayouts[PipelineName].UpdateDescriptorSetInfo(LogicalDevice, DescriptorSetLayoutIndex, DescriptorLayoutIndex, Index, BufferInfo);
}

void FDescriptorSetManager::UpdateDescriptorSetInfo(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkDescriptorImageInfo* ImageInfo)
{
    PipelineDescriptorSetLayouts[PipelineName].UpdateDescriptorSetInfo(LogicalDevice, DescriptorSetLayoutIndex, DescriptorLayoutIndex, Index, ImageInfo);
}

void FDescriptorSetManager::UpdateDescriptorSetInfo(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex, uint32_t DescriptorLayoutIndex, uint32_t Index, VkAccelerationStructureKHR* ASInfo)
{
    PipelineDescriptorSetLayouts[PipelineName].UpdateDescriptorSetInfo(LogicalDevice, DescriptorSetLayoutIndex, DescriptorLayoutIndex, Index, ASInfo);
}

VkDescriptorSet FDescriptorSetManager::GetSet(const std::string& PipelineName, uint32_t SetIndex, uint32_t Index)
{
    return PipelineDescriptorSetLayouts[PipelineName].GetSet(SetIndex, Index);
}

void FDescriptorSetManager::DestroyDescriptorSetLayout(const std::string& PipelineName, uint32_t DescriptorSetLayoutIndex)
{
    PipelineDescriptorSetLayouts[PipelineName].DestroyDescriptorSetLayout(LogicalDevice, DescriptorSetLayoutIndex);
}

void FDescriptorSetManager::DestroyPipelineLayout(const std::string& PipelineName)
{
    PipelineDescriptorSetLayouts[PipelineName].DestroyPipelineLayout(LogicalDevice);
}

void FDescriptorSetManager::FreeDescriptorPool(const std::string& PipelineName)
{
    PipelineDescriptorSetLayouts[PipelineName].FreeDescriptorPool(LogicalDevice);
}

void FDescriptorSetManager::Reset(const std::string& PipelineName)
{
    PipelineDescriptorSetLayouts[PipelineName].Reset(LogicalDevice);
}

