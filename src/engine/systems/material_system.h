#pragma once

#include "buffer.h"

#include "gpu_bufferable_system.h"
#include "coordinator.h"
#include "common_defines.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FMaterialSystem : public FSystem
        {
        public:
            FEntity CreateMaterial();
            FMaterialSystem& SetBaseColor(FEntity MaterialEntity, float Red, float Green, float Blue);
            FMaterialSystem& SetBaseColor(FEntity MaterialEntity, FEntity TextureEntity);
            FMaterialSystem& SetDiffuseRoughness(FEntity MaterialEntity, float DiffuseRoughness);
            FMaterialSystem& SetDiffuseRoughness(FEntity MaterialEntity, FEntity TextureEntity);
            FMaterialSystem& SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR);
            FMaterialSystem& SetSpecularIOR(FEntity MaterialEntity, FEntity TextureEntity);
            std::string GenerateMaterialCode(FEntity MaterialEntity);

            const uint32_t MAX_MATERIALS = TOTAL_MATERIALS - 1;
        };
    }
}

#define MATERIAL_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMaterialSystem>()
