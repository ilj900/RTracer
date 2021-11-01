#include "descriptors.h"

#include <stdexcept>

bool operator==(const FDescriptor& A, const FDescriptor& B)
{
    return (A.BindingIndex == B.BindingIndex);
}

FDescriptorSetLayout::FDescriptorSetLayout(const FDescriptor& Descriptor)
{
    Descriptors.push_back(Descriptor);
}

FDescriptorSetManager::FDescriptorSetManager(VkDevice LogicalDevice):
        LogicalDevice(LogicalDevice)
{
};

void FDescriptorSetManager::AddDescriptorLayout(const std::string& DescriptorSetLayoutName, uint32_t DescriptorSetLayoutIndex, const FDescriptor& Descriptor)
{
    auto Iterator = DescriptorSetLayouts.find(DescriptorSetLayoutName);

    /// If the map is not empty, then we have to run some checks, before inserting the value
    if (Iterator != DescriptorSetLayouts.end())
    {
        /// Fetch the set index.
        auto SetIndex = Iterator->second.second;
        /// If the set index is different, then it's an error
        if (SetIndex != DescriptorSetLayoutIndex)
        {
            throw std::runtime_error("For given descriptor set layout name: \"" + DescriptorSetLayoutName + "\" provided index: \"" + std::to_string(DescriptorSetLayoutIndex) +
            "\" doesn't match the one already assigned to it: \"" + std::to_string(SetIndex) + "\".\n");
        }
        /// Fetch vector of descriptors for that set
        auto Descriptors = Iterator->second.first.Descriptors;
        /// Check whether there are already descriptor with that binding
        for (auto& Entry : Descriptors)
        {
            /// There can't be two descriptor layouts with the same binding
            if (Entry == Descriptor)
            {
                throw std::runtime_error("Attempt to add descriptor layout: \"" + Descriptor.Name + "\" with binding index: \"" + std::to_string(Descriptor.BindingIndex) +
                "\", that already exists in descriptor set: \"" +  DescriptorSetLayoutName + "\".");
            }
        }
        /// Add descriptor
        Descriptors.push_back(Descriptor);
    }
    else
    {
        DescriptorSetLayouts[DescriptorSetLayoutName] = {FDescriptorSetLayout(Descriptor), DescriptorSetLayoutIndex};
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
            DescriptorSetLayoutBinding.binding = Layout.BindingIndex;
            DescriptorSetLayoutBinding.descriptorType = Layout.Type;
            DescriptorSetLayoutBinding.descriptorCount = 1;
            DescriptorSetLayoutBinding.stageFlags = Layout.StageFlags;
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
            TypeCount[Descriptor.Type] += Count;
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

void FDescriptorSetManager::CreateDescriptorSets()
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

VkDescriptorSet FDescriptorSetManager::GetSet(const std::string& Name, uint32_t Index)
{
    if (DescriptorSets.find(Name) == DescriptorSets.end())
    {
        throw std::runtime_error("Descriptor set: \"" + Name + "\" not registered.\n");
    }
    if (DescriptorSets[Name].size() >= Index)
    {
        throw std::runtime_error("Descriptor set: \"" + Name + "\" has only " + std::to_string(DescriptorSets[Name].size())
        + " sets allocated. You requested for " + std::to_string(Index) + "\n");
    }
    return DescriptorSets[Name][Index];
}
