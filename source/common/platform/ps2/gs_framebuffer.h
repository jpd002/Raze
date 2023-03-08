#pragma once

#include <gsKit.h>
#include "v_video.h"
#include "gs_renderstate.h"

class GsFrameBuffer : public DFrameBuffer
{
	typedef DFrameBuffer Super;
public:
	GsFrameBuffer();
	virtual ~GsFrameBuffer() = default;

	void InitializeState() override;
	bool IsFullscreen() override;
	int GetClientWidth() override;
	int GetClientHeight() override;

	IVertexBuffer* CreateVertexBuffer() override;
	IIndexBuffer* CreateIndexBuffer() override;
	IDataBuffer* CreateDataBuffer(int bindingpoint, bool ssbo, bool needsresize) override;

	IHardwareTexture* CreateHardwareTexture(int numchannels) override;

	void Update() override;

private:
	GSGLOBAL* m_gsContext = nullptr;
	GsRenderState m_renderState;
};
