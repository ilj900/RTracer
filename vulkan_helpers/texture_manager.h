#pragma once

#include "image.h"
#include "common_defines.h"

#include <vector>

class FTextureManager
{
public:
    FTextureManager();
    ~FTextureManager();

    int RegiseterTexture(ImagePtr ImagePointer, VkImageLayout ImageLayout);
    VkDescriptorImageInfo* GetDescriptorImageInfos();

private:
    std::vector<ImagePtr> Images;
    std::vector<VkDescriptorImageInfo> DescriptorImageInfos;
    ImagePtr DummyImage = nullptr;
};

FTextureManager* GetTextureManager();
void FreeTextureManager();