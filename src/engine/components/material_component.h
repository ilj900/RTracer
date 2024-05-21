#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FMaterialComponent : FDeviceMaterial
        {
            FMaterialComponent()
            {
                BaseWeight = 1.f;
                BaseColor = {1, 1, 1};
                DiffuseRoughness = 0.f;
                Metalness = 0.f;
                Normal = {0, 1, 0};

                SpecularWeight = 0.f;
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

            uint32_t BaseWeightTexture = UINT32_MAX;
            uint32_t BaseColorTexture = UINT32_MAX;
            uint32_t DiffuseRoughnessTexture = UINT32_MAX;
            uint32_t MetalnessTexture = UINT32_MAX;
            uint32_t NormalTexture = UINT32_MAX;

            uint32_t SpecularWeightTexture = UINT32_MAX;
            uint32_t SpecularColorTexture = UINT32_MAX;
            uint32_t SpecularRoughnessTexture = UINT32_MAX;
            uint32_t SpecularIORTexture = UINT32_MAX;
            uint32_t SpecularAnisotropyTexture = UINT32_MAX;
            uint32_t SpecularRotationTexture = UINT32_MAX;

            uint32_t TransmissionWeightTexture = UINT32_MAX;
            uint32_t TransmissionColorTexture = UINT32_MAX;
            uint32_t TransmissionDepthTexture = UINT32_MAX;
            uint32_t TransmissionScatterTexture = UINT32_MAX;
            uint32_t TransmissionAnisotropyTexture = UINT32_MAX;
            uint32_t TransmissionDispersionTexture = UINT32_MAX;
            uint32_t TransmissionRoughnessTexture = UINT32_MAX;

            uint32_t SubsurfaceWeightTexture = UINT32_MAX;
            uint32_t SubsurfaceColorTexture = UINT32_MAX;
            uint32_t SubsurfaceRadiusTexture = UINT32_MAX;
            uint32_t SubsurfaceScaleTexture = UINT32_MAX;
            uint32_t SubsurfaceAnisotropyTexture = UINT32_MAX;

            uint32_t SheenWeightTexture = UINT32_MAX;
            uint32_t SheenColorTexture = UINT32_MAX;
            uint32_t SheenRoughnessTexture = UINT32_MAX;

            uint32_t CoatWeightTexture = UINT32_MAX;
            uint32_t CoatColorTexture = UINT32_MAX;
            uint32_t CoatRoughnessTexture = UINT32_MAX;
            uint32_t CoatAnisotropyTexture = UINT32_MAX;
            uint32_t CoatRotationTexture = UINT32_MAX;
            uint32_t CoatIORTexture = UINT32_MAX;
            uint32_t CoatNormalTexture = UINT32_MAX;
            uint32_t CoatAffectColorTexture = UINT32_MAX;
            uint32_t CoatAffectRoughnessTexture = UINT32_MAX;

            uint32_t ThinFilmThicknessTexture = UINT32_MAX;
            uint32_t ThinFilmIORTexture = UINT32_MAX;

            uint32_t EmissionTexture = UINT32_MAX;
            uint32_t EmissionColorTexture = UINT32_MAX;

            uint32_t OpacityTexture = UINT32_MAX;
            uint32_t ThinWalledTexture = UINT32_MAX;
        };
    }
}