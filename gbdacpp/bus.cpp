#include "bus.h"

/*
	For now we only think about memory as a plain array.
*/

void Bus::Write(const u16 addr, const u8 val)
{
	memory[addr] = val;
}

u8 Bus::Read(const u16 addr)
{
	if (addr >= 0x0000 && addr <= 0x7FFF) {
		return rom->Read(addr);
	} else if (addr == 0xFF44)
		return 0x90;
	return memory[addr];
}

Bus::Bus(Rom* pRom) : rom(pRom)
{

}

Bus::~Bus()
{

}
