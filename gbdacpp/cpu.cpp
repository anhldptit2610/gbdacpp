#include "cpu.h"

#define FMT_HEADER_ONLY
#include <fmt/core.h>

void Cpu::SetFlag(CpuFlag flag, bool val)
{
	regs.F() = (val) ? (regs.F() | flag) : (regs.F() & ~flag);
}

bool Cpu::GetFlag(CpuFlag flag)
{
	return (regs.F() & flag) != 0;
}

void Cpu::SetZNHC(bool z, bool n, bool h, bool c)
{
	SetFlag(FLAG_Z, z);
	SetFlag(FLAG_N, n);
	SetFlag(FLAG_H, h);
	SetFlag(FLAG_C, c);
}

void Cpu::StackPush(u8 val)
{
	bus->Write(regs.SP(), val);
	regs.SP()--;
}

u8 Cpu::StackPop()
{
	regs.SP()++;
	return bus->Read(regs.SP());
}

/* 
	Instruction: LD r16,u16
	Usage:		 Load U16 value into u16 registers(couple also). E.g. BC, DE, SP, PC
	Cost:		 3 CPU cycles
*/
int Cpu::LD_R16_U16(u16& reg, u16 val)
{
	reg = val;
	this->regs.PC() += 2;
	return 3;
}

/* 
	Instruction: XOR A,r8
	Usage:		 XOR r8 value with A register, then store the result in A
	Cost:		 1 CPU cycle
*/
int Cpu::XOR_A_R8(u8& r8)
{
	regs.A() ^= r8;
	SetZNHC(regs.A() == 0, 0, 0, 0);
	return 1;
}

/*
	Instruction: LD (HL-),A 
	Usage:		 Load the value of A into memory pointed by HL, then decrement HL. 
	Cost:		 2 CPU cycles
*/
int Cpu::LD_HLD_A()
{
	if (regs.HL() == 0xff04)
		spdlog::info("Write to 0xff04, data: {:02X}", regs.A());
	bus->Write(regs.HL(), regs.A());
	regs.HL() -= 1;
	return 2;
}

/*
	Instruction: BIT X,r8
	Usage:		 Test bit X in register R8, set the Zero flag if bit not set;
				 also unset the Negative flag and set the Half Carry flag.
	Cost:		 2 CPU cycles
*/
int Cpu::BIT_X_R8(int x, const u8 r8)
{
	SetFlag(FLAG_Z, !NTHBIT(r8, x));
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 1);
	return 2;
}

/*
	Instruction: JR NZ,i8 
	Usage:		 Relative jump if condition NZ is met. 
	Cost:		 3 taken/2 untaken CPU cycles
*/
int Cpu::JR_NZ_I8(u8 offset)
{
	regs.PC() += 1;
	if (!GetFlag(FLAG_Z))
		regs.PC() += (i8)offset;
	return (!GetFlag(FLAG_Z)) ? 3 : 2;
}

/*
	Instruction: INC r8
	Usage:		 Increment the value in register r8 by 1.
	Cost:		 1 CPU cycle
*/
int Cpu::INC(u8& r8)
{
	SetFlag(FLAG_H, ((r8 & 0x0f) + 1) > 0x0f);
	r8 += 1;
	SetFlag(FLAG_Z, r8 == 0);
	SetFlag(FLAG_N, 0);
	return 1;
}

/*
	Instruction: INC r16
	Usage:		 Increment the value in register r16 by 1.
	Cost:		 2 CPU cycle
*/
int Cpu::INC(u16& r16)
{
	r16 += 1;
	return 2;
}

/*
	Instruction: LD r8,u8
	Usage:		 Copy the value of u8 into r8 register.
	Cost:		 2 CPU cycles
*/
int Cpu::LD_R8_U8(u8& r8, u8 u8)
{
	r8 = u8;
	regs.PC() += 1;
	return 2;
}

/*
	Instruction: LD [C],A
	Usage:		 Copy the value in register A into the byte at address $FF00 + C.
	Cost:		 2 CPU cycles
*/
int Cpu::LD_IC_A()
{
	bus->Write(0xFF00 + regs.C(), regs.A());
	return 2;
}

/*
	Instruction: LD [HL],r8
	Usage:		 Copy the value in register r8 into the byte pointed to by HL.
	Cost:		 2 CPU cycles
*/
int Cpu::LD_IHL_R8(u8& r8)
{
	bus->Write(regs.HL(), r8);
	return 2;
}

