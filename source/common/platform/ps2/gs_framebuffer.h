#pragma once

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
};
