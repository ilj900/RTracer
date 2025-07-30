#include "material_component.h"
#include "material_system.h"
#include "named_resources.h"
#include "texture_component.h"

#include "texture_manager.h"

#include <typeindex>
#include <type_traits>

namespace ECS
{
    namespace SYSTEMS
    {
    	void FMaterialSystem::Init()
    	{
    		FBuffer MaterialsBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(MAX_MATERIALS * sizeof(COMPONENTS::FMaterialComponent),
					VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MATERIAL_SYSTEM_DATA_BUFFER);
    		RESOURCE_ALLOCATOR()->RegisterBuffer(MaterialsBuffer, MATERIAL_SYSTEM_DATA_BUFFER);

    		for (int i = 0; i < MAX_MATERIALS; i++)
    		{
    			FreeIndices.push(i);
    		}
    	}

    	bool FMaterialSystem::Update()
    	{
    		/// TODO: Optimize loading
    		if (!ChangedMaterials.empty())
    		{
    			std::vector<COMPONENTS::FMaterialComponent> UpdatedMaterials(ChangedMaterials.size());
    			std::vector<VkDeviceSize> Sizes(ChangedMaterials.size());
    			std::vector<VkDeviceSize> Offsets(ChangedMaterials.size());
    			std::vector<void*> DataPointers(ChangedMaterials.size());

    			int i = 0;

    			for (auto Entity : ChangedMaterials)
    			{
    				Sizes[i] = sizeof(COMPONENTS::FMaterialComponent);
    				UpdatedMaterials[i] = COORDINATOR().GetComponent<COMPONENTS::FMaterialComponent>(Entity);
    				Offsets[i] = sizeof(COMPONENTS::FMaterialComponent) * MaterialToIndexMap[Entity];;
    				DataPointers[i] = &UpdatedMaterials[i];
    				++i;
    			}

    			RESOURCE_ALLOCATOR()->LoadDataToBuffer(MATERIAL_SYSTEM_DATA_BUFFER, Sizes, Offsets, DataPointers);
    			ChangedMaterials.clear();
    			return true;
    		}

    		return false;
    	}

        FEntity FMaterialSystem::CreateDefaultMaterial()
        {
            FEntity Material = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FMaterialComponent>(Material, {});
    		MaterialToIndexMap[Material] = FreeIndices.front();
    		FreeIndices.pop();
            return Material;
        }

		FEntity FMaterialSystem::CreateEmptyMaterial()
		{
			FEntity Material = COORDINATOR().CreateEntity();
			COORDINATOR().AddComponent<ECS::COMPONENTS::FMaterialComponent>(Material, {});
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(Material);

			MaterialComponent.BaseWeight = 0.f;
			MaterialComponent.BaseColor = {0.f, 0.f, 0.f};
			MaterialComponent.DiffuseRoughness = 0.f;
			MaterialComponent.Metalness = 0.f;
			MaterialComponent.Normal = {0.f, 0.f, 0.f};
			MaterialComponent.SpecularWeight = 0.f;
			MaterialComponent.SpecularColor = {0.f, 0.f, 0.f};
			MaterialComponent.SpecularRoughness = 0.f;
			MaterialComponent.SpecularIOR = 0.f;
			MaterialComponent.SpecularAnisotropy = 0.f;
			MaterialComponent.SpecularRotation = 0.f;
			MaterialComponent.TransmissionWeight = 0.f;
			MaterialComponent.TransmissionColor = {0.f, 0.f, 0.f};
			MaterialComponent.TransmissionDepth = 0.f;
			MaterialComponent.TransmissionScatter = {0.f, 0.f, 0.f};
			MaterialComponent.TransmissionAnisotropy = 0.f;
			MaterialComponent.TransmissionDispersion = 0.f;
			MaterialComponent.TransmissionRoughness = 0.f;
			MaterialComponent.SubsurfaceWeight = 0.f;
			MaterialComponent.SubsurfaceColor = {0.f, 0.f, 0.f};
			MaterialComponent.SubsurfaceRadius = {0.f, 0.f, 0.f};
			MaterialComponent.SubsurfaceScale = 0.f;
			MaterialComponent.SubsurfaceAnisotropy = 0.f;
			MaterialComponent.SheenWeight = 0.f;
			MaterialComponent.SheenColor = {0.f, 0.f, 0.f};
			MaterialComponent.SheenRoughness = 0.f;
			MaterialComponent.CoatWeight = 0.f;
			MaterialComponent.CoatColor = {0.f, 0.f, 0.f};
			MaterialComponent.CoatRoughness = 0.f;
			MaterialComponent.CoatAnisotropy = 0.f;
			MaterialComponent.CoatRotation = 0.f;
			MaterialComponent.CoatIOR = 0.f;
			MaterialComponent.CoatNormal = {0, 0.f, 0};
			MaterialComponent.CoatAffectColor = 0.f;
			MaterialComponent.CoatAffectRoughness = 0.f;
			MaterialComponent.ThinFilmThickness = 0.f;
			MaterialComponent.ThinFilmIOR = 0.f;
			MaterialComponent.EmissionWeight = 0;
			MaterialComponent.EmissionColor = {0.f, 0.f, 0.f};
			MaterialComponent.Opacity = {0.f, 0.f, 0.f};
			MaterialComponent.ThinWalled = false;

    		MaterialToIndexMap[Material] = FreeIndices.front();
    		FreeIndices.pop();

			return Material;
		}

