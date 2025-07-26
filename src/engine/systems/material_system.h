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
            FEntity			 CreateDefaultMaterial();
			FEntity			 CreateEmptyMaterial();
			/// Albedo
			FMaterialSystem& SetBaseColorWeight(FEntity MaterialEntity, float Weight);
			FMaterialSystem& SetBaseColorWeight(FEntity MaterialEntity, FEntity TextureEntity);
            FMaterialSystem& SetBaseColor(FEntity MaterialEntity, const FVector3& Color);
            FMaterialSystem& SetBaseColor(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetDiffuseRoughness(FEntity MaterialEntity, float Roughness);
			FMaterialSystem& SetDiffuseRoughness(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetMetalness(FEntity MaterialEntity, float Metalness);
			FMaterialSystem& SetMetalness(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetAlbedoNormal(FEntity MaterialEntity, FVector3 Normal);
			FMaterialSystem& SetAlbedoNormal(FEntity MaterialEntity, FEntity TextureEntity);

			/// Specular
			FMaterialSystem& SetSpecularWeight(FEntity MaterialEntity, float Weight);
			FMaterialSystem& SetSpecularWeight(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSpecularColor(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetSpecularColor(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSpecularRoughness(FEntity MaterialEntity, float Roughness);
			FMaterialSystem& SetSpecularRoughness(FEntity MaterialEntity, FEntity TextureEntity);
            FMaterialSystem& SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR);
            FMaterialSystem& SetSpecularIOR(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSpecularAnisotropy(FEntity MaterialEntity, float Anisotropy);
			FMaterialSystem& SetSpecularAnisotropy(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSpecularRotation(FEntity MaterialEntity, float Rotation);
			FMaterialSystem& SetSpecularRotation(FEntity MaterialEntity, FEntity TextureEntity);

			/// Transmission
			FMaterialSystem& SetTransmissionWeight(FEntity MaterialEntity, float Weight);
			FMaterialSystem& SetTransmissionWeight(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetTransmissionColor(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetTransmissionColor(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetTransmissionDepth(FEntity MaterialEntity, float Depth);
			FMaterialSystem& SetTransmissionDepth(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetTransmissionScatter(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetTransmissionScatter(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetTransmissionAnisotropy(FEntity MaterialEntity, float Anisotropy);
			FMaterialSystem& SetTransmissionAnisotropy(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetTransmissionDispersion(FEntity MaterialEntity, float Dispersion);
			FMaterialSystem& SetTransmissionDispersion(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetTransmissionRoughness(FEntity MaterialEntity, float Roughness);
			FMaterialSystem& SetTransmissionRoughness(FEntity MaterialEntity, FEntity TextureEntity);

			/// Subsurface
			FMaterialSystem& SetSubsurfaceWeight(FEntity MaterialEntity, float Weight);
			FMaterialSystem& SetSubsurfaceWeight(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSubsurfaceColor(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetSubsurfaceColor(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSubsurfaceRadius(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetSubsurfaceRadius(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSubsurfaceScale(FEntity MaterialEntity, float Scale);
			FMaterialSystem& SetSubsurfaceScale(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSubsurfaceAnisotropy(FEntity MaterialEntity, float Anisotropy);
			FMaterialSystem& SetSubsurfaceAnisotropy(FEntity MaterialEntity, FEntity TextureEntity);

			/// Sheen
			FMaterialSystem& SetSheenWeight(FEntity MaterialEntity, float Weight);
			FMaterialSystem& SetSheenWeight(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSheenColor(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetSheenColor(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetSheenRoughness(FEntity MaterialEntity, float Roughness);
			FMaterialSystem& SetSheenRoughness(FEntity MaterialEntity, FEntity TextureEntity);

			/// Coat
			FMaterialSystem& SetCoatWeight(FEntity MaterialEntity, float Weight);
			FMaterialSystem& SetCoatWeight(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatColor(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetCoatColor(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatRoughness(FEntity MaterialEntity, float Roughness);
			FMaterialSystem& SetCoatRoughness(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatAnisotropy(FEntity MaterialEntity, float Anisotropy);
			FMaterialSystem& SetCoatAnisotropy(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatRotation(FEntity MaterialEntity, float Rotation);
			FMaterialSystem& SetCoatRotation(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatIOR(FEntity MaterialEntity, float CoatIOR);
			FMaterialSystem& SetCoatIOR(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatNormal(FEntity MaterialEntity, FVector3 Normal);
			FMaterialSystem& SetCoatNormal(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatAffectColor(FEntity MaterialEntity, float Color);
			FMaterialSystem& SetCoatAffectColor(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetCoatAffectRoughness(FEntity MaterialEntity, float Roughness);
			FMaterialSystem& SetCoatAffectRoughness(FEntity MaterialEntity, FEntity TextureEntity);

			/// Thin film
			FMaterialSystem& SetThinFilmThickness(FEntity MaterialEntity, float Thickness);
			FMaterialSystem& SetThinFilmThickness(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetThinFilmIOR(FEntity MaterialEntity, float ThinFilmIOR);
			FMaterialSystem& SetThinFilmIOR(FEntity MaterialEntity, FEntity TextureEntity);

			/// Emission
			FMaterialSystem& SetEmissionWeight(FEntity MaterialEntity, float Weight);
			FMaterialSystem& SetEmissionWeight(FEntity MaterialEntity, FEntity TextureEntity);
			FMaterialSystem& SetEmissionColor(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetEmissionColor(FEntity MaterialEntity, FEntity TextureEntity);

			/// Opacity
			FMaterialSystem& SetOpacity(FEntity MaterialEntity, const FVector3& Color);
			FMaterialSystem& SetOpacity(FEntity MaterialEntity, FEntity TextureEntity);

			/// Thin walled
			FMaterialSystem& SetThinWalled(FEntity MaterialEntity, bool ThinWalled);
			FMaterialSystem& SetThinWalled(FEntity MaterialEntity, FEntity TextureEntity);

			std::string GenerateMaterialCode(FEntity MaterialEntity);
			std::string GenerateEmissiveMaterialsCode(const std::unordered_map<uint32_t , uint32_t>& EmissiveMaterials);

            const uint32_t MAX_MATERIALS = IBL_MATERIAL_INDEX;

        	std::unordered_map<FEntity, uint32_t> MaterialToIndexMap;
        	std::queue<uint32_t> FreeIndices;
        	std::set<FEntity> ChangedMaterials;
        };
    }
}

#define MATERIAL_SYSTEM() ECS::GetCoordinator().GetSystem<ECS::SYSTEMS::FMaterialSystem>()
