#include <cstdio>
#include "cmdlib.h"
#include "i_interface.h"

void I_PutInClipboard (const char *str)
{
	printf("I_PutInClipboard('%s')\n", str);
}

FString I_GetFromClipboard (bool use_primary_selection)
{
	printf("I_GetFromClipboard(...);\n");
	return "";
}

FString I_GetCWD()
{
	return "";
}

bool I_ChDir(const char* path)
{
	printf("I_ChDir('%s')\n", path);
	return false;
}

void I_DetectOS(void)
{
}

void I_SetIWADInfo()
{
}

void CalculateCPUSpeed()
{
}

int I_PickIWad(WadStuff* const wads, const int numwads, const bool showwin, const int defaultiwad, int&)
{
	return 0;
}

void I_PrintStr(const char *cp)
{
	printf("%s\n", cp);
}

void I_ShowFatalError(const char *message)
{
	printf("Fatal Error: %s\n", message);
}

TArray<FString> I_GetSteamPath()
{
	return TArray<FString>();
}

TArray<FString> I_GetGogPaths()
{
	return TArray<FString>();
}
