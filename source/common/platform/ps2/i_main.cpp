#include "m_argv.h"
#include "cmdlib.h"

int GameMain();

FArgs *Args;

int main(int argc, char** argv)
{
	const int result = GameMain();
	return result;
}

void I_SetWindowTitle(const char* caption)
{
	printf("I_SetWindowTitle('%s'\n);", caption);
}

void UpdateVRModes(bool)
{
}

extern "C"
unsigned int __atomic_fetch_add_4(volatile void* mem, unsigned int val, int model)
{
	return *reinterpret_cast<volatile unsigned int*>(mem) + val;
}