		/// Albedo
		FMaterialSystem& FMaterialSystem::SetBaseColorWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.BaseWeight = Weight;
    		ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColorWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.BaseWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.BaseColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.BaseColorTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.DiffuseRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughnessTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetMetalness(FEntity MaterialEntity, float Metalness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.Metalness = Metalness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetMetalness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.MetalnessTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetAlbedoNormal(FEntity MaterialEntity, const FVector3& Normal)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.Normal = Normal;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetAlbedoNormal(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.NormalTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Specular
		FMaterialSystem& FMaterialSystem::SetSpecularWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularColorTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularRoughnessTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularIOR = SpecularIOR;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularIORTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularAnisotropyTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRotation(FEntity MaterialEntity, float Rotation)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularRotation = Rotation;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRotation(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularRotationTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}


		/// Transmission
		FMaterialSystem& FMaterialSystem::SetTransmissionWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDepth(FEntity MaterialEntity, float Depth)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionDepth = Depth;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDepth(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionScatter(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionScatter = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionScatter(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDispersion(FEntity MaterialEntity, float Dispersion)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionDispersion = Dispersion;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDispersion(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionRoughnessTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}


		/// Subsurface
		FMaterialSystem& FMaterialSystem::SetSubsurfaceWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceRadius(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceRadius = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceRadius(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceScale(FEntity MaterialEntity, float Scale)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceScale = Scale;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceScale(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Sheen
		FMaterialSystem& FMaterialSystem::SetSheenWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SheenWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SheenWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SheenColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SheenRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Coat
		FMaterialSystem& FMaterialSystem::SetCoatWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatColorTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatRoughnessTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAnisotropyTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRotation(FEntity MaterialEntity, float Rotation)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatRotation = Rotation;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRotation(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatRotationTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatIOR(FEntity MaterialEntity, float CoatIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatIOR = CoatIOR;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatIORTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatNormal(FEntity MaterialEntity, FVector3 Normal)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatNormal = Normal;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatNormal(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatNormalTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectColor(FEntity MaterialEntity, float Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatAffectColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAffectColorTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatAffectRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAffectRoughnessTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Thin film
		FMaterialSystem& FMaterialSystem::SetThinFilmThickness(FEntity MaterialEntity, float Thickness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.ThinFilmThickness = Thickness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmThickness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinFilmThicknessTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmIOR(FEntity MaterialEntity, float ThinFilmIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.ThinFilmIOR = ThinFilmIOR;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinFilmIORTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}

		/// Emission
		FMaterialSystem& FMaterialSystem::SetEmissionWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.EmissionWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.EmissionWeightTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.EmissionColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.EmissionColorTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Opacity
		FMaterialSystem& FMaterialSystem::SetOpacity(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.Opacity = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetOpacity(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.OpacityTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Thin walled
		FMaterialSystem& FMaterialSystem::SetThinWalled(FEntity MaterialEntity, bool ThinWalled)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.ThinWalled = ThinWalled;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetThinWalled(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinWalledTexture = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

    	template <typename T> struct TypeToString;
    	template <> struct TypeToString<float> {static std::string Name() { return "SampleFloat"; }};
    	template <> struct TypeToString<uint32_t> {static std::string Name() { return "SampleUint"; }};
    	template <> struct TypeToString<int> {static std::string Name() { return "SampleInt"; }};
    	template <> struct TypeToString<FVector2> {static std::string Name() { return "SampleVec2"; }};
    	template <> struct TypeToString<FVector3> {static std::string Name() { return "SampleVec3"; }};
    	template <> struct TypeToString<FVector4> {static std::string Name() { return "SampleVec4"; }};

    	template <typename T>
    	std::string GenerateGetFunction(const std::string& ParameterName, uint32_t MaterialIndex, uint32_t DataOffset, uint32_t TextureOffset, uint32_t TextureIndex, bool bConvertToLinear = false)
    	{
    		assert((DataOffset % 4) == 0);
    		assert((TextureOffset % 4) == 0);
    		DataOffset = DataOffset / 4;
    		TextureOffset = TextureOffset / 4;
    		std::string Result = "    " + ParameterName;

    		if (UINT32_MAX == TextureIndex)
    		{
    			Result +=  " = " + TypeToString<T>::Name() + "(" + std::to_string(MaterialIndex) + ", " + std::to_string(DataOffset) + ");\r\n";
    		}
    		else
    		{
    			/// If we sample a texture, in some cases it might be in sRGB format, and we need to translate it to linear color space
    			if (bConvertToLinear)
    			{
    				Result += " = SRGBToLinear(" + TypeToString<T> ::Name()+ "(" + std::to_string(MaterialIndex) + ", " + std::to_string(TextureOffset) + ", TextureCoords));\r\n";
    			}
    			else
    			{
    				Result += " = " + TypeToString<T> ::Name()+ "(" + std::to_string(MaterialIndex) + ", " + std::to_string(TextureOffset) + ", TextureCoords);\r\n";
    			}
    		}

    		return Result;
    	}

        std::string FMaterialSystem::GenerateMaterialCode(FEntity MaterialEntity)
        {
            std::string Result;

            auto& MaterialComponent = GetComponent<COMPONENTS::FMaterialComponent>(MaterialEntity);
    		uint32_t MaterialIndex = MaterialToIndexMap[MaterialEntity];

            Result += "FDeviceMaterial GetMaterial(vec2 TextureCoords)\r\n";
            Result += "{\r\n";
            Result += "    FDeviceMaterial Material;\r\n";

            Result += GenerateGetFunction<float>(	"Material.BaseWeight",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, BaseWeight),				offsetof(COMPONENTS::FMaterialComponent, BaseWeightTexture),				MaterialComponent.BaseWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.BaseColor",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, BaseColor),				offsetof(COMPONENTS::FMaterialComponent, BaseColorTexture),					MaterialComponent.BaseColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.DiffuseRoughness",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, DiffuseRoughness),			offsetof(COMPONENTS::FMaterialComponent, DiffuseRoughnessTexture),			MaterialComponent.DiffuseRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.Metalness",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, Metalness),				offsetof(COMPONENTS::FMaterialComponent, MetalnessTexture),					MaterialComponent.MetalnessTexture);
            Result += GenerateGetFunction<FVector3>("Material.Normal",					MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, Normal),					offsetof(COMPONENTS::FMaterialComponent, NormalTexture),					MaterialComponent.NormalTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularWeight",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SpecularWeight),			offsetof(COMPONENTS::FMaterialComponent, SpecularWeightTexture),			MaterialComponent.SpecularWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.SpecularColor",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SpecularColor),			offsetof(COMPONENTS::FMaterialComponent, SpecularColorTexture),				MaterialComponent.SpecularColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.SpecularRoughness",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SpecularRoughness),		offsetof(COMPONENTS::FMaterialComponent, SpecularRoughnessTexture),			MaterialComponent.SpecularRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularIOR",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SpecularIOR),				offsetof(COMPONENTS::FMaterialComponent, SpecularIORTexture),				MaterialComponent.SpecularIORTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularAnisotropy",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SpecularAnisotropy),		offsetof(COMPONENTS::FMaterialComponent, SpecularAnisotropyTexture),		MaterialComponent.SpecularAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularRotation",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SpecularRotation),			offsetof(COMPONENTS::FMaterialComponent, SpecularRotationTexture),			MaterialComponent.SpecularRotationTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionWeight",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, TransmissionWeight),		offsetof(COMPONENTS::FMaterialComponent, TransmissionWeightTexture),		MaterialComponent.TransmissionWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.TransmissionColor",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, TransmissionColor),		offsetof(COMPONENTS::FMaterialComponent, TransmissionColorTexture),			MaterialComponent.TransmissionColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.TransmissionDepth",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, TransmissionDepth),		offsetof(COMPONENTS::FMaterialComponent, TransmissionDepthTexture),			MaterialComponent.TransmissionDepthTexture);
            Result += GenerateGetFunction<FVector3>("Material.TransmissionScatter",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, TransmissionScatter),		offsetof(COMPONENTS::FMaterialComponent, TransmissionScatterTexture),		MaterialComponent.TransmissionScatterTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionAnisotropy",	MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, TransmissionAnisotropy),	offsetof(COMPONENTS::FMaterialComponent, TransmissionAnisotropyTexture),	MaterialComponent.TransmissionAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionDispersion",	MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, TransmissionDispersion),	offsetof(COMPONENTS::FMaterialComponent, TransmissionDispersionTexture),	MaterialComponent.TransmissionDispersionTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionRoughness",	MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, TransmissionRoughness),	offsetof(COMPONENTS::FMaterialComponent, TransmissionRoughnessTexture),		MaterialComponent.TransmissionRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.SubsurfaceWeight",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SubsurfaceWeight),			offsetof(COMPONENTS::FMaterialComponent, SubsurfaceWeightTexture),			MaterialComponent.SubsurfaceWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.SubsurfaceColor",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SubsurfaceColor),			offsetof(COMPONENTS::FMaterialComponent, SubsurfaceColorTexture),			MaterialComponent.SubsurfaceColorTexture, true);
            Result += GenerateGetFunction<FVector3>("Material.SubsurfaceRadius",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SubsurfaceRadius),			offsetof(COMPONENTS::FMaterialComponent, SubsurfaceRadiusTexture),			MaterialComponent.SubsurfaceRadiusTexture);
            Result += GenerateGetFunction<float>(	"Material.SubsurfaceScale",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SubsurfaceScale),			offsetof(COMPONENTS::FMaterialComponent, SubsurfaceScaleTexture),			MaterialComponent.SubsurfaceScaleTexture);
            Result += GenerateGetFunction<float>(	"Material.SubsurfaceAnisotropy",	MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SubsurfaceAnisotropy),		offsetof(COMPONENTS::FMaterialComponent, SubsurfaceAnisotropyTexture),		MaterialComponent.SubsurfaceAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.SheenWeight",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SheenWeight),				offsetof(COMPONENTS::FMaterialComponent, SheenWeightTexture),				MaterialComponent.SheenWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.SheenColor",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SheenColor),				offsetof(COMPONENTS::FMaterialComponent, SheenColorTexture),				MaterialComponent.SheenColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.SheenRoughness",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, SheenRoughness),			offsetof(COMPONENTS::FMaterialComponent, SheenRoughnessTexture),			MaterialComponent.SheenRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatWeight",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatWeight),				offsetof(COMPONENTS::FMaterialComponent, CoatWeightTexture),				MaterialComponent.CoatWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.CoatColor",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatColor),				offsetof(COMPONENTS::FMaterialComponent, CoatColorTexture),					MaterialComponent.CoatColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.CoatRoughness",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatRoughness),			offsetof(COMPONENTS::FMaterialComponent, CoatRoughnessTexture),				MaterialComponent.CoatRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatAnisotropy",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatAnisotropy),			offsetof(COMPONENTS::FMaterialComponent, CoatAnisotropyTexture),			MaterialComponent.CoatAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatRotation",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatRotation),				offsetof(COMPONENTS::FMaterialComponent, CoatRotationTexture),				MaterialComponent.CoatRotationTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatIOR",					MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatIOR),					offsetof(COMPONENTS::FMaterialComponent, CoatIORTexture),					MaterialComponent.CoatIORTexture);
            Result += GenerateGetFunction<FVector3>("Material.CoatNormal",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatNormal),				offsetof(COMPONENTS::FMaterialComponent, CoatNormalTexture),				MaterialComponent.CoatNormalTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatAffectColor",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatAffectColor),			offsetof(COMPONENTS::FMaterialComponent, CoatAffectColorTexture),			MaterialComponent.CoatAffectColorTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatAffectRoughness",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, CoatAffectRoughness),		offsetof(COMPONENTS::FMaterialComponent, CoatAffectRoughnessTexture),		MaterialComponent.CoatAffectRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.ThinFilmThickness",		MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, ThinFilmThickness),		offsetof(COMPONENTS::FMaterialComponent, ThinFilmThicknessTexture),			MaterialComponent.ThinFilmThicknessTexture);
            Result += GenerateGetFunction<float>(	"Material.ThinFilmIOR",				MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, ThinFilmIOR),				offsetof(COMPONENTS::FMaterialComponent, ThinFilmIORTexture),				MaterialComponent.ThinFilmIORTexture);
            Result += GenerateGetFunction<float>(	"Material.EmissionWeight",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, EmissionWeight),			offsetof(COMPONENTS::FMaterialComponent, EmissionWeightTexture),			MaterialComponent.EmissionWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.EmissionColor",			MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, EmissionColor),			offsetof(COMPONENTS::FMaterialComponent, EmissionColorTexture),				MaterialComponent.EmissionColorTexture, true);
            Result += GenerateGetFunction<FVector3>("Material.Opacity",					MaterialIndex, offsetof(COMPONENTS::FMaterialComponent, Opacity),					offsetof(COMPONENTS::FMaterialComponent, OpacityTexture),					MaterialComponent.OpacityTexture);
            //TODO: Add uint textures
            Result += "    Material.ThinWalled = " + std::to_string(MaterialComponent.ThinWalled) + ";\r\n";

            Result += "    return Material;\r\n";
            Result += "};\r\n";

            return Result;
        }

		std::string FMaterialSystem::GenerateEmissiveMaterialsCode(const std::unordered_map<uint32_t , uint32_t>& EmissiveMaterials)
		{
			std::string Result;

			if (EmissiveMaterials.empty())
			{
				Result += "FDeviceMaterial GetEmissiveMaterial(vec2 TextureCoords, uint MaterialIndex)\r\n";
				Result += "{\r\n";
				Result += "    FDeviceMaterial Material;\r\n";
				Result += "    return Material;\r\n";
				Result += "};\r\n";
				return Result;
			}

			Result += "FDeviceMaterial GetEmissiveMaterial(vec2 TextureCoords, uint MaterialIndex)\r\n";
			Result += "{\r\n";
			Result += "    FDeviceMaterial Material;\r\n";
			/// We need to initialize it to 0, so that it would work as a mark whether material is emissive or not
			Result += "    Material.EmissionWeight = 0;\r\n";
			Result += "    switch(MaterialIndex)\r\n";
			Result += "    {\r\n";
			for (auto& Entry : EmissiveMaterials)
			{
				auto& MaterialComponent = GetComponentByIndex<ECS::COMPONENTS::FMaterialComponent>(Entry.first);
				Result += "    case " + std::to_string(Entry.first) + ":\r\n";
				Result += "    {\r\n";
				/// TODO: Fix the indexing
				Result += "    " + GenerateGetFunction<FVector3>("Material.EmissionColor", 0, offsetof(COMPONENTS::FMaterialComponent, EmissionColor), offsetof(COMPONENTS::FMaterialComponent, EmissionColorTexture), MaterialComponent.EmissionColorTexture);
				Result += "    " + GenerateGetFunction<float>("Material.EmissionWeight", 0, offsetof(COMPONENTS::FMaterialComponent, EmissionWeight), offsetof(COMPONENTS::FMaterialComponent, EmissionWeightTexture), MaterialComponent.EmissionWeightTexture);
				Result += "    break;\r\n";
				Result += "    }\r\n";
			}
			Result += "    };\r\n";

			Result += "    return Material;\r\n";
			Result += "};\r\n";

			return Result;
		}
    }
}