#pragma once

enum class EOutputType {
	Color 				= 0,
	Normal 				= 1,
	UV 					= 2,
	WorldSpacePosition 	= 3,
	MaterialID 			= 4,
	RenderableID		= 5,
	PrimitiveID			= 6,
	DebugLayer 			= 7,
	Max 				= 8};

struct FRenderState
{
	EOutputType RenderTarget = EOutputType::Color;
};

FRenderState* GetRenderState();

#define RENDER_STATE() GetRenderState()