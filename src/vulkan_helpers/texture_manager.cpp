#include "texture_manager.h"

#include "vk_context.h"

FTextureManager* TextureManager = nullptr;

FTextureManager* GetTextureManager()
{
    if (TextureManager == nullptr)
    {
        TextureManager = new FTextureManager();
    }

    return TextureManager;
}

void FreeTextureManager()
{
    if (TextureManager != nullptr)
    {
        delete TextureManager;
        TextureManager = nullptr;
    }
}

FTextureManager::FTextureManager()
{
    DummyImage = VK_CONTEXT()->CreateImage2D(1, 1, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->GetLogicalDevice(), "V_Dummy_Image");

    DummyImage->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    /// Populate all image views with dummy image view
    DescriptorImageInfos.resize(MAX_TEXTURES);

    for (int i = 0; i < MAX_TEXTURES; ++i)
    {
        DescriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        DescriptorImageInfos[i].imageView = DummyImage->View;
        DescriptorImageInfos[i].sampler = VK_NULL_HANDLE;
    }
}

FTextureManager::~FTextureManager()
{
    DummyImage = nullptr;
    Textures.clear();
    FramebufferImages.clear();
}

ImagePtr FTextureManager::CreateStorageImage(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName)
{
    return VK_CONTEXT()->CreateImage2D(WidthIn, HeightIn, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                              VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                              VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->LogicalDevice, DebugName);
}

ImagePtr FTextureManager::CreateClearableStorageImage(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName)
{
	return VK_CONTEXT()->CreateImage2D(WidthIn, HeightIn, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->LogicalDevice, DebugName);
}


ImagePtr FTextureManager::CreateSampledStorageImage(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName)
{
    return VK_CONTEXT()->CreateImage2D(WidthIn, HeightIn, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                      VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                      VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->LogicalDevice, DebugName);
}

ImagePtr FTextureManager::CreateColorAttachment(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName)
{
	return VK_CONTEXT()->CreateImage2D(WidthIn, HeightIn, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->LogicalDevice, DebugName);
}

uint32_t FTextureManager::RegisterTexture(const ImagePtr& ImagePointer, VkImageLayout ImageLayout, const std::string& Name)
{
    uint32_t Index = Textures.size();
    Textures.push_back(ImagePointer);
    DescriptorImageInfos[Index].imageLayout = ImageLayout;
    DescriptorImageInfos[Index].imageView = ImagePointer->View;
    DescriptorImageInfos[Index].sampler = VK_NULL_HANDLE;

    TextureNameToIndexMap[Name] = Index;

    return Index;
}

uint32_t FTextureManager::RegisterFramebuffer(const ImagePtr& ImagePointer, const std::string& Name)
{
    uint32_t Index = FramebufferImages.size();
    FramebufferImages.push_back(ImagePointer);

    FramebufferNameToIndexMap[Name] = Index;

    return Index;
}

void FTextureManager::UnregisterAndFreeFramebuffer(uint32_t FramebufferIndex)
{
	auto Image = GetFramebufferImage(FramebufferIndex);

	FramebufferImages[FramebufferIndex] = nullptr;

	for (auto It = FramebufferNameToIndexMap.begin(); It != FramebufferNameToIndexMap.end(); ++It)
	{
		if (It->second == FramebufferIndex)
		{
			FramebufferNameToIndexMap.erase(It);
			break;
		}
	}
}

ImagePtr FTextureManager::GetTexture(uint32_t TextureIndex)
{
    return Textures[TextureIndex];
}

ImagePtr FTextureManager::GetTexture(const std::string& Name)
{
    return Textures[TextureNameToIndexMap[Name]];
}

ImagePtr FTextureManager::GetFramebufferImage(uint32_t FramebufferImageIndex)
{
    return FramebufferImages[FramebufferImageIndex];
}

ImagePtr FTextureManager::GetFramebufferImage(const std::string& Name)
{
    return FramebufferImages[FramebufferNameToIndexMap[Name]];
}

VkDescriptorImageInfo* FTextureManager::GetDescriptorImageInfos()
{
    return DescriptorImageInfos.data();
}