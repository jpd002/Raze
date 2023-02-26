#include "st_start.h"
#include "cmdlib.h"

class FPS2StartupScreen : public FStartupScreen
{
public:
	FPS2StartupScreen(int max_progress)
		: FStartupScreen(max_progress)
	{
	}

	void AppendStatusLine(const char* status) override
	{
		printf("%s\n", status);
	}

	void LoadingStatus(const char* message, int colors) override
	{
		printf("Loading Status: %s\n", message);
	}
};

FStartupScreen *FStartupScreen::CreateInstance(int max_progress, bool showprogress)
{
	return new FPS2StartupScreen(max_progress);
}