/*
	Instruction: LD [u16],A
	Usage:		 Copy the value in register A into the byte pointed to by address $ff00 + u8.
	Cost:		 3 CPU cycles
*/
int Cpu::LD_IU8_A(u8 u8, Rom& rom)
{
	bus->Write(0xFF00 + u8, regs.A());
	regs.PC() += 1;
	if (u8 == 0x50) {
		spdlog::info("BootRom unlocked!");
		rom.UnlockBootROM();
	}
	return 3;
}

/*
	Instruction: LD r8,r8
	Usage:		 Copy the value in register on the right into the register on the left.
	Cost:		 1 CPU cycles
*/
int Cpu::LD_R8_R8(u8& r1, u8& r2)
{
	r1 = r2;
	return 1;
}

/*
	Instruction: LD A,[r16]
	Usage:		 Copy the byte pointed to by r16 register into regsiter A.
	Cost:		 2 CPU cycles
*/
int Cpu::LD_A_IR16(u16 r16)
{
	regs.A() = bus->Read(r16);
	return 2;
}

/*
 	Instruction: CALL u16
	Usage:		 Push the address after this instruction into the stack, then set
				 the PC to u16.
	Cost:		 6 CPU cycles
*/
int Cpu::CALL_U16(u16 newPC)
{
	regs.PC() += 2;
	StackPush(MSB(regs.PC()));
	StackPush(LSB(regs.PC()));
	regs.PC() = newPC;
	return 6;
}

/*
	Instruction: PUSH r16
	Usage:		 Push the value of register R16 to the stack.
	Cost:		 4 CPU cycles
*/
int Cpu::PUSH_R16(u16& r16)
{
	StackPush(MSB(r16));
	StackPush(LSB(r16));
	return 4;
}

/*
 	Instruction: RL r8
	Usage:		 Rotate bits in register r8 left, through the carry flag.
	Cost:		 2 CPU cycles
*/
int Cpu::RL_R8(u8& r8)
{
	u16 tmp = (r8 << 1) | GetFlag(FLAG_C);

	r8 = LSB(tmp);
	SetFlag(FLAG_Z, !r8);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, NTHBIT(tmp, 8));
	return 2;
}

/*
	Instruction: RLA
	Usage:		 Rotate register A left, through the carry flag.
	Cost:		 1 CPU cycle
*/
int Cpu::RLA()
{
	u16 tmp = (regs.A() << 1) | GetFlag(FLAG_C);

	regs.A() = LSB(tmp);
	SetFlag(FLAG_Z, 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, NTHBIT(tmp, 8));
	return 2;
}

/*	
	Instruction: POP r16
	Usage:		 Pop register r16 from the stack.
	Cost:		 3 CPU cycles
*/
int Cpu::POP_R16(u16& r16)
{
	u8 lsb, msb;

	lsb = StackPop();
	msb = StackPop();
	r16 = U16(lsb, msb);
	return 3;
}

/*	
	Instruction: DEC r8
	Usage:		 Decrement the value in register r8.
	Cost:		 1 CPU cycles
*/
int Cpu::DEC_R8(u8& r8)
{
	u8 carryBits = (r8 - 1) ^ r8 ^ 0xff;

	r8 -= 1;
	SetFlag(FLAG_Z, !r8);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
	return 1;
}

/*
	Instruction: LD (HL+),A
	Usage:		 Copy the value of register A into the byte pointed to by HL and increment HL.
	Cost:		 2 CPU cycles
*/
int Cpu::LD_HLI_A()
{
	bus->Write(regs.HL(), regs.A());
	regs.HL() += 1;
	return 2;
}

/*
	Instruction: RET
	Usage:		 Return from subroutine. Basically just pop the address and set it to PC.
	Cost:		 4 CPU cycles
*/
int Cpu::RET()
{
	POP_R16(regs.PC());
	return 4;
}

/*
	Instruction:	CP A,u8
	Usage:			Compare the value of register A with u8.
	Cost:			2 CPU cycles
*/
int Cpu::CP_U8(const u8 val)
{
	u8 res = regs.A() - val, carryBits = res ^ regs.A() ^ ~val;

	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, val > regs.A());
	regs.PC() += 1;
	return 2;
}

