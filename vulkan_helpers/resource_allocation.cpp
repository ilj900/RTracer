#include "resource_allocation.h"
#include "buffer.h"
#include "vk_context.h"
#include "vk_debug.h"

#include <stdexcept>

FResourceAllocator::FResourceAllocator(VkPhysicalDevice PhysicalDevice, VkDevice Device, FVulkanContext* Context)
    :Device(Device),
     Context(Context)
{
    vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &MemProperties);

    /// Create staging buffer
    StagingBuffer = CreateBuffer(StagingBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, "Staging_Buffer");
}

FResourceAllocator::~FResourceAllocator()
{
    DestroyBuffer(StagingBuffer);
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

    if (vkAllocateMemory(Device, &AllocInfo, nullptr, &MemoryRegion.Memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate buffer memory!");
    }

    return MemoryRegion;
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

    if (vkCreateBuffer(Device, &BufferInfo, nullptr, &Buffer.Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create buffer!");
    }

    V::SetName(Device, Buffer.Buffer, DebugName);

    VkMemoryRequirements MemRequirements;
    vkGetBufferMemoryRequirements(Device, Buffer.Buffer, &MemRequirements);

    bool bDeviceAddressRequired = (Usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) == VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

    Buffer.MemoryRegion = AllocateMemory(Size, MemRequirements, Properties, bDeviceAddressRequired);

    V::SetName(Device, Buffer.MemoryRegion.Memory, DebugName + "_Memmory");

    /// Bind Buffer Memory
    vkBindBufferMemory(Device, Buffer.Buffer, Buffer.MemoryRegion.Memory, 0);

    return Buffer;
}

FMemoryPtr FResourceAllocator::PushDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, void* Data)
{
    auto RemainingSize = Buffer.BufferSize - Buffer.CurrentOffset;
    if (Size <= RemainingSize)
    {
        FMemoryPtr FMemoryPtr;
        FMemoryPtr.Size = Size;
        FMemoryPtr.Offset = Buffer.CurrentOffset;
        LoadDataToBuffer(Buffer, Size, Buffer.CurrentOffset, Data);
        Buffer.MemoryRegion.MemoryPtrs.push_back(FMemoryPtr);
        return FMemoryPtr;
    }
    /// TODO: Try to compact the data in buffer
    assert(false && "Not enough space in Buffer");
    return FMemoryPtr();
}

FBuffer FResourceAllocator::LoadDataToBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data)
{
    auto SizeCopy = Size;

    for (int i = 0; Size > 0; ++i)
    {
        VkDeviceSize ChunkSize = (Size > StagingBufferSize) ? StagingBufferSize : Size;
        LoadDataToStagingBuffer(ChunkSize, ((char*)Data + (StagingBufferSize * i)), 0);
        CopyBuffer(StagingBuffer, Buffer, ChunkSize, 0, Offset + (StagingBufferSize * i));
        Size -= ChunkSize;
    }

    if (Offset + SizeCopy > Buffer.CurrentOffset)
    {
        Buffer.CurrentOffset = Offset + SizeCopy;
    }

    return Buffer;
}

FBuffer FResourceAllocator::LoadDataToBuffer(FBuffer& Buffer, std::vector<VkDeviceSize> Sizes, std::vector<VkDeviceSize> Offsets, std::vector<void*> Datas)
{
    struct CopySizeOffsetDataPtr
    {
        VkDeviceSize Size;
        VkDeviceSize Offset;
        void* Data;
    };

    std::vector<std::vector<CopySizeOffsetDataPtr>> PreparedData;
    int i = 0;

    while(true)
    {
        std::vector<CopySizeOffsetDataPtr> PreparedDataEntry;
        VkDeviceSize RemainingSpaceInStagingBuffer = StagingBufferSize;
        /// When some data is separated into multiple load calls
        VkDeviceSize AlreadyPushedPart = 0;

        while (i < Sizes.size() || RemainingSpaceInStagingBuffer > 0)
        {
            VkDeviceSize HowMuchToPush = Sizes[i] - AlreadyPushedPart;

            if (HowMuchToPush <= RemainingSpaceInStagingBuffer)
            {
                CopySizeOffsetDataPtr DataToPush;
                DataToPush.Size = HowMuchToPush;
                DataToPush.Offset = Offsets[i] + AlreadyPushedPart;
                DataToPush.Data = (char*)Datas[i] + AlreadyPushedPart;
                PreparedDataEntry.push_back(DataToPush);

                RemainingSpaceInStagingBuffer -= HowMuchToPush;
                AlreadyPushedPart = 0;
                ++i;

                continue;
            }

            CopySizeOffsetDataPtr DataToPush;
            DataToPush.Size = RemainingSpaceInStagingBuffer;
            DataToPush.Offset = Offsets[i] + AlreadyPushedPart;
            DataToPush.Data = (char*)Datas[i] + AlreadyPushedPart;
            PreparedDataEntry.push_back(DataToPush);

            AlreadyPushedPart += RemainingSpaceInStagingBuffer;
            RemainingSpaceInStagingBuffer = 0;
        }

        PreparedData.push_back(PreparedDataEntry);
        break;
    }

    for (auto &Entry : PreparedData)
    {

    }

    return Buffer;
}

