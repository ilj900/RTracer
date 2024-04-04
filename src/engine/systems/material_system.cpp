#include "material_component.h"
#include "material_system.h"

namespace ECS
{
    namespace SYSTEMS
    {
        void FMaterialSystem::Init(int NumberOfSimultaneousSubmits)
        {
            FGPUBufferableSystem::Init(NumberOfSimultaneousSubmits, sizeof(ECS::COMPONENTS::FMaterialComponent) * MAX_MATERIALS,
                                       VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, "Device_Material_Buffer");
        }

        void FMaterialSystem::Update()
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FMaterialComponent>();
        }

        void FMaterialSystem::Update(int Index)
        {
            FGPUBufferableSystem::UpdateTemplate<ECS::COMPONENTS::FMaterialComponent>(Index);
        }

        FEntity FMaterialSystem::CreateMaterial()
        {
            auto& Coordinator = GetCoordinator();
            FEntity Material = Coordinator.CreateEntity();
            Coordinator.AddComponent<ECS::COMPONENTS::FMaterialComponent>(Material, {});
            return Material;
        }

        FMaterialSystem& FMaterialSystem::SetBaseColor(FEntity MaterialEntity, float Red, float Green, float Blue)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.BaseColor = FVector3(Red, Green, Blue);
            MarkDirty(MaterialEntity);
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetDiffuseRoughness(FEntity MaterialEntity, float DiffuseRoughness)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.DiffuseRoughness = DiffuseRoughness;
            MarkDirty(MaterialEntity);
            return *this;
        }

        FMaterialSystem& FMaterialSystem::SetSpecularIOR(FEntity MaterialEntity, float SpecularIOR)
        {
            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);
            MaterialComponent.SpecularIOR = SpecularIOR;
            MarkDirty(MaterialEntity);
            return *this;
        }

        std::string FMaterialSystem::GenerateMaterialCode(FEntity MaterialEntity)
        {
            std::string Result;

            auto& MaterialComponent = GetComponent<ECS::COMPONENTS::FMaterialComponent>(MaterialEntity);

            Result += "FMaterial GetMaterial()\r\n";
            Result += "{\r\n";
            Result += "    FMaterial Material;\r\n";

            Result += "    Material.BaseWeight = " + std::to_string(MaterialComponent.BaseWeight) + ";\r\n";
            Result += "    Material.BaseColor = " + MaterialComponent.BaseColor.ToString() + ";\r\n";
            Result += "    Material.DiffuseRoughness = " + std::to_string(MaterialComponent.DiffuseRoughness) + ";\r\n";
            Result += "    Material.Metalness = " + std::to_string(MaterialComponent.Metalness) + ";\r\n";
            Result += "    Material.Normal = " + MaterialComponent.Normal.ToString() + ";\r\n";
            Result += "    Material.SpecularWeight = " + std::to_string(MaterialComponent.SpecularWeight) + ";\r\n";
            Result += "    Material.SpecularColor = " + MaterialComponent.SpecularColor.ToString() + ";\r\n";
            Result += "    Material.SpecularRoughness = " + std::to_string(MaterialComponent.SpecularRoughness) + ";\r\n";
            Result += "    Material.SpecularIOR = " + std::to_string(MaterialComponent.SpecularIOR) + ";\r\n";
            Result += "    Material.SpecularAnisotropy = " + std::to_string(MaterialComponent.SpecularAnisotropy) + ";\r\n";
            Result += "    Material.SpecularRotation = " + std::to_string(MaterialComponent.SpecularRotation) + ";\r\n";
            Result += "    Material.TransmissionWeight = " + std::to_string(MaterialComponent.TransmissionWeight) + ";\r\n";
            Result += "    Material.TransmissionColor = " + MaterialComponent.TransmissionColor.ToString() + ";\r\n";
            Result += "    Material.TransmissionDepth = " + std::to_string(MaterialComponent.TransmissionDepth) + ";\r\n";
            Result += "    Material.TransmissionScatter = " + MaterialComponent.TransmissionScatter.ToString() + ";\r\n";
            Result += "    Material.TransmissionAnisotropy = " + std::to_string(MaterialComponent.TransmissionAnisotropy) + ";\r\n";
            Result += "    Material.TransmissionDispersion = " + std::to_string(MaterialComponent.TransmissionDispersion) + ";\r\n";
            Result += "    Material.TransmissionRoughness = " + std::to_string(MaterialComponent.TransmissionRoughness) + ";\r\n";
            Result += "    Material.SubsurfaceWeight = " + std::to_string(MaterialComponent.SubsurfaceWeight) + ";\r\n";
            Result += "    Material.SubsurfaceColor = " + MaterialComponent.SubsurfaceColor.ToString() + ";\r\n";
            Result += "    Material.SubsurfaceRadius = " + MaterialComponent.SubsurfaceRadius.ToString() + ";\r\n";
            Result += "    Material.SubsurfaceScale = " + std::to_string(MaterialComponent.SubsurfaceScale) + ";\r\n";
            Result += "    Material.SubsurfaceAnisotropy = " + std::to_string(MaterialComponent.SubsurfaceAnisotropy) + ";\r\n";
            Result += "    Material.SheenWeight = " + std::to_string(MaterialComponent.SheenWeight) + ";\r\n";
            Result += "    Material.SheenColor = " + MaterialComponent.SheenColor.ToString() + ";\r\n";
            Result += "    Material.SheenRoughness = " + std::to_string(MaterialComponent.SheenRoughness) + ";\r\n";
            Result += "    Material.CoatWeight = " + std::to_string(MaterialComponent.CoatWeight) + ";\r\n";
            Result += "    Material.CoatColor = " + MaterialComponent.CoatColor.ToString() + ";\r\n";
            Result += "    Material.CoatRoughness = " + std::to_string(MaterialComponent.CoatRoughness) + ";\r\n";
            Result += "    Material.CoatAnisotropy = " + std::to_string(MaterialComponent.CoatAnisotropy) + ";\r\n";
            Result += "    Material.CoatRotation = " + std::to_string(MaterialComponent.CoatRotation) + ";\r\n";
            Result += "    Material.CoatIOR = " + std::to_string(MaterialComponent.CoatIOR) + ";\r\n";
            Result += "    Material.CoatNormal = " + MaterialComponent.CoatNormal.ToString() + ";\r\n";
            Result += "    Material.CoatAffectColor = " + std::to_string(MaterialComponent.CoatAffectColor) + ";\r\n";
            Result += "    Material.CoatAffectRoughness = " + std::to_string(MaterialComponent.CoatAffectRoughness) + ";\r\n";
            Result += "    Material.ThinFilmThickness = " + std::to_string(MaterialComponent.ThinFilmThickness) + ";\r\n";
            Result += "    Material.ThinFilmIOR = " + std::to_string(MaterialComponent.ThinFilmIOR) + ";\r\n";
            Result += "    Material.Emission = " + std::to_string(MaterialComponent.Emission) + ";\r\n";
            Result += "    Material.EmissionColor = " + MaterialComponent.EmissionColor.ToString() + ";\r\n";
            Result += "    Material.Opacity = " + MaterialComponent.Opacity.ToString() + ";\r\n";
            Result += "    Material.ThinWalled = " + std::to_string(MaterialComponent.ThinWalled) + ";\r\n";

            Result += "    return Material;\r\n";
            Result += "};\r\n";

            return Result;
        }
    }
}