#include "cmdlib.h"
#include "i_video.h"
#include "hw_cvars.h"

CVAR(Int, gl_multisample, 1, CVAR_ARCHIVE|CVAR_GLOBALCONFIG);

IVideo *Video;

void I_InitGraphics()
{
}

void I_ShutdownGraphics()
{
}
