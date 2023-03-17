#include "m_argv.h"
#include "cmdlib.h"

int GameMain();

FArgs *Args;

void SetupPad();

int main(int argc, char** argv)
{
	progdir = "host:/raze";
	SetupPad();
	const int result = GameMain();
	return result;
}

void I_SetWindowTitle(const char* caption)
{
	printf("I_SetWindowTitle('%s');\n", caption);
}

void UpdateVRModes(bool)
{
}

extern "C"
unsigned int __atomic_fetch_add_4(volatile void* mem, unsigned int val, int model)
{
	unsigned int prevValue = *reinterpret_cast<volatile unsigned int*>(mem);
	*reinterpret_cast<volatile unsigned int*>(mem) += val;
	return prevValue;
}
