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
    DummyImageFloat = VK_CONTEXT()->CreateImage2D(1, 1, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->GetLogicalDevice(), "V_Dummy_Image_Float");

    DummyImageFloat->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	DummyImageUint = VK_CONTEXT()->CreateImage2D(1, 1, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_UINT, VK_IMAGE_TILING_OPTIMAL,
									   VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									   VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->GetLogicalDevice(), "V_Dummy_Image_Uint");

	DummyImageUint->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	DummyImageInt = VK_CONTEXT()->CreateImage2D(1, 1, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32_SINT, VK_IMAGE_TILING_OPTIMAL,
									   VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
									   VK_IMAGE_ASPECT_COLOR_BIT, VK_CONTEXT()->GetLogicalDevice(), "V_Dummy_Image_Int");

	DummyImageInt->Transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    /// Populate all image views with dummy image view
    DescriptorImageInfosFloat = std::vector<VkDescriptorImageInfo>(MAX_TEXTURES, {VK_NULL_HANDLE, DummyImageFloat->View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
	DescriptorImageInfosUint = std::vector<VkDescriptorImageInfo>(MAX_TEXTURES, {VK_NULL_HANDLE, DummyImageUint->View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
	DescriptorImageInfosInt = std::vector<VkDescriptorImageInfo>(MAX_TEXTURES, {VK_NULL_HANDLE, DummyImageInt->View, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
}

FTextureManager::~FTextureManager()
{
    DummyImageFloat = nullptr;
	DummyImageUint = nullptr;
	DummyImageInt = nullptr;
    TexturesFloat.clear();
	TexturesUint.clear();
	TexturesInt.clear();
	FramebufferNameToImageMap.clear();
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
	uint32_t Index = -1;

	if (ImagePointer->Format == VK_FORMAT_R8G8B8A8_UNORM || ImagePointer->Format == VK_FORMAT_R8G8B8_UNORM || ImagePointer->Format == VK_FORMAT_R8G8_UNORM ||
		ImagePointer->Format == VK_FORMAT_R8_UNORM || ImagePointer->Format == VK_FORMAT_R32G32B32A32_SFLOAT)
	{
		Index = TexturesFloat.size();
		TexturesFloat.push_back(ImagePointer);
		DescriptorImageInfosFloat[Index].imageLayout = ImageLayout;
		DescriptorImageInfosFloat[Index].imageView = ImagePointer->View;
		DescriptorImageInfosFloat[Index].sampler = VK_NULL_HANDLE;
		return Index;
	}

	if (ImagePointer->Format == VK_FORMAT_R32G32B32A32_UINT || ImagePointer->Format == VK_FORMAT_R32G32B32_UINT || ImagePointer->Format == VK_FORMAT_R32G32_UINT || ImagePointer->Format == VK_FORMAT_R32_UINT)
	{
		Index = TexturesUint.size();
		TexturesUint.push_back(ImagePointer);
		DescriptorImageInfosUint[Index].imageLayout = ImageLayout;
		DescriptorImageInfosUint[Index].imageView = ImagePointer->View;
		DescriptorImageInfosUint[Index].sampler = VK_NULL_HANDLE;
		Index += MAX_TEXTURES;
	}

	if (ImagePointer->Format == VK_FORMAT_R32_SINT || ImagePointer->Format == VK_FORMAT_R32G32_SINT || ImagePointer->Format == VK_FORMAT_R32G32B32_SINT || ImagePointer->Format == VK_FORMAT_R32G32B32A32_SINT)
	{
		Index = TexturesInt.size();
		TexturesInt.push_back(ImagePointer);
		DescriptorImageInfosInt[Index].imageLayout = ImageLayout;
		DescriptorImageInfosInt[Index].imageView = ImagePointer->View;
		DescriptorImageInfosInt[Index].sampler = VK_NULL_HANDLE;
		Index += MAX_TEXTURES + MAX_TEXTURES;
	}

    TextureNameToIndexMap[Name] = Index;

    return Index;
}

void FTextureManager::RegisterIBL(const ImagePtr& ImagePointer)
{
	IBLImage = ImagePointer;
}

ImagePtr FTextureManager::GetIBLImage()
{
	return IBLImage;
}

void FTextureManager::RegisterFramebuffer(const ImagePtr& ImagePointer, const std::string& Name)
{
	if (FramebufferNameToImageMap.find(Name) != FramebufferNameToImageMap.end())
	{
		throw std::runtime_error("Trying to register already registered framebuffer!");
	}

	FramebufferNameToImageMap[Name] = ImagePointer;
}

void FTextureManager::UnregisterAndFreeFramebuffer(const std::string& Name)
{
	if (FramebufferNameToImageMap.find(Name) == FramebufferNameToImageMap.end())
	{
		throw std::runtime_error("Trying to unregister framebuffer that is not registered!");
	}

	FramebufferNameToImageMap.erase(Name);
}

ImagePtr FTextureManager::GetTexture(uint32_t TextureIndex)
{
	if (TextureIndex < MAX_TEXTURES)
	{
		return TexturesFloat[TextureIndex];
	}
	if (TextureIndex < (MAX_TEXTURES + MAX_TEXTURES))
	{
		return TexturesUint[TextureIndex];
	}
	if (TextureIndex < (MAX_TEXTURES + MAX_TEXTURES + MAX_TEXTURES))
	{
		return TexturesInt[TextureIndex];
	}

	throw std::runtime_error("Trying to get texture that is out of texture range!");
}

ImagePtr FTextureManager::GetTexture(const std::string& Name)
{
    return GetTexture(TextureNameToIndexMap[Name]);
}

ImagePtr FTextureManager::GetFramebufferImage(const std::string& Name)
{
    return FramebufferNameToImageMap[Name];
}

VkDescriptorImageInfo* FTextureManager::GetDescriptorImageInfosFloat()
{
    return DescriptorImageInfosFloat.data();
}

VkDescriptorImageInfo* FTextureManager::GetDescriptorImageInfosUint()
{
	return DescriptorImageInfosUint.data();
}

VkDescriptorImageInfo* FTextureManager::GetDescriptorImageInfosInt()
{
	return DescriptorImageInfosInt.data();
}