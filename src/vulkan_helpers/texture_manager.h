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

    ImagePtr CreateStorageImage(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName = "");
	ImagePtr CreateClearableStorageImage(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName = "");
    ImagePtr CreateSampledStorageImage(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName = "");
	ImagePtr CreateColorAttachment(uint32_t WidthIn, uint32_t HeightIn, const std::string& DebugName = "");
    uint32_t RegisterTexture(const ImagePtr& ImagePointer, VkImageLayout ImageLayout, const std::string& Name);
    void RegisterFramebuffer(const ImagePtr& ImagePointer, const std::string& Name);
	void UnregisterAndFreeFramebuffer(const std::string& Name);
    ImagePtr GetTexture(uint32_t TextureIndex);
    ImagePtr GetTexture(const std::string& Name);
    ImagePtr GetFramebufferImage(const std::string& Name);
    VkDescriptorImageInfo* GetDescriptorImageInfos();

private:
    std::vector<ImagePtr> Textures;
    std::vector<VkDescriptorImageInfo> DescriptorImageInfos;
    ImagePtr DummyImage = nullptr;
    std::unordered_map<std::string, uint32_t> TextureNameToIndexMap;

    std::unordered_map<std::string, ImagePtr> FramebufferNameToImageMap;
};

FTextureManager* GetTextureManager();
void FreeTextureManager();

#define TEXTURE_MANAGER() GetTextureManager()
#define FREE_TEXTURE_MANAGER() FreeTextureManager()