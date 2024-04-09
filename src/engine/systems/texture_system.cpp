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
    }
}