#include "cmdlib.h"
#include "i_specialpaths.h"

FString M_GetAutoexecPath()
{
	return FString();
}

FString M_GetAppDataPath(bool create)
{
	return "";
}

FString M_GetConfigPath(bool for_reading)
{
	return "";
}

FString M_GetScreenshotsPath()
{
	return "";
}

FString M_GetSavegamesPath()
{
	return "";
}

FString M_GetDocumentsPath()
{
	return "";
}

FString M_GetDemoPath()
{
	return "";
}

FString M_GetNormalizedPath(const char* path)
{
	return path;
}
