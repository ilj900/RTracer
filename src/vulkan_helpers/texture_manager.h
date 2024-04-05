#pragma once

#include "image.h"
#include "common_defines.h"

#include <vector>

class FTextureManager
{
public:
    FTextureManager();
    ~FTextureManager();

    uint32_t RegiseterTexture(ImagePtr ImagePointer, VkImageLayout ImageLayout);
    ImagePtr GetTexture(uint32_t TextureIndex);
    VkDescriptorImageInfo* GetDescriptorImageInfos();

private:
    std::vector<ImagePtr> Images;
    std::vector<VkDescriptorImageInfo> DescriptorImageInfos;
    ImagePtr DummyImage = nullptr;
};

FTextureManager* GetTextureManager();
void FreeTextureManager();