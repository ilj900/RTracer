#pragma once

enum class EOutputType {
	Color 				= 0,
	ShadingNormal		= 1,
	GeometricNormal		= 2,
	UV 					= 3,
	WorldSpacePosition 	= 4,
	Opacity				= 5,
	Depth				= 6,
	Albedo				= 7,
	Luminance			= 8,
	MaterialID 			= 9,
	RenderableID		= 10,
	PrimitiveID			= 11,
	DebugLayer0			= 12,
	DebugLayer1			= 13,
	DebugLayer2			= 14,
	DebugLayer3			= 15,
	Max 				= 16};

struct FRenderState
{
	EOutputType RenderTarget = EOutputType::Color;
};

FRenderState* GetRenderState();

#define RENDER_STATE() GetRenderState()