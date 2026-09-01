#pragma once

#include "maths.h"
#include "common_structures.h"

namespace ECS
{
    namespace COMPONENTS
    {
        struct FOpenPBRSurfaceComponent : FOpenPBRSurface
        {
            FOpenPBRSurfaceComponent()
			{
			    // Base
			    BaseWeight = 1.f;
			    BaseColor = {0.8f, 0.8f, 0.8f};
			    BaseMetalness = 0.f;
			    BaseDiffuseRoughness = 0.f;

			    // Specular
			    SpecularWeight = 1.f;
			    SpecularColor = {1.f, 1.f, 1.f};
			    SpecularRoughness = 0.3f;
			    SpecularRoughnessAnisotropy = 0.f;
			    SpecularIor = 1.5f;

			    // Transmission
			    TransmissionWeight = 0.f;
			    TransmissionColor = {1.f, 1.f, 1.f};
			    TransmissionDepth = 0.f;
			    TransmissionScatter = {0.f, 0.f, 0.f};
			    TransmissionScatterAnisotropy = 0.f;
			    TransmissionDispersionScale = 0.f;
			    TransmissionDispersionAbbeNumber = 20.f;

			    // Subsurface
			    SubsurfaceWeight = 0.f;
			    SubsurfaceColor = {0.8f, 0.8f, 0.8f};
			    SubsurfaceRadius = 1.f;
			    SubsurfaceRadiusScale = {1.f, 0.5f, 0.25f};
			    SubsurfaceScatterAnisotropy = 0.f;

			    // Coat
			    CoatWeight = 0.f;
			    CoatColor = {1.f, 1.f, 1.f};
			    CoatRoughness = 0.f;
			    CoatRoughnessAnisotropy = 0.f;
			    CoatIor = 1.6f;
			    CoatDarkening = 1.f;

			    // Fuzz
			    FuzzWeight = 0.f;
			    FuzzColor = {1.f, 1.f, 1.f};
			    FuzzRoughness = 0.5f;

			    // Emission
			    EmissionLuminance = 0.f;
			    EmissionColor = {1.f, 1.f, 1.f};

			    // Thin film
			    ThinFilmWeight = 0.f;
			    ThinFilmThickness = 0.5f;
			    ThinFilmIor = 1.4f;

			    // Geometry
			    GeometryOpacity = 1.f;
			    GeometryThinWalled = 0u;
			    GeometryNormal = {0.f, 0.f, 0.f};        // zero = use unperturbed shading normal
			    GeometryTangent = {0.f, 0.f, 0.f};       // zero = use unperturbed tangent
			    GeometryCoatNormal = {0.f, 0.f, 0.f};
			    GeometryCoatTangent = {0.f, 0.f, 0.f};

			    // Texture indices
			    BaseWeightTextureIndex = UINT32_MAX;
			    BaseColorTextureIndex = UINT32_MAX;
			    BaseMetalnessTextureIndex = UINT32_MAX;
			    BaseDiffuseRoughnessTextureIndex = UINT32_MAX;

			    SpecularWeightTextureIndex = UINT32_MAX;
			    SpecularColorTextureIndex = UINT32_MAX;
			    SpecularRoughnessTextureIndex = UINT32_MAX;
			    SpecularRoughnessAnisotropyTextureIndex = UINT32_MAX;
			    SpecularIorTextureIndex = UINT32_MAX;

			    TransmissionWeightTextureIndex = UINT32_MAX;
			    TransmissionColorTextureIndex = UINT32_MAX;
			    TransmissionDepthTextureIndex = UINT32_MAX;
			    TransmissionScatterTextureIndex = UINT32_MAX;
			    TransmissionScatterAnisotropyTextureIndex = UINT32_MAX;
			    TransmissionDispersionScaleTextureIndex = UINT32_MAX;
			    TransmissionDispersionAbbeNumberTextureIndex = UINT32_MAX;

			    SubsurfaceWeightTextureIndex = UINT32_MAX;
			    SubsurfaceColorTextureIndex = UINT32_MAX;
			    SubsurfaceRadiusTextureIndex = UINT32_MAX;
			    SubsurfaceRadiusScaleTextureIndex = UINT32_MAX;
			    SubsurfaceScatterAnisotropyTextureIndex = UINT32_MAX;

			    CoatWeightTextureIndex = UINT32_MAX;
			    CoatColorTextureIndex = UINT32_MAX;
			    CoatRoughnessTextureIndex = UINT32_MAX;
			    CoatRoughnessAnisotropyTextureIndex = UINT32_MAX;
			    CoatIorTextureIndex = UINT32_MAX;
			    CoatDarkeningTextureIndex = UINT32_MAX;

			    FuzzWeightTextureIndex = UINT32_MAX;
			    FuzzColorTextureIndex = UINT32_MAX;
			    FuzzRoughnessTextureIndex = UINT32_MAX;

			    EmissionLuminanceTextureIndex = UINT32_MAX;
			    EmissionColorTextureIndex = UINT32_MAX;

			    ThinFilmWeightTextureIndex = UINT32_MAX;
			    ThinFilmThicknessTextureIndex = UINT32_MAX;
			    ThinFilmIorTextureIndex = UINT32_MAX;

			    GeometryOpacityTextureIndex = UINT32_MAX;
			    GeometryThinWalledTextureIndex = UINT32_MAX;
			    GeometryNormalTextureIndex = UINT32_MAX;
			    GeometryTangentTextureIndex = UINT32_MAX;
			    GeometryCoatNormalTextureIndex = UINT32_MAX;
			    GeometryCoatTangentTextureIndex = UINT32_MAX;
			}
        };
    }
}