/*
	Instruction:	LD [u16],A
	Usage:			Compare the value of register A into the byte at address u16.
	Cost:			4 CPU cycles
*/
int Cpu::LD_IU16_A(const u16 addr)
{
	bus->Write(addr, regs.A());
	regs.PC() += 2;
	return 4;
}

/*
	Instruction:	JR Z,i8
	Usage:			Relative jump if Z flag is true.
	Cost:			2 CPU cycles
*/
int Cpu::JR_Z_I8(u8 offset)
{
	regs.PC() += 1;
	if (GetFlag(FLAG_Z))
		regs.PC() += (i8)offset;
	return 2;
}

/*
	Instruction:	JR i8
	Usage:			Relative jump.
	Cost:			2 CPU cycles
*/
int Cpu::JR_I8(u8 offset)
{
	regs.PC() += 1;
	regs.PC() += (i8)offset;
	return 2;
}

/*
	Instruction:	INC r8
	Usage:			Increment the value of r8 register.
	Cost:			1 CPU cycle
*/
int Cpu::INC_R8(u8& r8)
{
	u8 res = r8 + 1, carryBits = r8 ^ res ^ 0x01;

	r8 = r8 + 1;
	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, NTHBIT(carryBits, 4));
	return 1;
}

/*
	Instruction:	LD A,($FF00+u8)
	Usage:			Copy the byte at address $ff00 + u8 into register A.
	Cost:			3 CPU cycles
*/
int Cpu::LD_A_IU16(const u8 val)
{
	regs.PC() += 1;
	regs.A() = bus->Read(0xff00 + val);
	return 3;
}

/*
	Instruction:	SUB A,r8
	Usage:			Subtract the value in r8 from A.
	Cost:			1 CPU cycle
*/
int Cpu::SUB_A_R8(u8& r8)
{
	u8 res = regs.A() - r8, carryBits = res ^ regs.A() ^ ~r8;

	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, r8 > regs.A());
	regs.A() = res;
	return 1;
}

/*
	Instruction:	CP A,(HL)
	Usage:			Compare the value in A with the byte pointed to by HL.
	Cost:			2 CPU cycles
*/
int Cpu::CP_A_IHL()
{
	u8 val = bus->Read(regs.HL()), res = regs.A() - val, carryBits = res ^ regs.A() ^ ~val;

	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, val > regs.A());
	return 2;
}

/*
	Instruction:	ADD A,(HL)
	Usage:			Add the byte pointed to by HL to A.
	Cost:			2 CPU cycles
*/
int Cpu::ADD_A_IHL()
{
	u8 val = bus->Read(regs.HL()), res = regs.A() + val;
	u16 carryBits = res ^ regs.A() ^ val;

	regs.A() = regs.A() + val;
	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, NTHBIT(carryBits, 8));
	return 2;
}


/*
	Instruction:	NOP
	Usage:			No operation.
	Cost:			1 CPU cycle
*/
int Cpu::NOP()
{
	return 1;
}

/*
	Instruction:	JP u16
	Usage:			Set the PC register to u16
	Cost:			4 CPU cycles
*/
int Cpu::JP_U16(u16 addr)
{
	regs.PC() = addr;
	return 4;
}

/*
	Instruction:	LD (R16),A
	Usage:			Copy the value in register A into the byte pointed
					to by R16.
	Cost:			2 CPU cycles
*/
int Cpu::LD_IR16_A(u16& r16)
{
	bus->Write(r16, regs.A());
	return 2;
}

/*
	Instruction:	RLCA
	Usage:			Rotate regiser A left.
	Cost:			1 CPU cycle
*/
int Cpu::RLCA()
{
	SetFlag(FLAG_Z, 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, NTHBIT(regs.A(), 7));
	regs.A() = (regs.A() << 7) | NTHBIT(regs.A(), 7);
	return 1;
}

/*
	Instruction:	LD (u16),SP
	Usage:			Copy SP & $FF at address u16 and SP >> 8 at address u16 + 1.
	Cost:			5 CPU cycles
*/
int Cpu::LD_IU16_SP(u16 addr)
{
	bus->Write(addr, regs.SP() & 0xFF);
	bus->Write(addr + 1, regs.SP() >> 8);
	return 5;
}

