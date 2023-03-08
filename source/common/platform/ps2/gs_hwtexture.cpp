#include "gs_hwtexture.h"
#include <cassert>
#include "textures.h"

GsHwTexture::GsHwTexture(GSGLOBAL* gsContext)
	: m_gsContext(gsContext)
{
}

void GsHwTexture::AllocateBuffer(int w, int h, int texelsize)
{
	assert(false);
}

uint8_t* GsHwTexture::MapBuffer()
{
	assert(false);
	return nullptr;
}

unsigned int GsHwTexture::CreateTexture(unsigned char* buffer, int w, int h, int texunit, bool mipmap, const char* name)
{
	m_texture.Delayed = GS_SETTING_ON;
	m_texture.Width = w;
	m_texture.Height = h;
	m_texture.PSM = GS_PSM_CT32;
	u32 texSize = gsKit_texture_size_ee(m_texture.Width, m_texture.Height, m_texture.PSM);
	m_texture.Mem = reinterpret_cast<u32*>(memalign(128, texSize));

	//if(Texture->PSM != GS_PSM_T8 && Texture->PSM != GS_PSM_T4)
	{
		m_texture.VramClut = 0;
		m_texture.Clut = NULL;
	}

	memcpy(m_texture.Mem, buffer, texSize);
	
	//Input is BGRA, swap R and B
	for(u32 i = 0; i < texSize; i += 4)
	{
		std::swap(reinterpret_cast<uint8_t*>(m_texture.Mem)[i + 0], reinterpret_cast<uint8_t*>(m_texture.Mem)[i + 2]);
	}

	return 1;
}

GSTEXTURE* GsHwTexture::GetHandle()
{
	return &m_texture;
}

void GsHwTexture::BindOrCreate(FTexture *tex, int texunit, int clampmode, int translation, int flags)
{
	assert(!tex->isHardwareCanvas());

	if(!m_created)
	{
		FTextureBuffer texbuffer = tex->CreateTexBuffer(translation, flags | CTF_ProcessData);
		int w = texbuffer.mWidth;
		int h = texbuffer.mHeight;
		bool needmipmap = false;

		CreateTexture(texbuffer.mBuffer, w, h, texunit, needmipmap, "GsHwTexture.BindOrCreate");
		m_created = true;
	}

	gsKit_TexManager_bind(m_gsContext, &m_texture);
}
