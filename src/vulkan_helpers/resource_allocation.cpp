#include "resource_allocation.h"
#include "buffer.h"
#include "vk_context.h"
#include "vk_debug.h"

#include <stdexcept>

FResourceAllocator* ResourceAllocator = nullptr;

FResourceAllocator* GetResourceAllocator()
{
    if (ResourceAllocator == nullptr)
    {
        ResourceAllocator = new FResourceAllocator();
    }

    return ResourceAllocator;
}

void FreeResourceAllocator()
{
    if (ResourceAllocator != nullptr)
    {
        delete ResourceAllocator;
        ResourceAllocator = nullptr;
    }
}


FResourceAllocator::FResourceAllocator()
{
    vkGetPhysicalDeviceMemoryProperties(VK_CONTEXT()->PhysicalDevice, &MemProperties);

    /// Create staging buffer
    StagingBuffer = CreateBuffer(StagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Staging_Buffer");
    RegisterBuffer(StagingBuffer, "Staging_Buffer");
}

FResourceAllocator::~FResourceAllocator()
{
    for (auto& Buffer : Buffers)
    {
        DestroyBuffer(Buffer.second);
    }
}

FMemoryRegion FResourceAllocator::AllocateMemory(VkDeviceSize Size, VkMemoryRequirements MemRequirements, VkMemoryPropertyFlags Properties, bool bDeviceAddressRequired)
{
    /// Allocate Device Memory

    FMemoryRegion MemoryRegion;

    VkMemoryAllocateInfo  AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, Properties);

    if (bDeviceAddressRequired)
    {
        VkMemoryAllocateFlagsInfo MemoryAllocateFlagsInfo{};
        MemoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
        MemoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
        AllocInfo.pNext = &MemoryAllocateFlagsInfo;
    }

    if (vkAllocateMemory(VK_CONTEXT()->LogicalDevice, &AllocInfo, nullptr, &MemoryRegion.Memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    MemoryRegion.MemoryPtrs = {{0, Size}};

    return MemoryRegion;
}

FBuffer FResourceAllocator::RegisterBuffer(FBuffer Buffer, const std::string& Name)
{
    assert(Buffers.find(Name) == Buffers.end() && ("Buffer \"" + Name + "\" already registered!").c_str());
    Buffers[Name] = Buffer;
    return Buffer;
}

FBuffer FResourceAllocator::GetBuffer(const std::string& Name)
{
    assert(Buffers.find(Name) != Buffers.end() && ("Buffer \"" + Name + "\" can not be found!").c_str());
    return Buffers[Name];
}

void FResourceAllocator::UnregisterAndDestroyBuffer(const std::string& Name)
{
    assert(Buffers.find(Name) != Buffers.end() && ("Buffer \"" + Name + "\" can not be found!").c_str());
    DestroyBuffer(Buffers[Name]);
    Buffers.erase(Name);
}

FBuffer FResourceAllocator::CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, const std::string& DebugName)
{
    FBuffer Buffer;

    Buffer.BufferSize = Size;

    /// Create buffer
    VkBufferCreateInfo BufferInfo{};
    BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size = Size;
    BufferInfo.usage = Usage;
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(VK_CONTEXT()->LogicalDevice, &BufferInfo, nullptr, &Buffer.Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer!");
    }

    V::SetName(VK_CONTEXT()->LogicalDevice, Buffer.Buffer, DebugName);

    VkMemoryRequirements MemRequirements;
    vkGetBufferMemoryRequirements(VK_CONTEXT()->LogicalDevice, Buffer.Buffer, &MemRequirements);

    bool bDeviceAddressRequired = (Usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    Buffer.MemoryRegion = AllocateMemory(Size, MemRequirements, Properties, bDeviceAddressRequired);

    V::SetName(VK_CONTEXT()->LogicalDevice, Buffer.MemoryRegion.Memory, DebugName);

    /// Bind Buffer Memory
    vkBindBufferMemory(VK_CONTEXT()->LogicalDevice, Buffer.Buffer, Buffer.MemoryRegion.Memory, 0);

    return Buffer;
}

FBuffer FResourceAllocator::LoadDataToBuffer(FBuffer& Buffer, std::vector<VkDeviceSize> SizesIn, std::vector<VkDeviceSize> OffsetsIn, std::vector<void*> DatasIn)
{
    if (SizesIn.empty())
    {
        /// TODO: Log warning
        return Buffer;
    }

    struct CopySizeOffsetDataPtr
    {
        VkDeviceSize Size;
        VkDeviceSize Offset;
        void* Data;
    };

    std::vector<std::vector<CopySizeOffsetDataPtr>> PreparedData;
    int i = 0;
    /// When some data is separated into multiple load calls
    VkDeviceSize AlreadyPushedPart = 0;

    /// Calculate total size to be pushed
    VkDeviceSize TotalSizeToPush = 0;

    for (auto Entry : SizesIn)
    {
        TotalSizeToPush += Entry;
    }

    while(true)
    {
        std::vector<CopySizeOffsetDataPtr> PreparedDataEntry;
        VkDeviceSize RemainingSpaceInStagingBuffer = StagingBufferSize;

        while (i < SizesIn.size() && RemainingSpaceInStagingBuffer > 0)
        {
            VkDeviceSize HowMuchToPush = SizesIn[i] - AlreadyPushedPart;

            if (HowMuchToPush <= RemainingSpaceInStagingBuffer)
            {
                CopySizeOffsetDataPtr DataToPush{};
                DataToPush.Size = HowMuchToPush;
                DataToPush.Offset = OffsetsIn[i] + AlreadyPushedPart;
                DataToPush.Data = (char*)DatasIn[i] + AlreadyPushedPart;
                PreparedDataEntry.push_back(DataToPush);

                RemainingSpaceInStagingBuffer -= HowMuchToPush;
                TotalSizeToPush -= HowMuchToPush;
                AlreadyPushedPart = 0;
                ++i;

                continue;
            }

            CopySizeOffsetDataPtr LastDataToPush{};
            LastDataToPush.Size = RemainingSpaceInStagingBuffer;
            LastDataToPush.Offset = OffsetsIn[i] + AlreadyPushedPart;
            LastDataToPush.Data = (char*)DatasIn[i] + AlreadyPushedPart;
            PreparedDataEntry.push_back(LastDataToPush);

            AlreadyPushedPart += RemainingSpaceInStagingBuffer;
            TotalSizeToPush -= RemainingSpaceInStagingBuffer;
            RemainingSpaceInStagingBuffer = 0;
        }

        PreparedData.push_back(PreparedDataEntry);

        if (TotalSizeToPush == 0)
        {
            break;
        }
    }

    for (auto &Entry : PreparedData)
    {
        std::vector<VkDeviceSize> Sizes;
        std::vector<VkDeviceSize> SourceOffsets;
        std::vector<VkDeviceSize> DestinationOffsets;
        std::vector<void*> Datas;
        VkDeviceSize Offset = 0;

        for (auto &MiniEntry : Entry)
        {
            Sizes.push_back(MiniEntry.Size);
            SourceOffsets.push_back(Offset);
            DestinationOffsets.push_back(MiniEntry.Offset);
            Datas.push_back(MiniEntry.Data);
            Offset += MiniEntry.Size;

            if(MiniEntry.Offset + MiniEntry.Size > Buffer.CurrentOffset)
            {
                Buffer.CurrentOffset = MiniEntry.Offset + MiniEntry.Size;
            }
        }

        LoadDataToStagingBuffer(Sizes, Datas);
        CopyBuffer(StagingBuffer, Buffer, Sizes, SourceOffsets, DestinationOffsets);
    }

    return Buffer;
}

void FResourceAllocator::LoadDataFromBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data)
{
    for (int i = 0; Size > 0; ++i)
    {
        VkDeviceSize ChunkSize = (Size > StagingBufferSize) ? StagingBufferSize : Size;
        CopyBuffer(Buffer, StagingBuffer, {ChunkSize}, {Offset + (StagingBufferSize * i)}, {0});
        LoadDataFromStagingBuffer(Size, ((char*)Data + (StagingBufferSize * i)), 0);
        Size -= ChunkSize;
    }
}

void FResourceAllocator::LoadDataToStagingBuffer(std::vector<VkDeviceSize> Sizes, std::vector<void*> Datas)
{
    VkDeviceSize TotalSize = 0;

    for (int i = 0; i < Sizes.size(); ++i)
    {
        TotalSize += Sizes[i];
    }

    if (TotalSize > StagingBufferSize)
    {
        assert("Not enough memory in staging buffer");
    }

    void *StagingData;
    vkMapMemory(VK_CONTEXT()->LogicalDevice, StagingBuffer.MemoryRegion.Memory, 0, VK_WHOLE_SIZE, 0, &StagingData);

    VkDeviceSize CurrentOffset = 0;
    for (int i = 0; i < Sizes.size(); ++i)
    {
        memcpy((void*)((char*)StagingData + (std::size_t)CurrentOffset), Datas[i] , (std::size_t) Sizes[i]);
        CurrentOffset += Sizes[i];
    }

    vkUnmapMemory(VK_CONTEXT()->LogicalDevice, StagingBuffer.MemoryRegion.Memory);
}

void FResourceAllocator::LoadDataFromStagingBuffer(VkDeviceSize Size, void* Data, VkDeviceSize Offset)
{
    if (Offset + Size <= StagingBufferSize)
    {
        void *StagingData;
        vkMapMemory(VK_CONTEXT()->LogicalDevice, StagingBuffer.MemoryRegion.Memory, Offset, Size, 0, &StagingData);
        memcpy(Data, StagingData, (std::size_t) Size);
        vkUnmapMemory(VK_CONTEXT()->LogicalDevice, StagingBuffer.MemoryRegion.Memory);

        return;
    }

    assert("Not enough memory in staging buffer");
}

FMemoryRegion FResourceAllocator::LoadDataToImage(FImage& Image, VkDeviceSize Size, void* Data)
{
    LoadDataToStagingBuffer({Size}, {Data});

    CopyBufferToImage(StagingBuffer, Image);

    return Image.MemoryRegion;
}

void FResourceAllocator::CopyBuffer(const FBuffer &SrcBuffer, FBuffer &DstBuffer, std::vector<VkDeviceSize> Sizes, std::vector<VkDeviceSize> SourceOffsets, std::vector<VkDeviceSize> DestinationOffsets)
{
    COMMAND_BUFFER_MANAGER()->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        std::vector<VkBufferCopy> CopyRegions(Sizes.size());

        for (int i = 0; i < Sizes.size(); ++i)
        {
            CopyRegions[i].size = Sizes[i];
            CopyRegions[i].srcOffset = SourceOffsets[i];
            CopyRegions[i].dstOffset = DestinationOffsets[i];
        }

        vkCmdCopyBuffer(CommandBuffer, SrcBuffer.Buffer, DstBuffer.Buffer, CopyRegions.size(), CopyRegions.data());
    }, VK_QUEUE_TRANSFER_BIT);
}

