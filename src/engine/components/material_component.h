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

				EmissionWeight = 0;
                EmissionColor = {1, 1, 1};

                Opacity = {1, 1, 1};
                ThinWalled = false;

            	BaseWeightTexture = UINT32_MAX;
            	BaseColorTexture = UINT32_MAX;
            	DiffuseRoughnessTexture = UINT32_MAX;
            	MetalnessTexture = UINT32_MAX;

				NormalTexture = UINT32_MAX;
            	SpecularWeightTexture = UINT32_MAX;
            	SpecularColorTexture = UINT32_MAX;
            	SpecularRoughnessTexture = UINT32_MAX;

				SpecularIORTexture = UINT32_MAX;
            	SpecularAnisotropyTexture = UINT32_MAX;
            	SpecularRotationTexture = UINT32_MAX;
            	TransmissionWeightTexture = UINT32_MAX;

				TransmissionColorTexture = UINT32_MAX;
            	TransmissionDepthTexture = UINT32_MAX;
            	TransmissionScatterTexture = UINT32_MAX;
            	TransmissionAnisotropyTexture = UINT32_MAX;

				TransmissionDispersionTexture = UINT32_MAX;
            	TransmissionRoughnessTexture = UINT32_MAX;
            	SubsurfaceWeightTexture = UINT32_MAX;
            	SubsurfaceColorTexture = UINT32_MAX;

				SubsurfaceRadiusTexture = UINT32_MAX;
            	SubsurfaceScaleTexture = UINT32_MAX;
            	SubsurfaceAnisotropyTexture = UINT32_MAX;
            	SheenWeightTexture = UINT32_MAX;

				SheenColorTexture = UINT32_MAX;
            	SheenRoughnessTexture = UINT32_MAX;
            	CoatWeightTexture = UINT32_MAX;
            	CoatColorTexture = UINT32_MAX;

				CoatRoughnessTexture = UINT32_MAX;
            	CoatAnisotropyTexture = UINT32_MAX;
            	CoatRotationTexture = UINT32_MAX;
            	CoatIORTexture = UINT32_MAX;

				CoatNormalTexture = UINT32_MAX;
            	CoatAffectColorTexture = UINT32_MAX;
            	CoatAffectRoughnessTexture = UINT32_MAX;
            	ThinFilmThicknessTexture = UINT32_MAX;

				ThinFilmIORTexture = UINT32_MAX;
            	EmissionWeightTexture = UINT32_MAX;
            	EmissionColorTexture = UINT32_MAX;
            	OpacityTexture = UINT32_MAX;

				ThinWalledTexture = UINT32_MAX;
            };
        };
    }
}