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
    		FBuffer MaterialsBuffer = RESOURCE_ALLOCATOR()->CreateBuffer(MAX_MATERIALS * sizeof(COMPONENTS::FOpenPBRSurfaceComponent),
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
    			std::vector<COMPONENTS::FOpenPBRSurfaceComponent> UpdatedMaterials(ChangedMaterials.size());
    			std::vector<VkDeviceSize> Sizes(ChangedMaterials.size());
    			std::vector<VkDeviceSize> Offsets(ChangedMaterials.size());
    			std::vector<void*> DataPointers(ChangedMaterials.size());

    			int i = 0;

    			for (auto Entity : ChangedMaterials)
    			{
    				Sizes[i] = sizeof(COMPONENTS::FOpenPBRSurfaceComponent);
    				UpdatedMaterials[i] = COORDINATOR().GetComponent<COMPONENTS::FOpenPBRSurfaceComponent>(Entity);
    				Offsets[i] = sizeof(COMPONENTS::FOpenPBRSurfaceComponent) * MaterialToIndexMap[Entity];;
    				DataPointers[i] = &UpdatedMaterials[i];
    				++i;
    			}

    			RESOURCE_ALLOCATOR()->LoadDataToBuffer(MATERIAL_SYSTEM_DATA_BUFFER, Sizes, Offsets, DataPointers);
    			ChangedMaterials.clear();
    			return true;
    		}

    		return false;
    	}

        FEntity FMaterialSystem::CreateDefaultMaterial(const std::string& MaterialName)
        {
            FEntity Material = COORDINATOR().CreateEntity();
    		MaterialToName[Material] = MaterialName.empty() ? "Material entity " + std::to_string(Material) : MaterialName;
            COORDINATOR().AddComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Material, {});
    		MaterialToIndexMap[Material] = FreeIndices.front();
    		FreeIndices.pop();
            return Material;
        }

		FEntity FMaterialSystem::CreateEmptyMaterial(const std::string& MaterialName)
		{
			FEntity Material = COORDINATOR().CreateEntity();
    		MaterialToName[Material] = MaterialName.empty() ? "Material entity " + std::to_string(Material) : MaterialName;
			COORDINATOR().AddComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Material, {});
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Material);

    		MaterialComponent.BaseWeight = 0;
    		MaterialComponent.BaseColor = {0.f, 0.f, 0.f};
    		MaterialComponent.BaseMetalness = 0;
    		MaterialComponent.BaseDiffuseRoughness = 0;
    		MaterialComponent.SpecularWeight = 0;
    		MaterialComponent.SpecularColor = {0.f, 0.f, 0.f};
    		MaterialComponent.SpecularRoughness = 0;
    		MaterialComponent.SpecularRoughnessAnisotropy = 0;
    		MaterialComponent.SpecularIor = 0;
    		MaterialComponent.TransmissionWeight = 0;
    		MaterialComponent.TransmissionColor = {0.f, 0.f, 0.f};
    		MaterialComponent.TransmissionDepth = 0;
    		MaterialComponent.TransmissionScatter = {0.f, 0.f, 0.f};
    		MaterialComponent.TransmissionScatterAnisotropy = 0;
    		MaterialComponent.TransmissionDispersionScale = 0;
    		MaterialComponent.TransmissionDispersionAbbeNumber = 0;
    		MaterialComponent.SubsurfaceWeight = 0;
    		MaterialComponent.SubsurfaceColor = {0.f, 0.f, 0.f};
    		MaterialComponent.SubsurfaceRadius = 0;
    		MaterialComponent.SubsurfaceRadiusScale = {0.f, 0.f, 0.f};
    		MaterialComponent.SubsurfaceScatterAnisotropy = 0;
    		MaterialComponent.CoatWeight = 0;
    		MaterialComponent.CoatColor = {0.f, 0.f, 0.f};
    		MaterialComponent.CoatRoughness = 0;
    		MaterialComponent.CoatRoughnessAnisotropy = 0;
    		MaterialComponent.CoatIor = 0;
    		MaterialComponent.CoatDarkening = 0;
    		MaterialComponent.FuzzWeight = 0;
    		MaterialComponent.FuzzColor = {0.f, 0.f, 0.f};
    		MaterialComponent.FuzzRoughness = 0;
    		MaterialComponent.emissionLuminance = 0;
    		MaterialComponent.emissionColor = {0.f, 0.f, 0.f};
    		MaterialComponent.ThinFilmWeight = 0;
    		MaterialComponent.ThinFilmThickness = 0;
    		MaterialComponent.ThinFilmIor = 0;
    		MaterialComponent.GeometryOpacity = 0;
    		MaterialComponent.GeometryThinWalled = false;
    		MaterialComponent.GeometryNormal = {0.f, 0.f, 0.f};
    		MaterialComponent.GeometryTangent = {0.f, 0.f, 0.f};
    		MaterialComponent.GeometryCoatNormal = {0.f, 0.f, 0.f};
    		MaterialComponent.GeometryCoatTangent = {0.f, 0.f, 0.f};

    		MaterialToIndexMap[Material] = FreeIndices.front();
    		FreeIndices.pop();

			return Material;
		}

		/// Albedo
		FMaterialSystem& FMaterialSystem::SetBaseColorWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.BaseWeight = Weight;
    		ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColorWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.BaseWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.BaseColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.BaseColorTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.DiffuseRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughnessTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetMetalness(FEntity MaterialEntity, float Metalness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.Metalness = Metalness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetMetalness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.MetalnessTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetAlbedoNormal(FEntity MaterialEntity, const FVector3& Normal)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.Normal = Normal;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetAlbedoNormal(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.NormalTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Specular
		FMaterialSystem& FMaterialSystem::SetSpecularWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SpecularWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SpecularColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularColorTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SpecularRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularRoughnessTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SpecularIOR = SpecularIOR;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularIORTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SpecularAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularAnisotropyTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRotation(FEntity MaterialEntity, float Rotation)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SpecularRotation = Rotation;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRotation(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularRotationTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}


		/// Transmission
		FMaterialSystem& FMaterialSystem::SetTransmissionWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.TransmissionWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.TransmissionColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDepth(FEntity MaterialEntity, float Depth)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.TransmissionDepth = Depth;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDepth(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionScatter(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.TransmissionScatter = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionScatter(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.TransmissionAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDispersion(FEntity MaterialEntity, float Dispersion)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.TransmissionDispersion = Dispersion;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDispersion(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.TransmissionRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionRoughnessTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}


		/// Subsurface
		FMaterialSystem& FMaterialSystem::SetSubsurfaceWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceRadius(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceRadius = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceRadius(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceScale(FEntity MaterialEntity, float Scale)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceScale = Scale;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceScale(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Sheen
		FMaterialSystem& FMaterialSystem::SetSheenWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SheenWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SheenWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SheenColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.SheenRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Coat
		FMaterialSystem& FMaterialSystem::SetCoatWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatColorTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatRoughnessTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatAnisotropy = Anisotropy;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAnisotropyTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRotation(FEntity MaterialEntity, float Rotation)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatRotation = Rotation;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRotation(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatRotationTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatIOR(FEntity MaterialEntity, float CoatIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatIOR = CoatIOR;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatIORTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatNormal(FEntity MaterialEntity, FVector3 Normal)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatNormal = Normal;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatNormal(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatNormalTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectColor(FEntity MaterialEntity, float Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatAffectColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAffectColorTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.CoatAffectRoughness = Roughness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAffectRoughnessTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Thin film
		FMaterialSystem& FMaterialSystem::SetThinFilmThickness(FEntity MaterialEntity, float Thickness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.ThinFilmThickness = Thickness;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmThickness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinFilmThicknessTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmIOR(FEntity MaterialEntity, float ThinFilmIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.ThinFilmIOR = ThinFilmIOR;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinFilmIORTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;

		}

		/// Emission
		FMaterialSystem& FMaterialSystem::SetEmissionWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.EmissionWeight = Weight;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.EmissionWeightTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.EmissionColor = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.EmissionColorTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Opacity
		FMaterialSystem& FMaterialSystem::SetOpacity(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.Opacity = Color;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetOpacity(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.OpacityTextureIndex = TextureComponent.TextureIndex;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		/// Thin walled
		FMaterialSystem& FMaterialSystem::SetThinWalled(FEntity MaterialEntity, bool ThinWalled)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			MaterialComponent.ThinWalled = ThinWalled;
			ChangedMaterials.insert(MaterialEntity);
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetThinWalled(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinWalledTextureIndex = TextureComponent.TextureIndex;
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

            auto& MaterialComponent = GetComponent<COMPONENTS::FOpenPBRSurfaceComponent>(MaterialEntity);
    		uint32_t MaterialIndex = MaterialToIndexMap[MaterialEntity];

            Result += "FDeviceMaterial GetMaterial(vec2 TextureCoords)\r\n";
            Result += "{\r\n";
            Result += "    FDeviceMaterial Material;\r\n";

            Result += GenerateGetFunction<float>(	"Material.BaseWeight",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseWeight),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseWeightTexture),				MaterialComponent.BaseWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.BaseColor",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseColor),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, BaseColorTexture),					MaterialComponent.BaseColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.DiffuseRoughness",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, DiffuseRoughness),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, DiffuseRoughnessTexture),			MaterialComponent.DiffuseRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.Metalness",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, Metalness),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, MetalnessTexture),					MaterialComponent.MetalnessTexture);
            Result += GenerateGetFunction<FVector3>("Material.Normal",					MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, Normal),					offsetof(COMPONENTS::FOpenPBRSurfaceComponent, NormalTexture),					MaterialComponent.NormalTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularWeight",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularWeight),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularWeightTexture),			MaterialComponent.SpecularWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.SpecularColor",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularColor),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularColorTexture),				MaterialComponent.SpecularColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.SpecularRoughness",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRoughness),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRoughnessTexture),			MaterialComponent.SpecularRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularIOR",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularIOR),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularIORTexture),				MaterialComponent.SpecularIORTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularAnisotropy",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularAnisotropy),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularAnisotropyTexture),		MaterialComponent.SpecularAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.SpecularRotation",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRotation),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SpecularRotationTexture),			MaterialComponent.SpecularRotationTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionWeight",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionWeight),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionWeightTexture),		MaterialComponent.TransmissionWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.TransmissionColor",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionColor),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionColorTexture),			MaterialComponent.TransmissionColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.TransmissionDepth",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDepth),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDepthTexture),			MaterialComponent.TransmissionDepthTexture);
            Result += GenerateGetFunction<FVector3>("Material.TransmissionScatter",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionScatter),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionScatterTexture),		MaterialComponent.TransmissionScatterTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionAnisotropy",	MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionAnisotropy),	offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionAnisotropyTexture),	MaterialComponent.TransmissionAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionDispersion",	MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDispersion),	offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionDispersionTexture),	MaterialComponent.TransmissionDispersionTexture);
            Result += GenerateGetFunction<float>(	"Material.TransmissionRoughness",	MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionRoughness),	offsetof(COMPONENTS::FOpenPBRSurfaceComponent, TransmissionRoughnessTexture),		MaterialComponent.TransmissionRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.SubsurfaceWeight",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceWeight),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceWeightTexture),			MaterialComponent.SubsurfaceWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.SubsurfaceColor",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceColor),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceColorTexture),			MaterialComponent.SubsurfaceColorTexture, true);
            Result += GenerateGetFunction<FVector3>("Material.SubsurfaceRadius",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceRadius),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceRadiusTexture),			MaterialComponent.SubsurfaceRadiusTexture);
            Result += GenerateGetFunction<float>(	"Material.SubsurfaceScale",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceScale),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceScaleTexture),			MaterialComponent.SubsurfaceScaleTexture);
            Result += GenerateGetFunction<float>(	"Material.SubsurfaceAnisotropy",	MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceAnisotropy),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SubsurfaceAnisotropyTexture),		MaterialComponent.SubsurfaceAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.SheenWeight",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SheenWeight),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SheenWeightTexture),				MaterialComponent.SheenWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.SheenColor",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SheenColor),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SheenColorTexture),				MaterialComponent.SheenColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.SheenRoughness",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SheenRoughness),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, SheenRoughnessTexture),			MaterialComponent.SheenRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatWeight",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatWeight),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatWeightTexture),				MaterialComponent.CoatWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.CoatColor",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatColor),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatColorTexture),					MaterialComponent.CoatColorTexture, true);
            Result += GenerateGetFunction<float>(	"Material.CoatRoughness",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRoughness),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRoughnessTexture),				MaterialComponent.CoatRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatAnisotropy",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatAnisotropy),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatAnisotropyTexture),			MaterialComponent.CoatAnisotropyTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatRotation",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRotation),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatRotationTexture),				MaterialComponent.CoatRotationTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatIOR",					MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatIOR),					offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatIORTexture),					MaterialComponent.CoatIORTexture);
            Result += GenerateGetFunction<FVector3>("Material.CoatNormal",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatNormal),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatNormalTexture),				MaterialComponent.CoatNormalTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatAffectColor",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatAffectColor),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatAffectColorTexture),			MaterialComponent.CoatAffectColorTexture);
            Result += GenerateGetFunction<float>(	"Material.CoatAffectRoughness",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatAffectRoughness),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, CoatAffectRoughnessTexture),		MaterialComponent.CoatAffectRoughnessTexture);
            Result += GenerateGetFunction<float>(	"Material.ThinFilmThickness",		MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmThickness),		offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmThicknessTexture),			MaterialComponent.ThinFilmThicknessTexture);
            Result += GenerateGetFunction<float>(	"Material.ThinFilmIOR",				MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmIOR),				offsetof(COMPONENTS::FOpenPBRSurfaceComponent, ThinFilmIORTexture),				MaterialComponent.ThinFilmIORTexture);
            Result += GenerateGetFunction<float>(	"Material.EmissionWeight",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionWeight),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionWeightTexture),			MaterialComponent.EmissionWeightTexture);
            Result += GenerateGetFunction<FVector3>("Material.EmissionColor",			MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColor),			offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColorTexture),				MaterialComponent.EmissionColorTexture, true);
            Result += GenerateGetFunction<FVector3>("Material.Opacity",					MaterialIndex, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, Opacity),					offsetof(COMPONENTS::FOpenPBRSurfaceComponent, OpacityTexture),					MaterialComponent.OpacityTexture);
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
				auto& MaterialComponent = GetComponentByIndex<ECS::COMPONENTS::FOpenPBRSurfaceComponent>(Entry.first);
				Result += "    case " + std::to_string(Entry.first) + ":\r\n";
				Result += "    {\r\n";
				/// TODO: Fix the indexing
				Result += "    " + GenerateGetFunction<FVector3>("Material.EmissionColor", 0, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColor), offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionColorTexture), MaterialComponent.EmissionColorTexture);
				Result += "    " + GenerateGetFunction<float>("Material.EmissionWeight", 0, offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionWeight), offsetof(COMPONENTS::FOpenPBRSurfaceComponent, EmissionWeightTexture), MaterialComponent.EmissionWeightTexture);
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
