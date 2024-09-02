#include "renderer_options.h"

FRenderState* RenderState = nullptr;

FRenderState* GetRenderState()
{
	if (RenderState == nullptr)
	{
		RenderState = new FRenderState();
	}

	return RenderState;
}