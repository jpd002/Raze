#pragma once

#include <gsKit.h>
#include "v_video.h"

class GsFrameBuffer : public DFrameBuffer
{
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

	void Update() override;

private:
	GSGLOBAL* m_gsContext = nullptr;
};
