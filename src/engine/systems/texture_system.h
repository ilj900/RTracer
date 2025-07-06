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
        	FEntity CreateTextureFromData(const std::vector<unsigned char>& Data, int Width, int Height, int NumberOfChannels, const std::string& DebugImageName = "");
        };
    }
}

#define TEXTURE_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FTextureSystem>()
