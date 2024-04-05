#pragma once

#include "buffer.h"

#include "coordinator.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FTextureSystem : public FSystem
        {
        public:
            FEntity CreateTextureFromFile(const std::string& FilePath);
        };
    }
}

#define TEXTURE_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTextureSystem>()
