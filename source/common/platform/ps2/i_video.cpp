#include "cmdlib.h"
#include "i_video.h"
#include "hw_cvars.h"
#include "gs_framebuffer.h"

CVAR(Int, gl_multisample, 1, CVAR_ARCHIVE|CVAR_GLOBALCONFIG);

class GsVideo : public IVideo
{
public:
	DFrameBuffer* CreateFrameBuffer() override
	{
		return new GsFrameBuffer();
	}
};

IVideo* Video = nullptr;

void I_InitGraphics()
{
	Video = new GsVideo();
}

void I_ShutdownGraphics()
{
	delete Video;
	Video = nullptr;
}