void* FResourceAllocator::Map(FBuffer& Buffer)
{
    void* Data;
    vkMapMemory(VK_CONTEXT()->LogicalDevice, Buffer.MemoryRegion.Memory, 0, Buffer.BufferSize, 0, &Data);
    return Data;
}

void FResourceAllocator::Unmap(FBuffer& Buffer)
{
    vkUnmapMemory(VK_CONTEXT()->LogicalDevice, Buffer.MemoryRegion.Memory);
}

void FResourceAllocator::CopyBufferToImage(FBuffer &SrcBuffer, FImage &DstImage)
{
    COMMAND_BUFFER_MANAGER()->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkBufferImageCopy Region{};
        Region.bufferOffset = 0;
        Region.bufferRowLength = 0;
        Region.bufferImageHeight = 0;

        Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Region.imageSubresource.mipLevel = 0;
        Region.imageSubresource.baseArrayLayer = 0;
        Region.imageSubresource.layerCount = 1;

        Region.imageOffset = {0, 0, 0};
        Region.imageExtent = {DstImage.Width, DstImage.Height, 1};


        vkCmdCopyBufferToImage(CommandBuffer, SrcBuffer.Buffer, DstImage.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);
    }, VK_QUEUE_TRANSFER_BIT);
}

void FResourceAllocator::CopyImageToBuffer(const FImage& SrcImage, FBuffer& DstBuffer)
{
    COMMAND_BUFFER_MANAGER()->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkBufferImageCopy Region{};
        Region.bufferOffset = 0;
        Region.bufferRowLength = 0;
        Region.bufferImageHeight = 0;

        Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Region.imageSubresource.mipLevel = 0;
        Region.imageSubresource.baseArrayLayer = 0;
        Region.imageSubresource.layerCount = 1;

        Region.imageOffset = {0, 0, 0};
        Region.imageExtent = {SrcImage.Width, SrcImage.Height, 1};

        vkCmdCopyImageToBuffer(CommandBuffer, SrcImage.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, DstBuffer.Buffer, 1, &Region);
    }, VK_QUEUE_TRANSFER_BIT);
}

void FResourceAllocator::GetImageData(FImage& SrcImage, void* Data)
{
    CopyImageToBuffer(SrcImage, StagingBuffer);
    LoadDataFromStagingBuffer(SrcImage.Size, Data, 0);
}

void FResourceAllocator::DestroyBuffer(FBuffer& Buffer)
{
    vkDestroyBuffer(VK_CONTEXT()->LogicalDevice, Buffer.Buffer, nullptr);
    vkFreeMemory(VK_CONTEXT()->LogicalDevice, Buffer.MemoryRegion.Memory, nullptr);
    Buffer.Buffer = VK_NULL_HANDLE;
    Buffer.MemoryRegion.Memory = VK_NULL_HANDLE;
}

uint32_t FResourceAllocator::FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
{
    for (uint32_t i = 0; i < MemProperties.memoryTypeCount; ++i)
    {
        if (TypeFilter & (1 << i) && (MemProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}