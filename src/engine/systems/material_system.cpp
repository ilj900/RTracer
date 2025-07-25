#include "material_component.h"
#include "material_system.h"
#include "texture_component.h"

#include "texture_manager.h"

#include <typeindex>
#include <type_traits>

namespace ECS
{
    namespace SYSTEMS
    {
        FEntity FMaterialSystem::CreateDefaultMaterial()
        {
            FEntity Material = COORDINATOR().CreateEntity();
            COORDINATOR().AddComponent<ECS::COMPONENTS::FMaterialComponent>(Material, {});
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

			return Material;
		}

		/// Albedo
		FMaterialSystem& FMaterialSystem::SetBaseColorWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.BaseWeight = Weight;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColorWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.BaseWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.BaseColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.BaseColorTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.DiffuseRoughness = Roughness;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughnessTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetMetalness(FEntity MaterialEntity, float Metalness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.Metalness = Metalness;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetMetalness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.MetalnessTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetAlbedoNormal(FEntity MaterialEntity, FVector3 Normal)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.Normal = Normal;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetAlbedoNormal(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.NormalTexture = TextureComponent.TextureIndex;
			return *this;
		}

		/// Specular
		FMaterialSystem& FMaterialSystem::SetSpecularWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularWeight = Weight;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularColorTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularRoughness = Roughness;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularRoughnessTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularIOR = SpecularIOR;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularIORTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularAnisotropy = Anisotropy;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularAnisotropyTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRotation(FEntity MaterialEntity, float Rotation)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SpecularRotation = Rotation;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSpecularRotation(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularRotationTexture = TextureComponent.TextureIndex;
			return *this;
		}


		/// Transmission
		FMaterialSystem& FMaterialSystem::SetTransmissionWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionWeight = Weight;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDepth(FEntity MaterialEntity, float Depth)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionDepth = Depth;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDepth(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionScatter(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionScatter = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionScatter(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionAnisotropy = Anisotropy;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDispersion(FEntity MaterialEntity, float Dispersion)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionDispersion = Dispersion;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionDispersion(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.TransmissionRoughness = Roughness;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetTransmissionRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.TransmissionRoughnessTexture = TextureComponent.TextureIndex;
			return *this;
		}


		/// Subsurface
		FMaterialSystem& FMaterialSystem::SetSubsurfaceWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceWeight = Weight;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SpecularWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceRadius(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceRadius = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceRadius(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceScale(FEntity MaterialEntity, float Scale)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceScale = Scale;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceScale(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SubsurfaceAnisotropy = Anisotropy;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSubsurfaceAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		/// Sheen
		FMaterialSystem& FMaterialSystem::SetSheenWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SheenWeight = Weight;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.SheenWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SheenColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.SheenRoughness = Roughness;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetSheenRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.DiffuseRoughness = TextureComponent.TextureIndex;
			return *this;
		}

		/// Coat
		FMaterialSystem& FMaterialSystem::SetCoatWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatWeight = Weight;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatColorTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatRoughness = Roughness;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatRoughnessTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAnisotropy(FEntity MaterialEntity, float Anisotropy)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatAnisotropy = Anisotropy;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAnisotropy(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAnisotropyTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRotation(FEntity MaterialEntity, float Rotation)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatRotation = Rotation;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatRotation(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatRotationTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatIOR(FEntity MaterialEntity, float CoatIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatIOR = CoatIOR;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatIORTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatNormal(FEntity MaterialEntity, FVector3 Normal)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatNormal = Normal;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatNormal(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatNormalTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectColor(FEntity MaterialEntity, float Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatAffectColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAffectColorTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectRoughness(FEntity MaterialEntity, float Roughness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.CoatAffectRoughness = Roughness;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetCoatAffectRoughness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.CoatAffectRoughnessTexture = TextureComponent.TextureIndex;
			return *this;
		}

		/// Thin film
		FMaterialSystem& FMaterialSystem::SetThinFilmThickness(FEntity MaterialEntity, float Thickness)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.ThinFilmThickness = Thickness;
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmThickness(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinFilmThicknessTexture = TextureComponent.TextureIndex;
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmIOR(FEntity MaterialEntity, float ThinFilmIOR)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.ThinFilmIOR = ThinFilmIOR;
			return *this;

		}
		FMaterialSystem& FMaterialSystem::SetThinFilmIOR(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinFilmIORTexture = TextureComponent.TextureIndex;
			return *this;

		}

		/// Emission
		FMaterialSystem& FMaterialSystem::SetEmissionWeight(FEntity MaterialEntity, float Weight)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.EmissionWeight = Weight;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionWeight(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.EmissionWeightTexture = TextureComponent.TextureIndex;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionColor(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.EmissionColor = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetEmissionColor(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.EmissionColorTexture = TextureComponent.TextureIndex;
			return *this;
		}

		/// Opacity
		FMaterialSystem& FMaterialSystem::SetOpacity(FEntity MaterialEntity, const FVector3& Color)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.Opacity = Color;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetOpacity(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.OpacityTexture = TextureComponent.TextureIndex;
			return *this;
		}

		/// Thin walled
		FMaterialSystem& FMaterialSystem::SetThinWalled(FEntity MaterialEntity, bool ThinWalled)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			MaterialComponent.ThinWalled = ThinWalled;
			return *this;
		}

		FMaterialSystem& FMaterialSystem::SetThinWalled(FEntity MaterialEntity, FEntity TextureEntity)
		{
			auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
			auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
			MaterialComponent.ThinWalledTexture = TextureComponent.TextureIndex;
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
    	std::string GenerateGetFunction(const std::string& ParameterName, uint32_t TextureIndex, T FallbackValue, bool bConvertToLinear = false)
    	{
    		std::string Result = "    " + ParameterName;

    		if (UINT32_MAX == TextureIndex)
    		{
    			if constexpr (std::is_same_v<T, float> || std::is_same_v<T, int> || std::is_same_v<T, uint32_t>)
    			{
    				Result += " = " + std::to_string(FallbackValue) + ";\r\n";
    			}
    			else
    			{
    				Result += " = " + FallbackValue.ToString() + ";\r\n";

    			}
    		}
    		else
    		{
    			/// If we sample a texture, in some cases it might be in sRGB format, and we need to translate it to linear color space
    			if (bConvertToLinear)
    			{
    				Result += " = SRGBToLinear(" + TypeToString<T> ::Name()+ "(" + std::to_string(TextureIndex) + ", TextureCoords));\r\n";
    			}
    			else
    			{
    				Result += " = " + TypeToString<T> ::Name()+ "(" + std::to_string(TextureIndex) + ", TextureCoords);\r\n";
    			}
    		}

    		return Result;
    	}

        std::string FMaterialSystem::GenerateMaterialCode(FEntity MaterialEntity)
        {
            std::string Result;

            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);

            Result += "FDeviceMaterial GetMaterial(vec2 TextureCoords)\r\n";
            Result += "{\r\n";
            Result += "    FDeviceMaterial Material;\r\n";

            Result += GenerateGetFunction<float>("Material.BaseWeight", MaterialComponent.BaseWeightTexture, MaterialComponent.BaseWeight);
            Result += GenerateGetFunction<FVector3>("Material.BaseColor", MaterialComponent.BaseColorTexture, MaterialComponent.BaseColor, true);
            Result += GenerateGetFunction<float>("Material.DiffuseRoughness", MaterialComponent.DiffuseRoughnessTexture, MaterialComponent.DiffuseRoughness);
            Result += GenerateGetFunction<float>("Material.Metalness", MaterialComponent.MetalnessTexture, MaterialComponent.Metalness);
            Result += GenerateGetFunction<FVector3>("Material.Normal", MaterialComponent.NormalTexture, MaterialComponent.Normal);
            Result += GenerateGetFunction<float>("Material.SpecularWeight", MaterialComponent.SpecularWeightTexture, MaterialComponent.SpecularWeight);
            Result += GenerateGetFunction<FVector3>("Material.SpecularColor", MaterialComponent.SpecularColorTexture, MaterialComponent.SpecularColor, true);
            Result += GenerateGetFunction<float>("Material.SpecularRoughness", MaterialComponent.SpecularRoughnessTexture, MaterialComponent.SpecularRoughness);
            Result += GenerateGetFunction<float>("Material.SpecularIOR", MaterialComponent.SpecularIORTexture, MaterialComponent.SpecularIOR);
            Result += GenerateGetFunction<float>("Material.SpecularAnisotropy", MaterialComponent.SpecularAnisotropyTexture, MaterialComponent.SpecularAnisotropy);
            Result += GenerateGetFunction<float>("Material.SpecularRotation", MaterialComponent.SpecularRotationTexture, MaterialComponent.SpecularRotation);
            Result += GenerateGetFunction<float>("Material.TransmissionWeight", MaterialComponent.TransmissionWeightTexture, MaterialComponent.TransmissionWeight);
            Result += GenerateGetFunction<FVector3>("Material.TransmissionColor", MaterialComponent.TransmissionColorTexture, MaterialComponent.TransmissionColor, true);
            Result += GenerateGetFunction<float>("Material.TransmissionDepth", MaterialComponent.TransmissionDepthTexture, MaterialComponent.TransmissionDepth);
            Result += GenerateGetFunction<FVector3>("Material.TransmissionScatter", MaterialComponent.TransmissionScatterTexture, MaterialComponent.TransmissionScatter);
            Result += GenerateGetFunction<float>("Material.TransmissionAnisotropy", MaterialComponent.TransmissionAnisotropyTexture, MaterialComponent.TransmissionAnisotropy);
            Result += GenerateGetFunction<float>("Material.TransmissionDispersion", MaterialComponent.TransmissionDispersionTexture, MaterialComponent.TransmissionDispersion);
            Result += GenerateGetFunction<float>("Material.TransmissionRoughness", MaterialComponent.TransmissionRoughnessTexture, MaterialComponent.TransmissionRoughness);
            Result += GenerateGetFunction<float>("Material.SubsurfaceWeight", MaterialComponent.SubsurfaceWeightTexture, MaterialComponent.SubsurfaceWeight);
            Result += GenerateGetFunction<FVector3>("Material.SubsurfaceColor", MaterialComponent.SubsurfaceColorTexture, MaterialComponent.SubsurfaceColor, true);
            Result += GenerateGetFunction<FVector3>("Material.SubsurfaceRadius", MaterialComponent.SubsurfaceRadiusTexture, MaterialComponent.SubsurfaceRadius);
            Result += GenerateGetFunction<float>("Material.SubsurfaceScale", MaterialComponent.SubsurfaceScaleTexture, MaterialComponent.SubsurfaceScale);
            Result += GenerateGetFunction<float>("Material.SubsurfaceAnisotropy", MaterialComponent.SubsurfaceAnisotropyTexture, MaterialComponent.SubsurfaceAnisotropy);
            Result += GenerateGetFunction<float>("Material.SheenWeight", MaterialComponent.SheenWeightTexture, MaterialComponent.SheenWeight);
            Result += GenerateGetFunction<FVector3>("Material.SheenColor", MaterialComponent.SheenColorTexture, MaterialComponent.SheenColor, true);
            Result += GenerateGetFunction<float>("Material.SheenRoughness", MaterialComponent.SheenRoughnessTexture, MaterialComponent.SheenRoughness);
            Result += GenerateGetFunction<float>("Material.CoatWeight", MaterialComponent.CoatWeightTexture, MaterialComponent.CoatWeight);
            Result += GenerateGetFunction<FVector3>("Material.CoatColor", MaterialComponent.CoatColorTexture, MaterialComponent.CoatColor, true);
            Result += GenerateGetFunction<float>("Material.CoatRoughness", MaterialComponent.CoatRoughnessTexture, MaterialComponent.CoatRoughness);
            Result += GenerateGetFunction<float>("Material.CoatAnisotropy", MaterialComponent.CoatAnisotropyTexture, MaterialComponent.CoatAnisotropy);
            Result += GenerateGetFunction<float>("Material.CoatRotation", MaterialComponent.CoatRotationTexture, MaterialComponent.CoatRotation);
            Result += GenerateGetFunction<float>("Material.CoatIOR", MaterialComponent.CoatIORTexture, MaterialComponent.CoatIOR);
            Result += GenerateGetFunction<FVector3>("Material.CoatNormal", MaterialComponent.CoatNormalTexture, MaterialComponent.CoatNormal);
            Result += GenerateGetFunction<float>("Material.CoatAffectColor", MaterialComponent.CoatAffectColorTexture, MaterialComponent.CoatAffectColor);
            Result += GenerateGetFunction<float>("Material.CoatAffectRoughness", MaterialComponent.CoatAffectRoughnessTexture, MaterialComponent.CoatAffectRoughness);
            Result += GenerateGetFunction<float>("Material.ThinFilmThickness", MaterialComponent.ThinFilmThicknessTexture, MaterialComponent.ThinFilmThickness);
            Result += GenerateGetFunction<float>("Material.ThinFilmIOR", MaterialComponent.ThinFilmIORTexture, MaterialComponent.ThinFilmIOR);
            Result += GenerateGetFunction<float>("Material.EmissionWeight", MaterialComponent.EmissionWeightTexture, MaterialComponent.EmissionWeight);
            Result += GenerateGetFunction<FVector3>("Material.EmissionColor", MaterialComponent.EmissionColorTexture, MaterialComponent.EmissionColor, true);
            Result += GenerateGetFunction<FVector3>("Material.Opacity", MaterialComponent.OpacityTexture, MaterialComponent.Opacity);
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
				Result += "    " + GenerateGetFunction<FVector3>("Material.EmissionColor", MaterialComponent.EmissionColorTexture, MaterialComponent.EmissionColor);
				Result += "    " + GenerateGetFunction<float>("Material.EmissionWeight", MaterialComponent.EmissionWeightTexture, MaterialComponent.EmissionWeight);
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