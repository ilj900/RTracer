#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FMaterialComponent : FMaterial
        {
            FMaterialComponent()
            {
                BaseWeight = 0.8f;
                BaseColor = {1, 1, 1};
                DiffuseRoughness = 0.f;
                Metalness = 0.f;
                Normal = {0, 1, 0};

                SpecularWeight = 1.f;
                SpecularColor = {1, 1, 1};
                SpecularRoughness = 0.2f;
                SpecularIOR = 1.5;
                SpecularAnisotropy = 0.f;
                SpecularRotation = 0.f;

                TransmissionWeight = 0;
                TransmissionColor = {1, 1, 1};
                TransmissionDepth = 0.f;
                TransmissionScatter = {0, 0, 0};
                TransmissionAnisotropy = 0.f;
                TransmissionDispersion = 0.f;
                TransmissionRoughness = 0.f;

                SubsurfaceWeight = 0.f;
                SubsurfaceColor = {1, 1, 1};
                SubsurfaceRadius = {1, 1, 1};
                SubsurfaceScale = 1.f;
                SubsurfaceAnisotropy = 0.f;

                SheenWeight = 0.f;
                SheenColor = {1, 1, 1};
                SheenRoughness = 0.3f;

                CoatWeight = 0.f;
                CoatColor = {1, 1, 1};
                CoatRoughness = 0.1f;
                CoatAnisotropy = 0.f;
                CoatRotation = 0.f;
                CoatIOR = 1.5f;
                CoatNormal = {0, 1, 0};
                CoatAffectColor = 0.f;
                CoatAffectRoughness = 0.f;

                ThinFilmThickness = 0.f;
                ThinFilmIOR = 1.5f;

                Emission = 0;
                EmissionColor = {1, 1, 1};

                Opacity = {1, 1, 1};
                ThinWalled = false;
            };

            FMaterialComponent(float BaseWeightIn, FVector3 BaseColorIn, float DiffuseRoughnessIn, float MetalnessIn, FVector3 NormalIn,
            float SpecularWeightIn, FVector3 SpecularColorIn, float SpecularRoughnessIn, float SpecularIORIn, float SpecularAnisotropyIn, float SpecularRotationIn,
            float TransmissionWeightIn, FVector3 TransmissionColorIn, float TransmissionDepthIn, FVector3 TransmissionScatterIn, float TransmissionAnisotropyIn, float TransmissionDispersionIn, float TransmissionRoughnessIn,
            float SubsurfaceWeightIn, FVector3 SubsurfaceColorIn, FVector3 SubsurfaceRadiusIn, float SubsurfaceScaleIn, float SubsurfaceAnisotropyIn,
            float SheenWeightIn, FVector3 SheenColorIn, float SheenRoughnessIn,
            float CoatWeightIn, FVector3 CoatColorIn, float CoatRoughnessIn, float CoatAnisotropyIn, float CoatRotationIn, float CoatIORIn, FVector3 CoatNormalIn, float CoatAffectColorIn, float CoatAffectRoughnessIn,
            float ThinFilmThicknessIn, float ThinFilmIORIn,
            float EmissionIn, FVector3 EmissionColorIn,
            FVector3 OpacityIn, uint32_t ThinWalledIn)
            {
                BaseWeight = BaseWeightIn;
                BaseColor = BaseColorIn;
                DiffuseRoughness = DiffuseRoughnessIn;
                Metalness = MetalnessIn;
                Normal = NormalIn;

                SpecularWeight = SpecularWeightIn;
                SpecularColor = SpecularColorIn;
                SpecularRoughness = SpecularRoughnessIn;
                SpecularIOR = SpecularIORIn;
                SpecularAnisotropy = SpecularAnisotropyIn;
                SpecularRotation = SpecularRotationIn;

                TransmissionWeight = TransmissionWeightIn;
                TransmissionColor = TransmissionColorIn;
                TransmissionDepth = TransmissionDepthIn;
                TransmissionScatter = TransmissionScatterIn;
                TransmissionAnisotropy = TransmissionAnisotropyIn;
                TransmissionDispersion = TransmissionDispersionIn;
                TransmissionRoughness = TransmissionRoughnessIn;

                SubsurfaceWeight = SubsurfaceWeightIn;
                SubsurfaceColor = SubsurfaceColorIn;
                SubsurfaceRadius = SubsurfaceRadiusIn;
                SubsurfaceScale = SubsurfaceScaleIn;
                SubsurfaceAnisotropy = SubsurfaceAnisotropyIn;

                SheenWeight = SheenWeightIn;
                SheenColor = SheenColorIn;
                SheenRoughness = SheenRoughnessIn;

                CoatWeight = CoatWeightIn;
                CoatColor = CoatColorIn;
                CoatRoughness = CoatRoughnessIn;
                CoatAnisotropy = CoatAnisotropyIn;
                CoatRotation = CoatRotationIn;
                CoatIOR = CoatIORIn;
                CoatNormal = CoatNormalIn;
                CoatAffectColor = CoatAffectColorIn;
                CoatAffectRoughness = CoatAffectRoughnessIn;

                ThinFilmThickness = ThinFilmThicknessIn;
                ThinFilmIOR = ThinFilmIORIn;

                Emission = EmissionIn;
                EmissionColor = EmissionColorIn;

                Opacity = OpacityIn;
                ThinWalled = ThinWalledIn;
            };
        };
    }
}