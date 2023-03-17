#include "gs_renderstate.h"
#include "gs_buffers.h"
#include "gs_hwtexture.h"
#include "shaderuniforms.h"
#include "hw_viewpointuniforms.h"
#include <gsInline.h>

static const int32_t MAX_VERTICES = 0x1000;
static GSPRIMSTQPOINT g_gsVertices[MAX_VERTICES];

GsRenderState::GsRenderState(GSGLOBAL* gsContext)
	: m_gsContext(gsContext)
{
}

void GsRenderState::ClearScreen()
{
}

void GsRenderState::Draw(int dt, int index, int count, bool apply)
{
}

void GsRenderState::DrawIndexed(int dt, int index, int count, bool apply)
{
	assert(mIndexBuffer);
	assert(mVertexBuffer);
	assert(dt == DT_Triangles);
	assert(count <= MAX_VERTICES);

	GsHwTexture* hwTex = nullptr;
	if(mMaterial.mChanged)
	{
		MaterialLayerInfo* layer = nullptr;
		hwTex = static_cast<GsHwTexture*>(mMaterial.mMaterial->GetLayer(0, mMaterial.mTranslation, &layer));
		auto tex = mMaterial.mMaterial->Source();
		hwTex->BindOrCreate(tex->GetTexture(), 0, mMaterial.mClampMode, mMaterial.mTranslation, layer->scaleFlags);
	}

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
		float* texCoord = reinterpret_cast<float*>(vtxBase + vertexBuffer->m_texCoordOffset);
		auto& gsVertex = g_gsVertices[i];
		gsVertex.rgbaq = color_to_RGBAQ(0x80, 0x80, 0x80, 0x80, 1.0f);
		gsVertex.xyz2 = vertex_to_XYZ2(m_gsContext, position[0], position[1], position[2]);
		gsVertex.stq = vertex_to_STQ(texCoord[0], texCoord[1]);
	}
	mIndexBuffer->Unmap();
	mVertexBuffer->Unmap();
	if(hwTex != nullptr)
	{
		gsKit_prim_list_triangle_goraud_texture_stq_3d(m_gsContext, hwTex->GetHandle(), count, g_gsVertices);
		//gsKit_prim_list_triangle_gouraud_3d(m_gsContext, count, g_gsVertices);
	}
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
	//printf("SetScissor(x: %d, y: %d, w: %d, h: %d);\n", x, y, w, h);
}

void GsRenderState::SetViewport(int x, int y, int w, int h)
{
	//printf("SetViewport(x: %d, y: %d, w: %d, h: %d);\n", x, y, w, h);
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

void GsRenderState::BindUniformBuffer(int bindingPoint, uint8_t* buffer, size_t bufferSize)
{
	//We're only interested by viewpoint uniforms for now
	if(bindingPoint != VIEWPOINT_BINDINGPOINT)
	{
		return;
	}
	assert(bufferSize >= sizeof(HWViewpointUniforms));
	m_viewpointBuffer = buffer;
	m_viewpointBufferSize = bufferSize;
}