void FResourceAllocator::LoadDataFromBuffer(FBuffer& Buffer, VkDeviceSize Size, VkDeviceSize Offset, void* Data)
{
    for (int i = 0; Size > 0; ++i)
    {
        VkDeviceSize ChunkSize = (Size > StagingBufferSize) ? StagingBufferSize : Size;
        CopyBuffer(Buffer, StagingBuffer, ChunkSize, Offset + (StagingBufferSize * i), 0);
        LoadDataFromStagingBuffer(Size, ((char*)Data + (StagingBufferSize * i)), 0);
        Size -= ChunkSize;
    }
}

void FResourceAllocator::LoadDataToStagingBuffer(VkDeviceSize Size, void* Data, VkDeviceSize Offset)
{
    if (Offset + Size <= StagingBufferSize)
    {
        void *StagingData;
        vkMapMemory(Device, StagingBuffer.MemoryRegion.Memory, Offset, Size, 0, &StagingData);
        memcpy(StagingData, Data, (std::size_t) Size);
        vkUnmapMemory(Device, StagingBuffer.MemoryRegion.Memory);

        return;
    }

    assert("Not enough memory in staging buffer");
}

void FResourceAllocator::LoadDataFromStagingBuffer(VkDeviceSize Size, void* Data, VkDeviceSize Offset)
{
    if (Offset + Size <= StagingBufferSize)
    {
        void *StagingData;
        vkMapMemory(Device, StagingBuffer.MemoryRegion.Memory, Offset, Size, 0, &StagingData);
        memcpy(Data, StagingData, (std::size_t) Size);
        vkUnmapMemory(Device, StagingBuffer.MemoryRegion.Memory);

        return;
    }

    assert("Not enough memory in staging buffer");
}

FMemoryRegion FResourceAllocator::LoadDataToImage(FImage& Image, VkDeviceSize Size, void* Data)
{
    LoadDataToStagingBuffer(Size, Data, 0);

    CopyBufferToImage(StagingBuffer, Image);

    return Image.MemoryRegion;
}

void FResourceAllocator::CopyBuffer(FBuffer &SrcBuffer, FBuffer &DstBuffer, VkDeviceSize Size, VkDeviceSize SourceOffset, VkDeviceSize DestinationOffset)
{
    Context->CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkBufferCopy CopyRegion{};
        CopyRegion.size = Size;
        CopyRegion.srcOffset = SourceOffset;
        CopyRegion.dstOffset = DestinationOffset;
        vkCmdCopyBuffer(CommandBuffer, SrcBuffer.Buffer, DstBuffer.Buffer, 1, &CopyRegion);
    });
}

void* FResourceAllocator::Map(FBuffer& Buffer)
{
    void* Data;
    vkMapMemory(Device, Buffer.MemoryRegion.Memory, 0, Buffer.BufferSize, 0, &Data);
    return Data;
}

void FResourceAllocator::Unmap(FBuffer& Buffer)
{
    vkUnmapMemory(Device, Buffer.MemoryRegion.Memory);
}

void FResourceAllocator::CopyBufferToImage(FBuffer &SrcBuffer, FImage &DstImage)
{
    Context->CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
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
    });
}

void FResourceAllocator::CopyImageToBuffer(const FImage& SrcImage, FBuffer& DstBuffer)
{
    Context->CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
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
    });
}

void FResourceAllocator::GetImageData(FImage& SrcImage, void* Data)
{
    CopyImageToBuffer(SrcImage, StagingBuffer);
    LoadDataFromStagingBuffer(SrcImage.Size, Data, 0);
}

void FResourceAllocator::DestroyBuffer(FBuffer& Buffer)
{
    vkDestroyBuffer(Device, Buffer.Buffer, nullptr);
    vkFreeMemory(Device, Buffer.MemoryRegion.Memory, nullptr);
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