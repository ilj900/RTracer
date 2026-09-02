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
        	void Init();
        	bool Update();
            FEntity			 CreateDefaultMaterial(const std::string& MaterialName = "");
			FEntity			 CreateEmptyMaterial(const std::string& MaterialName = "");

        	FMaterialSystem& SetF(FEntity MaterialEntity, uint32_t MaterialProperty, float Property);
        	FMaterialSystem& Set3F(FEntity MaterialEntity, uint32_t MaterialProperty, FVector3 Property);
        	FMaterialSystem& SetTexture(FEntity MaterialEntity, uint32_t MaterialProperty, FEntity TextureEntity);

			std::string GenerateMaterialCode(FEntity MaterialEntity);
			std::string GenerateEmissiveMaterialsCode(const std::unordered_map<uint32_t , uint32_t>& EmissiveMaterials);

            const uint32_t MAX_MATERIALS = IBL_MATERIAL_INDEX;

        	std::unordered_map<FEntity, uint32_t> MaterialToIndexMap;
        	std::queue<uint32_t> FreeIndices;
        	std::set<FEntity> ChangedMaterials;
        	std::unordered_map<FEntity, std::string> MaterialToName;
        };
    }
}

#define MATERIAL_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMaterialSystem>()
