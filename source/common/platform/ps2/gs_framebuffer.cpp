#include "gs_framebuffer.h"

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 448

GsFrameBuffer::GsFrameBuffer()
	: DFrameBuffer(SCREEN_WIDTH, SCREEN_HEIGHT)
{
}

void GsFrameBuffer::InitializeState()
{
}

bool GsFrameBuffer::IsFullscreen()
{
	return true;
}

int GsFrameBuffer::GetClientWidth()
{
	return SCREEN_WIDTH;
}

int GsFrameBuffer::GetClientHeight()
{
	return SCREEN_HEIGHT;
}
