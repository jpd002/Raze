#include "gs_renderstate.h"
#include "gs_buffers.h"
#include <gsInline.h>

static const int32_t MAX_VERTICES = 0x1000;
static GSPRIMPOINT g_gsVertices[MAX_VERTICES];

GsRenderState::GsRenderState(GSGLOBAL* gsContext)
	: m_gsContext(gsContext)
{
}

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
	assert(mVertexBuffer);
	assert(dt == DT_Triangles);
	assert(count <= MAX_VERTICES);
	static uint8_t color = 0;
	mIndexBuffer->Map();
	mVertexBuffer->Map();
	auto vertexBuffer = static_cast<GsVertexBuffer*>(mVertexBuffer);
	auto vertices = reinterpret_cast<uint8_t*>(mVertexBuffer->Memory());
	auto indices = reinterpret_cast<uint32_t*>(mIndexBuffer->Memory());
	for(int i = 0; i < count; i++)
	{
		uint32_t vidx = indices[index + i];
		auto vtxBase = vertices + (vertexBuffer->m_stride * vidx);
		float* position = reinterpret_cast<float*>(vtxBase + vertexBuffer->m_positionOffset);
		auto& gsVertex = g_gsVertices[i];
		gsVertex.rgbaq = color_to_RGBAQ(color, color, color, 0xFF, 0.0f);
		gsVertex.xyz2 = vertex_to_XYZ2(m_gsContext, position[0], position[1], position[2]);
		//printf("%f, %f, %f\n", position[0], position[1], position[2]);
	}
	color += 0x10;
	mIndexBuffer->Unmap();
	mVertexBuffer->Unmap();
	gsKit_prim_list_triangle_gouraud_3d(m_gsContext, count, g_gsVertices);
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
