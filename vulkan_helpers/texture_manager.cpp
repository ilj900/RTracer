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

FTextureManager::FTextureManager()
{
    auto& Context = GetContext();

    DummyImage = Context.CreateImage2D(1, 1, false, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
                                       VK_IMAGE_USAGE_STORAGE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                       VK_IMAGE_ASPECT_COLOR_BIT, Context.GetLogicalDevice(), "V_Dummy_Image");

    Sampler = Context.CreateTextureSampler(VK_SAMPLE_COUNT_1_BIT);

    /// Populate all image views with dummy image view
    DescriptorImageInfos.resize(MAX_TEXTURES);

    for (int i = 0; i < MAX_TEXTURES; ++i)
    {
        DescriptorImageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        DescriptorImageInfos[i].imageView = DummyImage->View;
        DescriptorImageInfos[i].sampler = Sampler;
    }
}

FTextureManager::~FTextureManager()
{
    DummyImage = nullptr;
    auto& Context = GetContext();

    vkDestroySampler(Context.GetLogicalDevice(), Sampler, nullptr);
}

int FTextureManager::RegiseterTexture(ImagePtr ImagePointer, VkImageLayout ImageLayout, VkSampler Sampler)
{
    int Index = Images.size();
    Images.push_back(ImagePointer);
    DescriptorImageInfos[Index].imageLayout = ImageLayout;
    DescriptorImageInfos[Index].imageView = ImagePointer->View;
    DescriptorImageInfos[Index].sampler = Sampler;

    return Index;
}

VkDescriptorImageInfo* FTextureManager::GetDescriptorImageInfos()
{
    return DescriptorImageInfos.data();
}