#include "emulator.h"

int main(int argc, char* argv[])
{
	Emulator emu(const_cast<const char*>(argv[1]));

	emu.Run();
	return 0;
}
