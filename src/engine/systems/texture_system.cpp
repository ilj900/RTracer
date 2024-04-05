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
            auto& Coordinator = GetCoordinator();
            FEntity Texture = Coordinator.CreateEntity();
            auto Image = GetContext().LoadImageFromFile(FilePath, ExtractFileName(FilePath));
            uint32_t ImageIndex = GetTextureManager()->RegiseterTexture(Image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            Coordinator.AddComponent<ECS::COMPONENTS::FTextureComponent>(Texture, {ImageIndex});
            return Texture;
        }
    }
}