#include "gs_renderstate.h"
#include "gs_buffers.h"
#include "gs_hwtexture.h"
#include "shaderuniforms.h"
#include "hw_viewpointuniforms.h"
#include <gsInline.h>
#include "gsKit_ext.h"

struct CLIP_VERTEX
{
	FVector4 position;
	FVector2 texCoord;
};

static const int32_t MAX_VERTICES = 0x1000;
static GSPRIMSTQPOINT g_gsVertices[MAX_VERTICES];
static CLIP_VERTEX g_clipVertices[MAX_VERTICES];

static FVector4 TransformVector4(const VSMatrix& mat, const FVector4& vec)
{
	auto ind = mat.get();
	FVector4 res(
		(ind[0]  * vec.X) + (ind[4]  * vec.Y) + (ind[8]  * vec.Z) + (ind[12] * vec.W),
		(ind[1]  * vec.X) + (ind[5]  * vec.Y) + (ind[9]  * vec.Z) + (ind[13] * vec.W),
		(ind[2]  * vec.X) + (ind[6]  * vec.Y) + (ind[10] * vec.Z) + (ind[14] * vec.W),
		(ind[3]  * vec.X) + (ind[7]  * vec.Y) + (ind[11] * vec.Z) + (ind[15] * vec.W)
	);
	return res;
}

static void PrintVector(const FVector4& vec)
{
	printf("x: %f, y: %f, z: %f, w: %f\n", vec.X, vec.Y, vec.Z, vec.W);
}

static void PrintMatrix(const VSMatrix& mat)
{
	auto ind = mat.get();
	printf("0  : %f, 1  : %f, 2 : %f, 3  : %f\n", ind[0], ind[1], ind[2], ind[3]);
	printf("4  : %f, 5  : %f, 6 : %f, 7  : %f\n", ind[4], ind[5], ind[6], ind[7]);
	printf("8  : %f, 9  : %f, 10 :%f, 11 : %f\n", ind[8], ind[9], ind[10], ind[11]);
	printf("12 : %f, 13 : %f, 14 :%f, 15 : %f\n", ind[12], ind[13], ind[14], ind[15]);
}

GsRenderState::GsRenderState(GSGLOBAL* gsContext)
	: m_gsContext(gsContext)
{
}

void GsRenderState::ClearScreen()
{
}

