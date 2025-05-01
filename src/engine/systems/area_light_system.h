#pragma once

#include "buffer.h"

#include "gpu_bufferable_system.h"
#include "coordinator.h"

#include "maths.h"

#include "common_structures.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FAreaLightSystem : public FGPUBufferableSystem
        {
        public:
            void Init(uint32_t NumberOfSimultaneousSubmits) override;
            bool Update() override;
            bool Update(int Index) override;

			FEntity CreateAreaLightInstance(FEntity InstanceEntity);
			const std::unordered_map<uint32_t, uint32_t>& GetEmissiveMaterials();

            static const uint32_t MAX_AREA_LIGHTS = 512;
			uint32_t LoadedAreaLightsCount = 0;
			float LoadedAreaLightArea = 0.f;
			uint32_t CurrentAreaLightsCount = 0;
			float CurrentAreaLightArea = 0.f;
			const uint32_t MAX_EMISSIVE_MATERIALS = 8;
			bool bAreaLightAddressTableShouldBeUpdated = false;

		private:
			std::unordered_map<uint32_t, uint32_t> EmissiveMaterialsCount;
        };
    }
}

#define AREA_LIGHT_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FAreaLightSystem>()