/*
	Instruction:	ADD HL R16
	Usage:			Add the value in r16 to HL.
	Cost:			2 CPU cycles
*/
int Cpu::ADD_HL_R16(u16& r16)
{
	u32 res = regs.HL() + r16;
	u32 carryBits = res ^ regs.HL() ^ r16;

	regs.HL() = res;
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, NTHBIT(carryBits, 12));
	SetFlag(FLAG_C, NTHBIT(carryBits, 16));
	return 2;
}

int Cpu::Step(Rom& rom, std::shared_ptr<spdlog::logger> logger)
{
	int cycle = -1;
	u8 opcode, operandA, operandB;

	/* 
		using the boot rom's log file from https://github.com/wheremyfoodat/Gameboy-logs
		to see if something goes wrong 
	 */
	opcode = rom.Read(regs.PC());
	logger->info("A: {:02X} F: {:02X} B: {:02X} C: {:02X} D: {:02X} E: {:02X} H: {:02X} L: {:02X} SP: {:04X} PC: 00:{:04X} ({:02X} {:02X} {:02X} {:02X})", 
		regs.A(), regs.F(), regs.B(), regs.C(), regs.D(), regs.E(), regs.H(), regs.L(), regs.SP(), regs.PC(),
				rom.Read(regs.PC()), rom.Read(regs.PC() + 1), rom.Read(regs.PC() + 2), rom.Read(regs.PC() + 3));
	regs.PC() += 1;
	operandA = rom.Read(regs.PC());
	operandB = rom.Read(regs.PC() + 1);

	switch (opcode) {
	case 0x00:		/* NOP */
		cycle = NOP();
		break;
	case 0x01:		/* LD BC,u16 */
		cycle = LD_R16_U16(regs.BC(), U16(operandA, operandB));
		break;
	case 0x02:		/* LD (BC),A */
		cycle = LD_IR16_A(regs.BC());
		break;
	case 0x03:		/* INC BC */
		cycle = INC(regs.BC());
		break;
	case 0x04:		/* INC B */
		cycle = INC_R8(regs.B());
		break;
	case 0x05:		/* DEC B */
		cycle = DEC_R8(regs.B());
		break;
	case 0x06:		/* LD B,u8 */
		cycle = LD_R8_U8(regs.B(), operandA);
		break;
	case 0x07:		/* RLCA */
		cycle = RLCA();
		break;
	case 0x08:		/* LD (u16),SP */
		cycle = LD_IU16_SP(U16(operandA, operandB));
		break;
	case 0x09:		/* ADD HL, BC */
		cycle = ADD_HL_R16(regs.BC());
		break;
	case 0x0d:		/* DEC C */
		cycle = DEC_R8(regs.C());
		break;
	case 0x0c:		/* INC C */
		cycle = INC(regs.C());
		break;
	case 0x0e:		/* LD C,u8 */
		cycle = LD_R8_U8(regs.C(), operandA);
		break;
	case 0x11:		/* LD DE,u16 */
		cycle = LD_R16_U16(regs.DE(), U16(operandA, operandB));
		break;
	case 0x13:		/* INC DE */
		cycle = INC(regs.DE());
		break;
	case 0x15:		/* DEC D */
		cycle = DEC_R8(regs.D());
		break;
	case 0x16:		/* LD D,u8 */
		cycle = LD_R8_U8(regs.D(), operandA);
		break;
	case 0x17:		/* RLA */
		cycle = RLA();
		break;
	case 0x18:		/* JR i8 */
		cycle = JR_I8(operandA);
		break;
	case 0x1a:		/* LD A,(DE) */
		cycle = LD_A_IR16(regs.DE());
		break;
	case 0x1d:		/* DEC E */
		cycle = DEC_R8(regs.E());
		break;
	case 0x1e:		/* LD E,u8 */
		cycle = LD_R8_U8(regs.E(), operandA);
		break;
	case 0x3e:		/* LD A,u8 */
		cycle = LD_R8_U8(regs.A(), operandA);
		break;
	case 0x20:		/* JR NZ,i8 */
		cycle = JR_NZ_I8(operandA);
		break;
	case 0x21:		/* LD HL,u16 */
		cycle = LD_R16_U16(regs.HL(), U16(operandA, operandB));
		break;
	case 0x22:		/* LD (HL+),A */
		cycle = LD_HLI_A();
		break;
	case 0x23:		/* INC HL */
		cycle = INC(regs.HL());
		break;
	case 0x24:		/* INC H */
		cycle = INC_R8(regs.H());
		break;
	case 0x28:		/* JR Z,i8 */
		cycle = JR_Z_I8(operandA);
		break;
	case 0x2e:		/* LD L,u8 */
		cycle = LD_R8_U8(regs.L(), operandA);
		break;
	case 0x31:		/* LD SP,u16 */
		cycle = LD_R16_U16(regs.SP(), U16(operandA, operandB));
		break;
	case 0x32:		/* LD (HL-),A */
		cycle = LD_HLD_A();
		break;
	case 0x47:		/* LD B,A */
		cycle = LD_R8_R8(regs.B(), regs.A());
		break;
	case 0x3d:		/* DEC A */
		cycle = DEC_R8(regs.A());
		break;
	case 0x4f:		/* LD C,A */
		cycle = LD_R8_R8(regs.C(), regs.A());
		break;
	case 0x57:		/* LD D,A */
		cycle = LD_R8_R8(regs.D(), regs.A());
		break;
	case 0x67:		/* LD H,A */
		cycle = LD_R8_R8(regs.H(), regs.A());
		break;
	case 0x77:		/* LD (HL),A */
		cycle = LD_IHL_R8(regs.A());
		break;
	case 0x78:		/* LD A, B */
		cycle = LD_R8_R8(regs.A(), regs.B());
		break;
	case 0x7b:		/* LD A, E */
		cycle = LD_R8_R8(regs.A(), regs.E());
		break;
	case 0x7c:		/* LD A, H */
		cycle = LD_R8_R8(regs.A(), regs.H());
		break;
	case 0x7d:
		cycle = LD_R8_R8(regs.A(), regs.L());
		break;
	case 0x86:
		cycle = ADD_A_IHL();
		break;
	case 0x90:		/* SUB A,B */
		cycle = SUB_A_R8(regs.B());
		break;
	case 0xaf:		/* XOR A,A */
		cycle = XOR_A_R8(regs.A());
		break;
	case 0xbe:		/* CP A,(HL) */
		cycle = CP_A_IHL();
		break;
	case 0xc1:		/* POP BC */
		cycle = POP_R16(regs.BC());
		break;
	case 0xc3:		/* JP u16 */
		cycle = JP_U16(U16(operandA, operandB));
		break;
	case 0xc5:		/* PUSH BC */
		cycle = PUSH_R16(regs.BC());
		break;
	case 0xc9:		/* RET */
		cycle = RET();
		break;
	case 0xcb:
		switch (operandA) {
		case 0x7c:	/* BIT 7,H */
			cycle = BIT_X_R8(7, regs.H());
			break;
		case 0x11:	/* RL C */
			cycle = RL_R8(regs.C());
			break;
		default:
			spdlog::error("Opcode invalid - ${:02X} ${:02X}", opcode, operandA);
			return cycle;
		};
		regs.PC() += 1;
		break;
	case 0xcd:		/* CALL u16 */
		cycle = CALL_U16(U16(operandA, operandB));
		break;
	case 0xe0:		/* LD [u16],A */
		cycle = LD_IU8_A(operandA, rom);
		break;
	case 0xe2:		/* LD [$FF00+C],A */
		cycle = LD_IC_A();
		break;
	case 0xea:		/* LD [u16],A */
		cycle = LD_IU16_A(U16(operandA, operandB));
		break;
	case 0xf0:		/* LD A,[$FF00+u8] */
		cycle = LD_A_IU16(operandA);
		break;
	case 0xfe:		/* CP A,u8 */
		cycle = CP_U8(operandA);
		break;
	default:
		spdlog::error("Opcode invalid - ${:02X}", opcode);
		break;
	};
	return cycle;
}

Cpu::Cpu(Bus *pBus) : bus(pBus)
{
	// DMG's registers start up value. Src:
	// https://gbdev.io/pandocs/Power_Up_Sequence.html#power-up-sequence
	//regs.af.a = 0x01;
	//regs.af.f.z = 1;
	//regs.af.f.n = 0;
	//regs.bc.b = 0x00;
	//regs.bc.c = 0x13;
	//regs.de.d = 0x00;
	//regs.de.e = 0xd8;
	//regs.hl.h = 0x01;
	//regs.hl.l = 0x4d;
	//regs.pc = 0x0100;
	//regs.sp = 0xfffe;
	regs.PC() = 0x0000;
}

Cpu::~Cpu()
{

}