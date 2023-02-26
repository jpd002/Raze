#include <cstdio>
#include "cmdlib.h"
#include "m_joy.h"

CVAR (Bool,  use_mouse,				true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)

void I_GetEvent()
{
	printf("I_GetEvent();\n");
}

void I_StartTic()
{
}

void I_StartFrame()
{
}

void I_SetMouseCapture()
{
}

void I_ReleaseMouseCapture()
{
}

void I_ShutdownInput()
{
}

void I_GetJoysticks(TArray<IJoystickConfig*>& sticks)
{
}

void I_GetAxes(float axes[NUM_JOYAXIS])
{
}

IJoystickConfig* I_UpdateDeviceList()
{
	return nullptr;
}
