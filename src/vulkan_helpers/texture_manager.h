#pragma once

#include "image.h"
#include "common_defines.h"

#include <unordered_map>
#include <vector>

class FTextureManager
{
public:
    FTextureManager();
    ~FTextureManager();

    ImagePtr CreateStorageImage(uint32_t WidthIn, uint32_t HeightIn, VkFormat Format = VK_FORMAT_R32G32B32A32_SFLOAT, const std::string& DebugName = "");
	ImagePtr CreateClearableStorageImage(uint32_t WidthIn, uint32_t HeightIn, VkFormat Format = VK_FORMAT_R32G32B32A32_SFLOAT, const std::string& DebugName = "");
    ImagePtr CreateSampledStorageImage(uint32_t WidthIn, uint32_t HeightIn, VkFormat Format = VK_FORMAT_R32G32B32A32_SFLOAT, const std::string& DebugName = "");
	ImagePtr CreateColorAttachment(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName = "");
    uint32_t RegisterTexture(const ImagePtr& ImagePointer, VkImageLayout ImageLayout, const std::string& Name);
    void RegisterFramebuffer(const ImagePtr& ImagePointer, const std::string& Name);
	void UnregisterAndFreeFramebuffer(const std::string& Name);
    ImagePtr GetTexture(uint32_t TextureIndex);
    ImagePtr GetTexture(const std::string& Name);
    ImagePtr GetFramebufferImage(const std::string& Name);
    VkDescriptorImageInfo* GetDescriptorImageInfosFloat();
	VkDescriptorImageInfo* GetDescriptorImageInfosUint();
	VkDescriptorImageInfo* GetDescriptorImageInfosInt();
	void RegisterIBL(const ImagePtr& ImagePointer);
	ImagePtr GetIBLImage();

private:
    std::vector<ImagePtr> TexturesFloat;
	std::vector<ImagePtr> TexturesUint;
	std::vector<ImagePtr> TexturesInt;
    std::vector<VkDescriptorImageInfo> DescriptorImageInfosFloat;
	std::vector<VkDescriptorImageInfo> DescriptorImageInfosUint;
	std::vector<VkDescriptorImageInfo> DescriptorImageInfosInt;
    ImagePtr DummyImageFloat = nullptr;
	ImagePtr DummyImageUint = nullptr;
	ImagePtr DummyImageInt = nullptr;
	ImagePtr IBLImage = nullptr;
    std::unordered_map<std::string, uint32_t> TextureNameToIndexMap;

    std::unordered_map<std::string, ImagePtr> FramebufferNameToImageMap;
};

FTextureManager* GetTextureManager();
void FreeTextureManager();

#define TEXTURE_MANAGER() GetTextureManager()
#define FREE_TEXTURE_MANAGER() FreeTextureManager()