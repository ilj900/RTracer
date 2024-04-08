#include "image.h"

#include "vk_context.h"
#include "vk_debug.h"

#include "iostream"

FImage::FImage(uint32_t Width, uint32_t Height, bool bMipMapsRequired, VkSampleCountFlagBits NumSamples,
               VkFormat Format, VkImageTiling Tiling, VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties,
               VkImageAspectFlags AspectFlags, VkDevice Device, const std::string& DebugImageName) :
Device(Device), Width(Width), MipLevels(1), Height(Height), Samples(NumSamples), Format(Format),
Tiling(Tiling), Usage(Usage), Properties(Properties), AspectFlags(AspectFlags), DebugName(DebugImageName)
{
    /// Calculate number of MipLevels in advance
    if (bMipMapsRequired)
    {
        MipLevels = static_cast<uint32_t>(std::floor(static_cast<float>(std::log2(std::max(Width, Height))))) + 1;
    }

    CreateImage();
    AllocateMemory();
    BindMemoryToImage();
    CreateImageView();
}

FImage::FImage(VkImage ImageToWrap, VkFormat Format, VkImageAspectFlags AspectFlags, VkDevice LogicalDevice, const std::string& DebugImageName)
{
    bIsWrappedImage = true;

    Image = ImageToWrap;
    this->Format = Format;
    Device = LogicalDevice;
    this->AspectFlags = AspectFlags;
    MipLevels = 1;
    DebugName = DebugImageName;

    V::SetName(Device, ImageToWrap, DebugImageName);

    CreateImageView();
}


FImage::~FImage()
{
    vkDestroyImageView(Device, View, nullptr);
    if (!bIsWrappedImage)
    {
        vkDestroyImage(Device, Image, nullptr);
        vkFreeMemory(Device, MemoryRegion.Memory, nullptr);
    }
}

void FImage::Transition(VkImageLayout  OldLayout, VkImageLayout NewLayout)
{
    VK_CONTEXT().CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer CommandBuffer)
    {
        VkImageMemoryBarrier Barrier{};
        Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        Barrier.oldLayout = OldLayout;
        Barrier.newLayout = NewLayout;
        Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        Barrier.image = Image;
        Barrier.subresourceRange.baseMipLevel = 0;
        Barrier.subresourceRange.levelCount = MipLevels;
        Barrier.subresourceRange.baseArrayLayer = 0;
        Barrier.subresourceRange.layerCount = 1;

        if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
            Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (VK_CONTEXT().HasStensilComponent(Format)) {
                Barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        } else {
            Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkPipelineStageFlags SourceStage;
        VkPipelineStageFlags DestinationStage;

        if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            Barrier.srcAccessMask = 0;
            Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   NewLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            Barrier.srcAccessMask = 0;
            Barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                   NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            Barrier.srcAccessMask = 0;
            Barrier.dstAccessMask = 0;

            SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            DestinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        }  else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   NewLayout == VK_IMAGE_LAYOUT_GENERAL) {
            Barrier.srcAccessMask = 0;
            Barrier.dstAccessMask = 0;

            SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            DestinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        } else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                   NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            Barrier.srcAccessMask = 0;
            Barrier.dstAccessMask =
                    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            DestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        } else {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(CommandBuffer, SourceStage, DestinationStage, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

    }, "Transitioning_Image_Layout");

    CurrentLayout = NewLayout;
}

void FImage::SwapData(FImage& Other)
{
    VkImage TempImage = Image;
    auto TempMemory = MemoryRegion;
    VkImageView TempView = View;

    VkFormat TempFormat = Format;
    VkSampleCountFlagBits TempSamples= Samples;
    VkDevice TempDevice = Device;
    VkImageTiling TempTiling = Tiling;
    VkImageUsageFlags TempUsage = Usage;
    VkMemoryPropertyFlags TempProperties = Properties;
    VkImageAspectFlags TempAspectFlags = AspectFlags;

    uint32_t TempWidth = Width;
    uint32_t TempHeight = Height;
    uint32_t TempMipLevels = MipLevels;

    Image = Other.Image;
    MemoryRegion = Other.MemoryRegion;
    View = Other.View;

    Format = Other.Format;
    Samples = Other.Samples;
    Device = Other.Device;
    Tiling = Other.Tiling;
    Usage = Other.Usage;
    Properties = Other.Properties;
    AspectFlags = Other.AspectFlags;

    Width = Other.Width;
    Height = Other.Height;;
    MipLevels = Other.MipLevels;

    Other.Image = TempImage;
    Other.MemoryRegion = TempMemory;
    Other.View = TempView;
    Other.Format = TempFormat;
    Other.Samples = TempSamples;
    Other.Device = TempDevice;
    Other.Tiling = TempTiling;
    Other.Usage = TempUsage;
    Other.Properties = TempProperties;
    Other.AspectFlags = TempAspectFlags;
    Other.Width = TempWidth;
    Other.Height = TempHeight;
    Other.MipLevels = TempMipLevels;
}

