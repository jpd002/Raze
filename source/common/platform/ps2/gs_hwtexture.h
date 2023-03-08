#pragma once

#include <gsKit.h>
#include "hw_ihwtexture.h"

class GsHwTexture : public IHardwareTexture
{
public:
	GsHwTexture(GSGLOBAL*);
	virtual ~GsHwTexture() = default;

	void AllocateBuffer(int w, int h, int texelsize) override;
	uint8_t *MapBuffer() override;
	unsigned int CreateTexture(unsigned char* buffer, int w, int h, int texunit, bool mipmap, const char* name) override;

	GSTEXTURE* GetHandle();
	void BindOrCreate(FTexture *tex, int texunit, int clampmode, int translation, int flags);

private:
	GSGLOBAL* m_gsContext = nullptr;
	bool m_created = false;
	GSTEXTURE m_texture;
};
