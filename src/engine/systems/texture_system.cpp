#include "texture_system.h"
#include "texture_component.h"
#include "string_manipulation.h"
#include "vk_context.h"
#include "texture_manager.h"

namespace ECS
{
    namespace SYSTEMS
    {
        FEntity FTextureSystem::CreateTextureFromFile(const std::string& FilePath)
        {
            auto FileName = ExtractFileName(FilePath);
            auto Image = VK_CONTEXT()->LoadImageFromFile(FilePath, FileName);
            uint32_t ImageIndex = TEXTURE_MANAGER()->RegisterTexture(Image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, FileName);

            FEntity Texture = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FTextureComponent>(Texture, {ImageIndex});
            return Texture;
        }

    	FEntity FTextureSystem::CreateTextureFromData(const std::vector<unsigned char>& Data, int Width, int Height, int NumberOfChannels, const std::string& DebugImageName)
        {
        	static int i = 0;
        	std::string ImageUniqueName = "Image_from_data_" + std::to_string(i);
        	++i;
        	auto Image = VK_CONTEXT()->LoadImageFromMemory(Data, Width, Height, NumberOfChannels, DebugImageName);
        	uint32_t ImageIndex = TEXTURE_MANAGER()->RegisterTexture(Image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, ImageUniqueName);

        	FEntity Texture = COORDINATOR().CreateEntity();
        	COORDINATOR().AddComponent<ECS::COMPONENTS::FTextureComponent>(Texture, {ImageIndex});
        	return Texture;
        }
    }
}