#pragma once

#include "common_defines.h"

enum class EOutputType
{
	Color                                       = AOV_COLOR,
	ShadingNormal                        		= AOV_SHADING_NORMAL,
	GeometricNormal                      		= AOV_GEOMETRIC_NORMAL,
	UV                                   		= AOV_UV_COORDS,
	WorldSpacePosition                   		= AOV_WORLD_SPACE_POSITION,
	Opacity                              		= AOV_OPACITY,
	Depth                                		= AOV_DEPTH,
	// Base
	MaterialBaseWeight                   		= AOV_MATERIAL_BASE_WEIGHT,
	MaterialBaseColor                    		= AOV_MATERIAL_BASE_COLOR,
	MaterialBaseMetalness                		= AOV_MATERIAL_BASE_METALNESS,
	MaterialBaseDiffuseRoughness         		= AOV_MATERIAL_BASE_DIFFUSE_ROUGHNESS,
	// Specular
	MaterialSpecularWeight               		= AOV_MATERIAL_SPECULAR_WEIGHT,
	MaterialSpecularColor                		= AOV_MATERIAL_SPECULAR_COLOR,
	MaterialSpecularRoughness            		= AOV_MATERIAL_SPECULAR_ROUGHNESS,
	MaterialSpecularRoughnessAnisotropy  		= AOV_MATERIAL_SPECULAR_ROUGHNESS_ANISOTROPY,
	MaterialSpecularIOR                  		= AOV_MATERIAL_SPECULAR_IOR,
	// Transmission
	MaterialTransmissionWeight           		= AOV_MATERIAL_TRANSMISSION_WEIGHT,
	MaterialTransmissionColor            		= AOV_MATERIAL_TRANSMISSION_COLOR,
	MaterialTransmissionDepth            		= AOV_MATERIAL_TRANSMISSION_DEPTH,
	MaterialTransmissionScatter          		= AOV_MATERIAL_TRANSMISSION_SCATTER,
	MaterialTransmissionScatterAnisotropy		= AOV_MATERIAL_TRANSMISSION_SCATTER_ANISOTROPY,
	MaterialTransmissionDispersionScale  		= AOV_MATERIAL_TRANSMISSION_DISPERSION_SCALE,
	MaterialTransmissionDispersionAbbeNumber	= AOV_MATERIAL_TRANSMISSION_DISPERSION_ABBE_NUMBER,
	// Subsurface
	MaterialSubsurfaceWeight                    = AOV_MATERIAL_SUBSURFACE_WEIGHT,
	MaterialSubsurfaceColor                     = AOV_MATERIAL_SUBSURFACE_COLOR,
	MaterialSubsurfaceRadius                    = AOV_MATERIAL_SUBSURFACE_RADIUS,
	MaterialSubsurfaceRadiusScale               = AOV_MATERIAL_SUBSURFACE_RADIUS_SCALE,
	MaterialSubsurfaceScatterAnisotropy         = AOV_MATERIAL_SUBSURFACE_SCATTER_ANISOTROPY,
	// Coat
	MaterialCoatWeight                          = AOV_MATERIAL_COAT_WEIGHT,
	MaterialCoatColor                           = AOV_MATERIAL_COAT_COLOR,
	MaterialCoatRoughness                       = AOV_MATERIAL_COAT_ROUGHNESS,
	MaterialCoatRoughnessAnisotropy             = AOV_MATERIAL_COAT_ROUGHNESS_ANISOTROPY,
	MaterialCoatIOR                             = AOV_MATERIAL_COAT_IOR,
	MaterialCoatDarkening                       = AOV_MATERIAL_COAT_DARKENING,
	// Fuzz
	MaterialFuzzWeight                          = AOV_MATERIAL_FUZZ_WEIGHT,
	MaterialFuzzColor                           = AOV_MATERIAL_FUZZ_COLOR,
	MaterialFuzzRoughness                       = AOV_MATERIAL_FUZZ_ROUGHNESS,
	// Emission
	MaterialEmissionLuminance                   = AOV_MATERIAL_EMISSION_LUMINANCE,
	MaterialEmissionColor                       = AOV_MATERIAL_EMISSION_COLOR,
	// Thin film
	MaterialThinFilmWeight                      = AOV_MATERIAL_THIN_FILM_WEIGHT,
	MaterialThinFilmThickness                   = AOV_MATERIAL_THIN_FILM_THICKNESS,
	MaterialThinFilmIOR                         = AOV_MATERIAL_THIN_FILM_IOR,
	// Geometry
	MaterialGeometryOpacity                     = AOV_MATERIAL_GEOMETRY_OPACITY,
	MaterialGeometryThinWalled                  = AOV_MATERIAL_GEOMETRY_THIN_WALLED,
	MaterialGeometryNormal                      = AOV_MATERIAL_GEOMETRY_NORMAL,
	MaterialGeometryTangent                     = AOV_MATERIAL_GEOMETRY_TANGENT,
	MaterialGeometryCoatNormal                  = AOV_MATERIAL_GEOMETRY_COAT_NORMAL,
	MaterialGeometryCoatTangent                 = AOV_MATERIAL_GEOMETRY_COAT_TANGENT,
	// Misc
	Luminance                                   = AOV_LUMINANCE,
	BounceHeatmap                               = AOV_BOUNCE_HEATMAP,
	MaterialID                                  = AOV_MATERIAL_INDEX,
	RenderableID                                = AOV_RENDERABLE_INDEX,
	PrimitiveID                                 = AOV_PRIMITIVE_INDEX,
	DebugLayer0                                 = AOV_DEBUG_LAYER_0,
	DebugLayer1                                 = AOV_DEBUG_LAYER_1,
	DebugLayer2                                 = AOV_DEBUG_LAYER_2,
	DebugLayer3                                 = AOV_DEBUG_LAYER_3,
	DebugLayer4                                 = AOV_DEBUG_LAYER_4,
	DebugLayer5                                 = AOV_DEBUG_LAYER_5,
	DebugLayer6                                 = AOV_DEBUG_LAYER_6,
	DebugLayer7                                 = AOV_DEBUG_LAYER_7,
	Max                                         = AOV_MAX
};

struct FRenderState
{
	EOutputType RenderTarget = EOutputType::Color;
};

FRenderState* GetRenderState();

#define RENDER_STATE() GetRenderState()
