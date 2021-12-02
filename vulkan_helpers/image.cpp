#include "image.h"

#include "context.h"

size_t FImage::GetHash()
{
    return std::hash<FImage>()(*this);
}

FImage::FImage(uint32_t Width, uint32_t Height, uint32_t MipLevels, VkSampleCountFlagBits NumSamples, VkFormat Format, VkImageTiling Tiling,
               VkImageUsageFlags Usage, VkMemoryPropertyFlags Properties,  VkImageAspectFlags AspectFlags, VkDevice Device) :
Device(Device), Width(Width), Height(Height), MipLevels(MipLevels), Samples(NumSamples), Format(Format),
Tiling(Tiling), Usage(Usage), Properties(Properties), AspectFlags(AspectFlags)
{
    CreateImage();
    AllocateMemory();
    BindMemoryToImage();
    CreateImageView();
}


void FImage::Wrap(VkImage ImageToWrap, VkFormat Format, VkImageAspectFlags AspectFlags, uint32_t MipLevels, VkDevice LogicalDevice, FImage& Image)
{
    Image.Image = ImageToWrap;
    Image.Format = Format;
    Image.Device = LogicalDevice;
    Image.AspectFlags = AspectFlags;
    Image.MipLevels = MipLevels;
    Image.bIsWrappedImage = true;

    Image.CreateImageView();
}

FImage::~FImage()
{
    vkDestroyImageView(Device, View, nullptr);
    if (!bIsWrappedImage)
    {
        vkDestroyImage(Device, Image, nullptr);
        vkFreeMemory(Device, Memory, nullptr);
    }
}

void FImage::Transition(VkImageLayout  OldLayout, VkImageLayout NewLayout)
{
    auto& Context = GetContext();

    VkCommandBuffer CommandBuffer = Context.BeginSingleTimeCommands();

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

    if (NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (Context.HasStensilComponent(Format))
        {
            Barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
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
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(OldLayout == VK_IMAGE_LAYOUT_UNDEFINED && NewLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(CommandBuffer, SourceStage, DestinationStage, 0, 0, nullptr, 0, nullptr, 1, &Barrier);

    Context.EndSingleTimeCommand(CommandBuffer);
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
}

void FImage::AllocateMemory()
{
    VkMemoryRequirements MemRequirements;
    vkGetImageMemoryRequirements(Device, Image, &MemRequirements);

    VkMemoryAllocateInfo AllocInfo{};
    AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    AllocInfo.allocationSize = MemRequirements.size;
    AllocInfo.memoryTypeIndex = GetContext().ResourceAllocator->FindMemoryType(MemRequirements.memoryTypeBits, Properties);

    if (vkAllocateMemory(Device, &AllocInfo, nullptr, &Memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory!");
    }
}

void FImage::BindMemoryToImage()
{
    vkBindImageMemory(Device, Image, Memory, 0);
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
}

bool operator<(const FImage& A, const FImage& B)
{
    return A.Image < B.Image;
}
