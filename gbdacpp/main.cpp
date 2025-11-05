#include "emulator.h"

int main(int argc, char* argv[])
{
	Emulator emu(const_cast<const char *>(argv[1]));

	if (emu.Load(argv[1]) == STT_FAILED)
		return EXIT_FAILURE;
	emu.Run();
	return 0;
}
