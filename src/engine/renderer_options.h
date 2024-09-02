#pragma once

enum class EOutputType {Color = 0, Normal = 1, UV = 2, Max = 3};

struct FRenderState
{
	EOutputType RenderTarget = EOutputType::Normal;
};

FRenderState* GetRenderState();

#define RENDER_STATE() GetRenderState()