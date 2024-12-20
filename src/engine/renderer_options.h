#pragma once

enum class EOutputType {Color = 0, Normal = 1, UV = 2, WorldSpacePosition = 3, DebugLayer = 4, Max = 5};

struct FRenderState
{
	EOutputType RenderTarget = EOutputType::Color;
};

FRenderState* GetRenderState();

#define RENDER_STATE() GetRenderState()