void GsRenderState::Draw(int dt, int index, int count, bool apply)
{
	assert(mVertexBuffer);
	if(dt != DT_Triangles)
	{
		return;
	}
	//if(count != 60)
	//{
	//	return;
	//}

	assert(m_viewpointBuffer);
	assert(dt == DT_Triangles);
	assert(count <= MAX_VERTICES);

	GsHwTexture* hwTex = nullptr;
	if(mMaterial.mChanged)
	{
		MaterialLayerInfo* layer = nullptr;
		hwTex = static_cast<GsHwTexture*>(mMaterial.mMaterial->GetLayer(0, mMaterial.mTranslation, &layer));
		auto tex = mMaterial.mMaterial->Source();
		assert(tex);
		hwTex->BindOrCreate(tex->GetTexture(), 0, mMaterial.mClampMode, mMaterial.mTranslation, layer->scaleFlags);
	}

	//Transform all vertices
	{
		const auto* viewpointUniforms = reinterpret_cast<const HWViewpointUniforms*>(m_viewpointBuffer);
		mVertexBuffer->Map();
		auto vertexBuffer = static_cast<GsVertexBuffer*>(mVertexBuffer);
		auto vertices = reinterpret_cast<uint8_t*>(mVertexBuffer->Memory());
		for(int i = 0; i < count; i++)
		{
			auto& clipVertex = g_clipVertices[i];
			uint32_t vidx = index + i;
			auto vtxBase = vertices + (vertexBuffer->m_stride * vidx);
			float* position = reinterpret_cast<float*>(vtxBase + vertexBuffer->m_positionOffset);
			float* texCoord = reinterpret_cast<float*>(vtxBase + vertexBuffer->m_texCoordOffset);
			auto inputPos = FVector4(position[0], position[1], position[2], 1);
			auto eyeCoordPos = TransformVector4(viewpointUniforms->mViewMatrix, inputPos);
			clipVertex.position = TransformVector4(viewpointUniforms->mProjectionMatrix, eyeCoordPos);
			clipVertex.texCoord = FVector2(texCoord[0], texCoord[1]);
		}
		mVertexBuffer->Unmap();
	}

	static const float g_nearClip = 1.f;
	uint32_t gsVertexCount = 0;

	const auto makeClippedVertex =
		[](const CLIP_VERTEX& v0, const CLIP_VERTEX& v1)
		{
			const auto& p0 = v0.position;
			const auto& p1 = v1.position;
			const auto& t0 = v0.texCoord;
			const auto& t1 = v1.texCoord;
			float factor = (g_nearClip - p1.W) / (p0.W - p1.W);
			CLIP_VERTEX clipped;
			clipped.position = (factor * (p0 - p1)) + p1;
			clipped.texCoord = (factor * (t0 - t1)) + t1;
			return clipped;
		};

	const auto registerVertex = 
		[&](const FVector4& position, const FVector2& texCoord)
		{
			assert(gsVertexCount < MAX_VERTICES);
			auto& gsVertex = g_gsVertices[gsVertexCount++];
			gsVertex.rgbaq = color_to_RGBAQ(0x80, 0x80, 0x80, 0x80, 1.f / position.W);
			gsVertex.xyz2 = vertex_to_XYZ_clip(m_gsContext, 
				((( position.X / position.W) + 1.0f) / 2.0f) * 640.f,
				(((-position.Y / position.W) + 1.0f) / 2.0f) * 448.f,
				position.Z, false);
			gsVertex.stq = vertex_to_STQ(texCoord.X / position.W, texCoord.Y / position.W);
		};

	const auto singleClipTriangle =
		[&](int inIndex, int outIndex0, int outIndex1)
		{
			const auto& inVertex = g_clipVertices[inIndex];
			const auto& outVertex0 = g_clipVertices[outIndex0];
			const auto& outVertex1 = g_clipVertices[outIndex1];
			auto clippedVertex0 = makeClippedVertex(inVertex, outVertex0);
			auto clippedVertex1 = makeClippedVertex(inVertex, outVertex1);
			registerVertex(inVertex.position, inVertex.texCoord);
			registerVertex(clippedVertex0.position, clippedVertex0.texCoord);
			registerVertex(clippedVertex1.position, clippedVertex1.texCoord);
		};

	const auto doubleClipTriangle =
		[&](int inIndex0, int inIndex1, int outIndex)
		{
			const auto& inVertex0 = g_clipVertices[inIndex0];
			const auto& inVertex1 = g_clipVertices[inIndex1];
			const auto& outVertex = g_clipVertices[outIndex];
			auto clippedVertex0 = makeClippedVertex(inVertex0, outVertex);
			auto clippedVertex1 = makeClippedVertex(inVertex1, outVertex);

#if 0
			//assert(inVertex0.position.W > nearClip);
			//assert(inVertex1.position.W > nearClip);
			//assert(outVertex.position.W <= nearClip);

			if((inVertex0.position.W <= g_nearClip) ||
				(inVertex1.position.W <= g_nearClip) ||
				(outVertex.position.W > g_nearClip))
			{
				printf("inVertex0: "); PrintVector(inVertex0.position);
				printf("inVertex1: "); PrintVector(inVertex1.position);
				printf("outVertex: "); PrintVector(outVertex.position);
				printf("Clipped vertex0: "); PrintVector(clippedVertex0);
				printf("Clipped vertex1: "); PrintVector(clippedVertex1);
				assert(false);
			}
#endif

			//Triangle 0
			registerVertex(inVertex0.position, inVertex0.texCoord);
			registerVertex(inVertex1.position, inVertex1.texCoord);
			registerVertex(clippedVertex0.position, clippedVertex0.texCoord);

			//Triangle 1
			registerVertex(clippedVertex0.position, clippedVertex0.texCoord);
			registerVertex(clippedVertex1.position, clippedVertex1.texCoord);
			registerVertex(inVertex1.position, inVertex1.texCoord);
		};

	//Cull/clip triangles
	for(int i = 0; i < count; i += 3)
	{
		//Cull triangles that have all their points outside.
		uint32_t clipFlag = 0;
		for(int j = 0; j < 3; j++)
		{
			const auto& clipVertex = g_clipVertices[i + j];
			const auto& clipPosition = clipVertex.position;
			float absW = fabs(clipPosition.W);
			//Cheap simulation of CLIP instruction
			clipFlag <<= 6;
			//clipFlag |= (clipPosition.X >  absW) ? 0x01 : 0;
			//clipFlag |= (clipPosition.X < -absW) ? 0x02 : 0;
			//clipFlag |= (clipPosition.Y >  absW) ? 0x04 : 0;
			//clipFlag |= (clipPosition.Y < -absW) ? 0x08 : 0;
			//clipFlag |= (clipPosition.Z >  absW) ? 0x10 : 0;
			//clipFlag |= (clipPosition.Z < -absW) ? 0x20 : 0;
			clipFlag |= (clipPosition.W <= g_nearClip) ? 0x01 : 0;
		}
		
		//if(clipFlag != 0) continue;

//		if((clipFlag & 0x01041) == 0x01041) continue; //All X > +W
//		if((clipFlag & 0x02082) == 0x02082) continue; //All X < -W
//		if((clipFlag & 0x04104) == 0x04104) continue; //All Y > +W
//		if((clipFlag & 0x08208) == 0x08208) continue; //All Y < -W
//		if((clipFlag & 0x10410) == 0x10410) continue; //All Z > +W
//		if((clipFlag & 0x20820) == 0x20820) continue; //All Z < -W

		switch(clipFlag & 0b000001'000001'000001)
		{
		case 0b000000'000000'000000:
			//All points in
			for(int j = 0; j < 3; j++)
			{
				const auto& clipVertex = g_clipVertices[i + j];
				const auto& clipPosition = clipVertex.position;
				const auto& clipTexCoord = clipVertex.texCoord;
				registerVertex(clipPosition, clipTexCoord);
			}
			break;
		case 0b000001'000001'000001:
			//All points out
			break;
		case 0b000000'000001'000001:
			singleClipTriangle(i + 0, i + 1, i + 2);
			break;
		case 0b000001'000000'000001:
			singleClipTriangle(i + 1, i + 0, i + 2);
			break;
		case 0b000001'000001'000000:
			singleClipTriangle(i + 2, i + 0, i + 1);
			break;
		case 0b000000'000000'000001:
			doubleClipTriangle(i + 0, i + 1, i + 2);
			break;
		case 0b000000'000001'000000:
			doubleClipTriangle(i + 0, i + 2, i + 1);
			break;
		case 0b000001'000000'000000:
			doubleClipTriangle(i + 1, i + 2, i + 0);
			break;
		}
#if 0
		for(int j = 0; j < 3; j++)
		{
			const auto& clipVertex = g_clipVertices[i + j];
			const auto& clipPosition = clipVertex.position;
			const auto& clipTexCoord = clipVertex.texCoord;
			registerVertex(clipPosition, clipTexCoord);
		}
#endif
	}

	if(hwTex != nullptr)
	{
		gsKit_prim_list_triangle_goraud_texture_stq_3d(m_gsContext, hwTex->GetHandle(), gsVertexCount, g_gsVertices);
	}
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
		if(hwTex)
		{
			auto tex = mMaterial.mMaterial->Source();
			assert(tex);
			hwTex->BindOrCreate(tex->GetTexture(), 0, mMaterial.mClampMode, mMaterial.mTranslation, layer->scaleFlags);
		}
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
	if(targets & (CT_Color | CT_Depth))
	{
		gsKit_clear(m_gsContext, 0);
	}
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
