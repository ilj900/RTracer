#include "image_manager.h"

#include "buffer.h"
#include "context.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void FImageManager::Init(FContext& Context)
{
    this->Context = &Context;
    Images.reserve(1024);
}

FImageManager::~FImageManager() {};

void FImageManager::LoadImageFromFile(const std::string& ImageName, const std::string& Path)
{
    /// Load data from image file
    int TexWidth, TexHeight, TexChannels;
    stbi_uc* Pixels = stbi_load(Path.c_str(), &TexWidth, &TexHeight, &TexChannels, STBI_rgb_alpha);
    VkDeviceSize ImageSize = TexWidth * TexHeight * 4;

    if (!Pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    /// Load data into staging buffer
    FBuffer TempStagingBuffer = Context->ResourceAllocator->CreateBuffer(ImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void *Data;
    vkMapMemory(Context->LogicalDevice, TempStagingBuffer.Memory, 0, ImageSize, 0, &Data);
    memcpy(Data, Pixels, static_cast<size_t>(ImageSize));
    vkUnmapMemory(Context->LogicalDevice, TempStagingBuffer.Memory);
    stbi_image_free(Pixels);

    CreateImage(ImageName, TexWidth, TexHeight, true, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                                     VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                     VK_IMAGE_ASPECT_COLOR_BIT);

    Images.back().Image.Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(TempStagingBuffer, ImageName);
    /// Generate MipMaps only after we loaded image data
    Images.back().Image.GenerateMipMaps();

    Context->ResourceAllocator->DestroyBuffer(TempStagingBuffer);
}

void FImageManager::CreateImage(const std::string& ImageName, uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties, VkImageAspectFlags AspectFlags)
{
    Images.emplace_back(ImageName, Width, Height, bMipMapsRequired, NumSamples, Format, Tiling, Usage, Properties, AspectFlags, Context->LogicalDevice);
    auto ImageHash = Images.back().Image.GetHash();
    HashToIndexMap[ImageHash] = Images.size() - 1;
    NameToHashMap[ImageName] = ImageHash;
}

void FImageManager::RemoveImage(const std::string& ImageName)
{
    auto Hash = NameToHashMap[ImageName];
    auto Index = HashToIndexMap[Hash];

    /// Remove entry from maps
    NameToHashMap.erase(NameToHashMap.find(ImageName));
    HashToIndexMap.erase(HashToIndexMap.find(Hash));

    /// Remove image from vector by replacing it with the one from the back of the vector
    if (Index != (Images.size() - 1))
    {
        Images.back().Image.SwapData(Images[Index].Image);
        auto TempName = Images.back().Name;
        Images[Index].Name = Images.back().Name;
        Images.back().Name = TempName;


                /// Update data of the image we moved from the back of the vector
        HashToIndexMap[NameToHashMap[Images[Index].Name]] = Index;
    }
    Images.pop_back();
}

FImage& FImageManager::operator()(const std::string& ImageName)
{
    return Images[HashToIndexMap[NameToHashMap[ImageName]]].Image;
}


void FImageManager::CopyBufferToImage(const FBuffer& Buffer, const std::string& ImageName)
{
    VkCommandBuffer CommandBuffer = Context->BeginSingleTimeCommands();

    auto& Image = operator()(ImageName);

    VkBufferImageCopy Region{};
    Region.bufferOffset = 0;
    Region.bufferRowLength = 0;
    Region.bufferImageHeight = 0;

    Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.imageSubresource.mipLevel = 0;
    Region.imageSubresource.baseArrayLayer = 0;
    Region.imageSubresource.layerCount = 1;

    Region.imageOffset = {0, 0, 0};
    Region.imageExtent = {Image.Width, Image.Height, 1};


    vkCmdCopyBufferToImage(CommandBuffer, Buffer.Buffer, Image.Image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

    Context->EndSingleTimeCommand(CommandBuffer);
}

void FImageManager::CopyImageToBuffer(const std::string& ImageName, const FBuffer& Buffer)
{
    auto CommandBuffer = Context->BeginSingleTimeCommands();

    auto& Image = operator()(ImageName);

    VkBufferImageCopy Region{};
    Region.bufferOffset = 0;
    Region.bufferRowLength = 0;
    Region.bufferImageHeight = 0;

    Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.imageSubresource.mipLevel = 0;
    Region.imageSubresource.baseArrayLayer = 0;
    Region.imageSubresource.layerCount = 1;

    Region.imageOffset = {0, 0, 0};
    Region.imageExtent = {Image.Width, Image.Height, 1};

    vkCmdCopyImageToBuffer(CommandBuffer, Image.Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Buffer.Buffer, 1, &Region);

    Context->EndSingleTimeCommand(CommandBuffer);
}

void FImageManager::SaveImage(const std::string& ImageName)
{
    std::vector<char> Data;

    auto& Image = operator()(ImageName);

    FetchImageData(ImageName, Data);

    stbi_write_bmp((ImageName + ".png").c_str(), Image.Width, Image.Height, 4, Data.data());
}

void FImageManager::FetchImageData(const std::string& ImageName, std::vector<char>& Data)
{
    auto& Image = this->operator()(ImageName);
    uint32_t PixelSize = 4;
    switch (Image.Format)
    {
        case VK_FORMAT_R8G8B8A8_SRGB:
            PixelSize = 4;
            break;
        case VK_FORMAT_R32_UINT:
            PixelSize = 4;
            break;
    }

    auto& Context = GetContext();
    uint32_t Size = Image.Height * Image.Width * PixelSize;
    FBuffer Buffer = Context.ResourceAllocator->CreateBuffer(Size, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    Image.Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    CopyImageToBuffer(ImageName, Buffer);

    Data.resize(Size);

    void* BufferData;
    vkMapMemory(Context.LogicalDevice, Buffer.Memory, 0, Buffer.Size, 0, &BufferData);
    memcpy(Data.data(), BufferData, (std::size_t)Buffer.Size);
    vkUnmapMemory(Context.LogicalDevice, Buffer.Memory);

    Context.ResourceAllocator->DestroyBuffer(Buffer);
}