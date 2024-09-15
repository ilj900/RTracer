#include "material_component.h"
#include "material_system.h"
#include "texture_component.h"

#include "texture_manager.h"

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

        FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, float Red, float Green, float Blue)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.BaseColor = FVector3(Red, Green, Blue);
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, float DiffuseRoughness)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.DiffuseRoughness = DiffuseRoughness;
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.SpecularIOR = SpecularIOR;
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetNormal(FEntity MaterialEntity, FVector3 Normal)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.Normal = Normal;
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, FEntity TextureEntity)
        {
            auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.BaseColorTexture = TextureComponent.TextureIndex;
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, FEntity TextureEntity)
        {
            auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.DiffuseRoughnessTexture = TextureComponent.TextureIndex;
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, FEntity TextureEntity)
        {
            auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.SpecularIORTexture = TextureComponent.TextureIndex;
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetNormal(FEntity MaterialEntity, FEntity TextureEntity)
        {
            auto& TextureComponent = GetComponent<ECS::COMPONENTS::FTextureComponent>(TextureEntity);
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.NormalTexture = TextureComponent.TextureIndex;
            return *this;
        }

        std::string FMaterialSystem::GenerateMaterialCode(FEntity MaterialEntity)
        {
            std::string Result;

            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);

            auto GenerateGetFunctionFloat = [](const std::string& ParameterName, uint32_t TextureIndex, float FallbackValue)
            {
                std::string Result = "    " + ParameterName;

                if (UINT32_MAX == TextureIndex)
                {
                    Result += " = " + std::to_string(FallbackValue) + ";\r\n";
                }
                else
                {
                    Result += " = SampleFloat(" + std::to_string(TextureIndex) + ", TextureCoords);\r\n";
                }

                return Result;
            };

            auto GenerateGetFunctionVector3 = [](const std::string& ParameterName, uint32_t TextureIndex, FVector3 FallbackValue)
            {
                std::string Result = "    " + ParameterName;

                if (UINT32_MAX == TextureIndex)
                {
                    Result += " = " + FallbackValue.ToString() + ";\r\n";
                }
                else
                {
                    Result += " = SampleVec3(" + std::to_string(TextureIndex) + ", TextureCoords);\r\n";
                }

                return Result;
            };

            Result += "FDeviceMaterial GetMaterial(vec2 TextureCoords)\r\n";
            Result += "{\r\n";
            Result += "    FDeviceMaterial Material;\r\n";

            Result += GenerateGetFunctionFloat("Material.BaseWeight", MaterialComponent.BaseWeightTexture, MaterialComponent.BaseWeight);
            Result += GenerateGetFunctionVector3("Material.BaseColor", MaterialComponent.BaseColorTexture, MaterialComponent.BaseColor);
            Result += GenerateGetFunctionFloat("Material.DiffuseRoughness", MaterialComponent.DiffuseRoughnessTexture, MaterialComponent.DiffuseRoughness);
            Result += GenerateGetFunctionFloat("Material.Metalness", MaterialComponent.MetalnessTexture, MaterialComponent.Metalness);
            Result += GenerateGetFunctionVector3("Material.Normal", MaterialComponent.NormalTexture, MaterialComponent.Normal);
            Result += GenerateGetFunctionFloat("Material.SpecularWeight", MaterialComponent.SpecularWeightTexture, MaterialComponent.SpecularWeight);
            Result += GenerateGetFunctionVector3("Material.SpecularColor", MaterialComponent.SpecularColorTexture, MaterialComponent.SpecularColor);
            Result += GenerateGetFunctionFloat("Material.SpecularRoughness", MaterialComponent.SpecularRoughnessTexture, MaterialComponent.SpecularRoughness);
            Result += GenerateGetFunctionFloat("Material.SpecularIOR", MaterialComponent.SpecularIORTexture, MaterialComponent.SpecularIOR);
            Result += GenerateGetFunctionFloat("Material.SpecularAnisotropy", MaterialComponent.SpecularAnisotropyTexture, MaterialComponent.SpecularAnisotropy);
            Result += GenerateGetFunctionFloat("Material.SpecularRotation", MaterialComponent.SpecularRotationTexture, MaterialComponent.SpecularRotation);
            Result += GenerateGetFunctionFloat("Material.TransmissionWeight", MaterialComponent.TransmissionWeightTexture, MaterialComponent.TransmissionWeight);
            Result += GenerateGetFunctionVector3("Material.TransmissionColor", MaterialComponent.TransmissionColorTexture, MaterialComponent.TransmissionColor);
            Result += GenerateGetFunctionFloat("Material.TransmissionDepth", MaterialComponent.TransmissionDepthTexture, MaterialComponent.TransmissionDepth);
            Result += GenerateGetFunctionVector3("Material.TransmissionScatter", MaterialComponent.TransmissionScatterTexture, MaterialComponent.TransmissionScatter);
            Result += GenerateGetFunctionFloat("Material.TransmissionAnisotropy", MaterialComponent.TransmissionAnisotropyTexture, MaterialComponent.TransmissionAnisotropy);
            Result += GenerateGetFunctionFloat("Material.TransmissionDispersion", MaterialComponent.TransmissionDispersionTexture, MaterialComponent.TransmissionDispersion);
            Result += GenerateGetFunctionFloat("Material.TransmissionRoughness", MaterialComponent.TransmissionRoughnessTexture, MaterialComponent.TransmissionRoughness);
            Result += GenerateGetFunctionFloat("Material.SubsurfaceWeight", MaterialComponent.SubsurfaceWeightTexture, MaterialComponent.SubsurfaceWeight);
            Result += GenerateGetFunctionVector3("Material.SubsurfaceColor", MaterialComponent.SubsurfaceColorTexture, MaterialComponent.SubsurfaceColor);
            Result += GenerateGetFunctionVector3("Material.SubsurfaceRadius", MaterialComponent.SubsurfaceRadiusTexture, MaterialComponent.SubsurfaceRadius);
            Result += GenerateGetFunctionFloat("Material.SubsurfaceScale", MaterialComponent.SubsurfaceScaleTexture, MaterialComponent.SubsurfaceScale);
            Result += GenerateGetFunctionFloat("Material.SubsurfaceAnisotropy", MaterialComponent.SubsurfaceAnisotropyTexture, MaterialComponent.SubsurfaceAnisotropy);
            Result += GenerateGetFunctionFloat("Material.SheenWeight", MaterialComponent.SheenWeightTexture, MaterialComponent.SheenWeight);
            Result += GenerateGetFunctionVector3("Material.SheenColor", MaterialComponent.SheenColorTexture, MaterialComponent.SheenColor);
            Result += GenerateGetFunctionFloat("Material.SheenRoughness", MaterialComponent.SheenRoughnessTexture, MaterialComponent.SheenRoughness);
            Result += GenerateGetFunctionFloat("Material.CoatWeight", MaterialComponent.CoatWeightTexture, MaterialComponent.CoatWeight);
            Result += GenerateGetFunctionVector3("Material.CoatColor", MaterialComponent.CoatColorTexture, MaterialComponent.CoatColor);
            Result += GenerateGetFunctionFloat("Material.CoatRoughness", MaterialComponent.CoatRoughnessTexture, MaterialComponent.CoatRoughness);
            Result += GenerateGetFunctionFloat("Material.CoatAnisotropy", MaterialComponent.CoatAnisotropyTexture, MaterialComponent.CoatAnisotropy);
            Result += GenerateGetFunctionFloat("Material.CoatRotation", MaterialComponent.CoatRotationTexture, MaterialComponent.CoatRotation);
            Result += GenerateGetFunctionFloat("Material.CoatIOR", MaterialComponent.CoatIORTexture, MaterialComponent.CoatIOR);
            Result += GenerateGetFunctionVector3("Material.CoatNormal", MaterialComponent.CoatNormalTexture, MaterialComponent.CoatNormal);
            Result += GenerateGetFunctionFloat("Material.CoatAffectColor", MaterialComponent.CoatAffectColorTexture, MaterialComponent.CoatAffectColor);
            Result += GenerateGetFunctionFloat("Material.CoatAffectRoughness", MaterialComponent.CoatAffectRoughnessTexture, MaterialComponent.CoatAffectRoughness);
            Result += GenerateGetFunctionFloat("Material.ThinFilmThickness", MaterialComponent.ThinFilmThicknessTexture, MaterialComponent.ThinFilmThickness);
            Result += GenerateGetFunctionFloat("Material.ThinFilmIOR", MaterialComponent.ThinFilmIORTexture, MaterialComponent.ThinFilmIOR);
            Result += GenerateGetFunctionFloat("Material.Emission", MaterialComponent.EmissionTexture, MaterialComponent.Emission);
            Result += GenerateGetFunctionVector3("Material.EmissionColor", MaterialComponent.EmissionColorTexture, MaterialComponent.EmissionColor);
            Result += GenerateGetFunctionVector3("Material.Opacity", MaterialComponent.OpacityTexture, MaterialComponent.Opacity);
            //TODO: Add uint textures
            Result += "    Material.ThinWalled = " + std::to_string(MaterialComponent.ThinWalled) + ";\r\n";

            Result += "    return Material;\r\n";
            Result += "};\r\n";

            return Result;
        }
    }
}