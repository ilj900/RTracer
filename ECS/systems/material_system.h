#pragma once

#include "buffer.h"

#include "gpu_bufferable_system.h"
#include "coordinator.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FMaterialSystem : public FGPUBufferableSystem
        {
        public:
            void Init(int NumberOfSimultaneousSubmits) override;
            void Update() override;
            void Update(int Index) override;

            FMaterialSystem& SetBaseColor(FEntity MaterialEntity, float Red, float Green, float Blue);
            FMaterialSystem& SetDiffuseRoughness(FEntity MaterialEntity, float DiffuseRoughness);
            FMaterialSystem& SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR);

            const uint32_t MAX_MATERIALS = 32;
        };
    }
}

#define MATERIAL_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMaterialSystem>()
