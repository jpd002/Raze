#include "gs_renderstate.h"
#include "buffers.h"

void GsRenderState::ClearScreen()
{
}

void GsRenderState::Draw(int dt, int index, int count, bool apply)
{
	printf("Draw(dt = %d, index = %d, count = %d, apply = %d);\n",
		dt, index, count, apply);
}

void GsRenderState::DrawIndexed(int dt, int index, int count, bool apply)
{
	printf("DrawIndexed(dt = %d, index = %d, count = %d, apply = %d);\n",
		dt, index, count, apply);
	assert(mIndexBuffer);
	mIndexBuffer->Map();
	auto indices = reinterpret_cast<uint32_t*>(mIndexBuffer->Memory());
	for(int i = 0; i < count; i++)
	{
		printf("%d, ", indices[index + i]);
	}
	printf("\n");
	mIndexBuffer->Unmap();
}

bool GsRenderState::SetDepthClamp(bool on)
{
	return false;
}

void GsRenderState::SetDepthMask(bool on)
{
}

void GsRenderState::SetDepthFunc(int func)
{
}

void GsRenderState::SetDepthRange(float min, float max)
{
}

void GsRenderState::SetColorMask(bool r, bool g, bool b, bool a)
{
}

void GsRenderState::SetStencil(int offs, int op, int flags)
{
}

void GsRenderState::SetCulling(int mode)
{
}

void GsRenderState::EnableClipDistance(int num, bool state)
{
}

void GsRenderState::Clear(int targets)
{
}

void GsRenderState::EnableStencil(bool on)
{
}

void GsRenderState::SetScissor(int x, int y, int w, int h)
{
	printf("SetScissor(x: %d, y: %d, w: %d, h: %d);\n", x, y, w, h);
}

void GsRenderState::SetViewport(int x, int y, int w, int h)
{
	printf("SetViewport(x: %d, y: %d, w: %d, h: %d);\n", x, y, w, h);
}

void GsRenderState::EnableDepthTest(bool on)
{
}

void GsRenderState::EnableMultisampling(bool on)
{
}

void GsRenderState::EnableLineSmooth(bool on)
{
}

void GsRenderState::EnableDrawBuffers(int count, bool apply)
{
}