void FImage::CreateImage()
{
    VkImageCreateInfo ImageInfo{};
    ImageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageInfo.imageType = VK_IMAGE_TYPE_2D;
    ImageInfo.extent.width = Width;
    ImageInfo.extent.height = Height;
    ImageInfo.extent.depth = 1;
    ImageInfo.mipLevels = MipLevels;
    ImageInfo.arrayLayers = 1;
    ImageInfo.format = Format;
    ImageInfo.tiling = Tiling;
    ImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageInfo.usage = Usage;
    ImageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageInfo.samples = Samples;

    if (vkCreateImage(Device, &ImageInfo, nullptr, &Image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image!");
    }

    V::SetName(Device, Image, DebugName);
}

void FImage::AllocateMemory()
{
    VkMemoryRequirements MemRequirements;
    vkGetImageMemoryRequirements(Device, Image, &MemRequirements);

    Size = MemRequirements.size;

    MemoryRegion = RESOURCE_ALLOCATOR()->AllocateMemory(MemRequirements.size, MemRequirements, Properties);
}

void FImage::BindMemoryToImage()
{
    vkBindImageMemory(Device, Image, MemoryRegion.Memory, 0);
}

void FImage::CreateImageView()
{
    VkImageViewCreateInfo ViewInfo{};
    ViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ViewInfo.image = Image;
    ViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ViewInfo.format = Format;
    ViewInfo.subresourceRange.aspectMask = AspectFlags;
    ViewInfo.subresourceRange.baseMipLevel = 0;
    ViewInfo.subresourceRange.levelCount = MipLevels;
    ViewInfo.subresourceRange.baseArrayLayer = 0;
    ViewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(Device, &ViewInfo, nullptr, &View) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture image view!");
    }

    V::SetName(Device, View, DebugName);
}

void FImage::GenerateMipMaps()
{
    VkFormatProperties FormatProperties;
    vkGetPhysicalDeviceFormatProperties(VK_CONTEXT().PhysicalDevice, Format, &FormatProperties);

    if (!(FormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    VK_CONTEXT().CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer& CommandBuffer)
    {
        VkImageMemoryBarrier Barrier{};
        Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        Barrier.image = Image;
        Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        Barrier.subresourceRange.baseArrayLayer = 0;
        Barrier.subresourceRange.layerCount = 1;
        Barrier.subresourceRange.levelCount = 1;

        int32_t MipWidth = Width;
        int32_t MipHeight = Height;

        for (uint32_t i = 1; i < MipLevels; ++i) {
            Barrier.subresourceRange.baseMipLevel = i - 1;
            Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            Barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            Barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                 nullptr, 0, nullptr, 1, &Barrier);

            VkImageBlit Blit{};
            Blit.srcOffsets[0] = {0, 0, 0};
            Blit.srcOffsets[1] = {MipWidth, MipHeight, 1};
            Blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            Blit.srcSubresource.mipLevel = i - 1;
            Blit.srcSubresource.baseArrayLayer = 0;
            Blit.srcSubresource.layerCount = 1;
            Blit.dstOffsets[0] = {0, 0, 0};
            Blit.dstOffsets[1] = {MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight / 2 : 1, 1};
            Blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            Blit.dstSubresource.mipLevel = i;
            Blit.dstSubresource.baseArrayLayer = 0;
            Blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Blit, VK_FILTER_LINEAR);

            Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            Barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                                 0, 0, nullptr, 0, nullptr, 1, &Barrier);

            if (MipWidth > 1) {
                MipWidth /= 2;
            }

            if (MipHeight > 1) {
                MipHeight /= 2;
            }
        }

        Barrier.subresourceRange.baseMipLevel = MipLevels - 1;
        Barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        Barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(CommandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
                             nullptr, 0, nullptr, 1, &Barrier);

    }, "Generating_MipMaps");
}

void FImage::Resolve(FImage& ImageToResolveTo)
{
    if (ImageToResolveTo.Format != Format)
    {
        throw std::runtime_error("Failed to resolve image because formats are different.");
    }

    VK_CONTEXT().CommandBufferManager->RunSingletimeCommand([&, this](VkCommandBuffer& CommandBuffer)
    {
        VkImageResolve ImageResolve{};
        ImageResolve.srcOffset = {0, 0, 0};
        ImageResolve.dstOffset = {0, 0, 0};
        ImageResolve.extent = {Width, Height, 1};

        ImageResolve.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageResolve.srcSubresource.mipLevel = 0;
        ImageResolve.srcSubresource.baseArrayLayer = 0;
        ImageResolve.srcSubresource.layerCount = 1;

        ImageResolve.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        ImageResolve.dstSubresource.mipLevel = 0;
        ImageResolve.dstSubresource.baseArrayLayer = 0;
        ImageResolve.dstSubresource.layerCount = 1;

        vkCmdResolveImage(CommandBuffer, Image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, ImageToResolveTo.Image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &ImageResolve);
    }, "Resolving_Image");
}

size_t FImage::GetHash()
{
    return std::hash<FImage>()(*this);
}

bool operator<(const FImage& A, const FImage& B)
{
    return A.Image < B.Image;
}
