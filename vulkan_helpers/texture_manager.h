#pragma once

#include "image.h"
#include "common_defines.h"

#include <vector>

class FTextureManager
{
public:
    FTextureManager();
    ~FTextureManager();

    int RegiseterTexture(ImagePtr ImagePointer, VkImageLayout ImageLayout, VkSampler Sampler);
    VkDescriptorImageInfo* GetDescriptorImageInfos();

private:
    std::vector<ImagePtr> Images;
    std::vector<VkDescriptorImageInfo> DescriptorImageInfos;
    ImagePtr DummyImage = nullptr;
    VkSampler Sampler = VK_NULL_HANDLE;
};

FTextureManager* GetTextureManager();