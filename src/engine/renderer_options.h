#pragma once

#include "common_defines.h"

enum class EOutputType {
	Color 				= AOV_COLOR,
	ShadingNormal		= AOV_SHADING_NORMAL,
	GeometricNormal		= AOV_GEOMETRIC_NORMAL,
	UV 					= AOV_UV_COORDS,
	WorldSpacePosition 	= AOV_WORLD_SPACE_POSITION,
	Opacity				= AOV_OPACITY,
	Depth				= AOV_DEPTH,
	Albedo				= AOV_MATERIAL_ALBEDO,
	Luminance			= AOV_LUMINANCE,
	MaterialID 			= AOV_MATERIAL_INDEX,
	RenderableID		= AOV_RENDERABLE_INDEX,
	PrimitiveID			= AOV_PRIMITIVE_INDEX,
	DebugLayer0			= AOV_DEBUG_LAYER_0,
	DebugLayer1			= AOV_DEBUG_LAYER_1,
	DebugLayer2			= AOV_DEBUG_LAYER_2,
	DebugLayer3			= AOV_DEBUG_LAYER_3,
	Max 				= AOV_MAX};

struct FRenderState
{
	EOutputType RenderTarget = EOutputType::Color;
};

FRenderState* GetRenderState();

#define RENDER_STATE() GetRenderState()