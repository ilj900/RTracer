#pragma once

#include "common_defines.h"

enum class EOutputType {
	Color 								= AOV_COLOR,
	ShadingNormal						= AOV_SHADING_NORMAL,
	GeometricNormal						= AOV_GEOMETRIC_NORMAL,
	UV 									= AOV_UV_COORDS,
	WorldSpacePosition 					= AOV_WORLD_SPACE_POSITION,
	Opacity								= AOV_OPACITY,
	Depth								= AOV_DEPTH,
	MaterialBaseWeight					= AOV_MATERIAL_ALBEDO_WEIGHT,
	MaterialBaseColor					= AOV_MATERIAL_ALBEDO,
	MaterialDiffuseRoughness			= AOV_MATERIAL_DIFFUSE_ROUGHNESS,
	MaterialMetalness					= AOV_MATERIAL_METALNESS,
	MaterialNormal						= AOV_MATERIAL_NORMAL,
	MaterialSpecularWeight				= AOV_MATERIAL_SPECULAR_WEIGHT,
	MaterialSpecularColor				= AOV_MATERIAL_SPECULAR_COLOR,
	MaterialSpecularRoughness			= AOV_MATERIAL_SPECULAR_ROUGHNESS,
	MaterialSpecularIOR					= AOV_MATERIAL_SPECULAR_IOR,
	MaterialSpecularAnisotropy			= AOV_MATERIAL_SPECULAR_ANISOTROPY,
	MaterialSpecularRotation			= AOV_MATERIAL_SPECULAR_ROTATION,
	MaterialTransmissionWeight			= AOV_MATERIAL_TRANSMISSION_WEIGHT,
	MaterialTransmissionColor			= AOV_MATERIAL_TRANSMISSION_COLOR,
	MaterialTransmissionDepth			= AOV_MATERIAL_TRANSMISSION_DEPTH,
	MaterialTransmissionScatter			= AOV_MATERIAL_TRANSMISSION_SCATTER,
	MaterialTransmissionAnisotropy		= AOV_MATERIAL_TRANSMISSION_ANISOTROPY,
	MaterialTransmissionDispersion		= AOV_MATERIAL_TRANSMISSION_DISPERSION,
	MaterialTransmissionRoughness		= AOV_MATERIAL_TRANSMISSION_ROUGHNESS,
	MaterialSubsurfaceWeight			= AOV_MATERIAL_SUBSURFACE_WEIGHT,
	MaterialSubsurfaceColor				= AOV_MATERIAL_SUBSURFACE_COLOR,
	MaterialSubsurfaceRadius			= AOV_MATERIAL_SUBSURFACE_RADIUS,
	MaterialSubsurfaceScale				= AOV_MATERIAL_SUBSURFACE_SCALE,
	MaterialSubsurfaceAnisotropy		= AOV_MATERIAL_SUBSURFACE_ANISOTROPY,
	MaterialSheenWeight					= AOV_MATERIAL_SHEEN_WEIGHT,
	MaterialSheenColor					= AOV_MATERIAL_SHEEN_COLOR,
	MaterialSheenRoughness				= AOV_MATERIAL_SHEEN_ROUGHNESS,
	MaterialCoatWeight					= AOV_MATERIAL_COAT_WEIGHT,
	MaterialCoatColor					= AOV_MATERIAL_COAT_COLOR,
	MaterialCoatRoughness				= AOV_MATERIAL_COAT_ROUGHNESS,
	MaterialCoatAnisotropy				= AOV_MATERIAL_COAT_ANISOTROPY,
	MaterialCoatRotation				= AOV_MATERIAL_COAT_ROTATION,
	MaterialCoatIOR						= AOV_MATERIAL_COAT_IOR,
	MaterialCoatNormal					= AOV_MATERIAL_COAT_NORMAL,
	MaterialCoatAffectColor				= AOV_MATERIAL_COAT_AFFECT_COLOR,
	MaterialCoatAffectRoughness			= AOV_MATERIAL_COAT_AFFECT_ROUGHNESS,
	MaterialThinFilmThickness			= AOV_MATERIAL_THIN_FILM_THICKNESS,
	MaterialThinFilmIOR					= AOV_MATERIAL_THIN_FILM_IOR,
	MaterialEmissionWeight				= AOV_MATERIAL_EMISSION_WEIGHT,
	MaterialEmissionColor				= AOV_MATERIAL_COLOR,
	MaterialOpacity						= AOV_MATERIAL_OPACITY,
	MaterialThinWalled					= AOV_MATERIAL_THIN_WALLED,
	Luminance							= AOV_LUMINANCE,
	MaterialID 							= AOV_MATERIAL_INDEX,
	RenderableID						= AOV_RENDERABLE_INDEX,
	PrimitiveID							= AOV_PRIMITIVE_INDEX,
	DebugLayer0							= AOV_DEBUG_LAYER_0,
	DebugLayer1							= AOV_DEBUG_LAYER_1,
	DebugLayer2							= AOV_DEBUG_LAYER_2,
	DebugLayer3							= AOV_DEBUG_LAYER_3,
	DebugLayer4							= AOV_DEBUG_LAYER_4,
	DebugLayer5							= AOV_DEBUG_LAYER_5,
	DebugLayer6							= AOV_DEBUG_LAYER_6,
	DebugLayer7							= AOV_DEBUG_LAYER_7,
	Max 								= AOV_MAX};

struct FRenderState
{
	EOutputType RenderTarget = EOutputType::Color;
};

FRenderState* GetRenderState();

#define RENDER_STATE() GetRenderState()