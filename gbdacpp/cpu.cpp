#include "cpu.h"
#include <array>
#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#define OPCODE_TBL_SIZE			256
#define OPCODE_UNKNOWN			-1

std::array<u8, 256> mainOpcodeMCycles = {
    // 0x0_
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1,
    // 0x1_
    1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
    // 0x2_
    3, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1,
    // 0x3_
    3, 3, 2, 2, 3, 3, 3, 1, 3, 2, 2, 2, 1, 1, 2, 1,
    // 0x4_
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0x5_
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0x6_
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0x7_
    2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0x8_
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0x9_
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0xA_
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0xB_
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1,
    // 0xC_
    5, 3, 4, 4, 6, 4, 2, 4, 5, 4, 4, 1, 6, 6, 2, 4,
    // 0xD_
    5, 3, 4, 1, 6, 4, 2, 4, 5, 4, 4, 1, 6, 1, 2, 4,
    // 0xE_
    3, 3, 2, 1, 1, 4, 2, 4, 4, 1, 4, 1, 1, 1, 2, 4,
    // 0xF_
    3, 3, 2, 1, 1, 4, 2, 4, 3, 2, 4, 1, 1, 1, 2, 4
};

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
void Cpu::LD(u16& dst, u16 val)
{
	dst = val;
	regs.PC() += 2;
}

/* 
	Instruction: XOR A,r8
	Usage:		 XOR r8 value with A register, then store the result in A
	Cost:		 1 CPU cycle
*/
void Cpu::XOR_A_R8(u8& r8)
{
	regs.A() ^= r8;
	SetZNHC(regs.A() == 0, 0, 0, 0);
}

/*
	Instruction: LD (HL-),A 
	Usage:		 Load the value of A into memory pointed by HL, then decrement HL. 
	Cost:		 2 CPU cycles
*/
void Cpu::LD_HLD_A()
{
	if (regs.HL() == 0xff04)
		spdlog::info("Write to 0xff04, data: {:02X}", regs.A());
	bus->Write(regs.HL(), regs.A());
	regs.HL() -= 1;
}

/*
	Instruction: BIT X,r8
	Usage:		 Test bit X in register R8, set the Zero flag if bit not set;
				 also unset the Negative flag and set the Half Carry flag.
	Cost:		 2 CPU cycles
*/
void Cpu::BIT_X_R8(int x, const u8 r8)
{
	SetFlag(FLAG_Z, !NTHBIT(r8, x));
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 1);
}

/*
	Instruction: JR NZ,i8 
	Usage:		 Relative jump if condition NZ is met. 
	Cost:		 3 taken/2 untaken CPU cycles
*/
void Cpu::JR_NZ_I8(u8 offset)
{
	regs.PC() += 1;
	if (!GetFlag(FLAG_Z))
		regs.PC() += (i8)offset;
	mCycles += (!GetFlag(FLAG_Z)) ? 1 : 0;
}

/*
	Instruction: INC r8
	Usage:		 Increment the value in register r8 by 1.
	Cost:		 1 CPU cycle
*/
void Cpu::INC(u8& r8)
{
	SetFlag(FLAG_H, ((r8 & 0x0f) + 1) > 0x0f);
	r8 += 1;
	SetFlag(FLAG_Z, r8 == 0);
	SetFlag(FLAG_N, 0);
}

/*
	Instruction: INC r16
	Usage:		 Increment the value in register r16 by 1.
	Cost:		 2 CPU cycle
*/
void Cpu::INC(u16& r16)
{
	r16 += 1;
}

/*
	Instruction: LD r8,u8
	Usage:		 Copy the value of u8 into r8 register.
	Cost:		 2 CPU cycles
*/
void Cpu::LD(u8& r8, u8 u8)
{
	r8 = u8;
	regs.PC() += 1;
}

/*
	Instruction: LD [C],A
	Usage:		 Copy the value in register A into the byte at address $FF00 + C.
	Cost:		 2 CPU cycles
*/
void Cpu::LD_IC_A()
{
	bus->Write(0xFF00 + regs.C(), regs.A());
}

/*
	Instruction: LD [HL],r8
	Usage:		 Copy the value in register r8 into the byte pointed to by HL.
	Cost:		 2 CPU cycles
*/
void Cpu::LD_IHL_R8(u8& r8)
{
	bus->Write(regs.HL(), r8);
}

/*
	Instruction: LD [u16],A
	Usage:		 Copy the value in register A into the byte pointed to by address $ff00 + u8.
	Cost:		 3 CPU cycles
*/
void Cpu::LD_IU8_A(u8 u8, Rom& rom)
{
	bus->Write(0xFF00 + u8, regs.A());
	regs.PC() += 1;
	if (u8 == 0x50) {
		spdlog::info("BootRom unlocked!");
		rom.UnlockBootROM();
	}
}

/*
	Instruction: LD A,[r16]
	Usage:		 Copy the byte pointed to by r16 register into regsiter A.
	Cost:		 2 CPU cycles
*/
void Cpu::LD_A_IR16(u16 r16)
{
	regs.A() = bus->Read(r16);
}

/*
 	Instruction: CALL u16
	Usage:		 Push the address after this instruction into the stack, then set
				 the PC to u16.
	Cost:		 6 CPU cycles
*/
void Cpu::CALL_U16(u16 newPC)
{
	regs.PC() += 2;
	StackPush(MSB(regs.PC()));
	StackPush(LSB(regs.PC()));
	regs.PC() = newPC;
}

/*
	Instruction: PUSH r16
	Usage:		 Push the value of register R16 to the stack.
	Cost:		 4 CPU cycles
*/
void Cpu::PUSH_R16(u16& r16)
{
	StackPush(MSB(r16));
	StackPush(LSB(r16));
}

/*
 	Instruction: RL r8
	Usage:		 Rotate bits in register r8 left, through the carry flag.
	Cost:		 2 CPU cycles
*/
void Cpu::RL_R8(u8& r8)
{
	u16 tmp = (r8 << 1) | GetFlag(FLAG_C);

	r8 = LSB(tmp);
	SetFlag(FLAG_Z, !r8);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, NTHBIT(tmp, 8));
}

/*
	Instruction: RLA
	Usage:		 Rotate register A left, through the carry flag.
	Cost:		 1 CPU cycle
*/
void Cpu::RLA()
{
	u16 tmp = (regs.A() << 1) | GetFlag(FLAG_C);

	regs.A() = LSB(tmp);
	SetFlag(FLAG_Z, 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, NTHBIT(tmp, 8));
}

/*	
	Instruction: POP r16
	Usage:		 Pop register r16 from the stack.
	Cost:		 3 CPU cycles
*/
void Cpu::POP_R16(u16& r16)
{
	u8 lsb, msb;

	lsb = StackPop();
	msb = StackPop();
	r16 = U16(lsb, msb);
}

/*	
	Instruction: DEC r8
	Usage:		 Decrement the value in register r8.
	Cost:		 1 CPU cycles
*/
void Cpu::DEC(u8& r8)
{
	u8 carryBits = (r8 - 1) ^ r8 ^ 0xff;

	r8 -= 1;
	SetFlag(FLAG_Z, !r8);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
}

/*
	Instruction: LD (HL+),A
	Usage:		 Copy the value of register A into the byte pointed to by HL and increment HL.
	Cost:		 2 CPU cycles
*/
void Cpu::LD_HLI_A()
{
	bus->Write(regs.HL(), regs.A());
	regs.HL() += 1;
}

/*
	Instruction: RET
	Usage:		 Return from subroutine. Basically just pop the address and set it to PC.
	Cost:		 4 CPU cycles
*/
void Cpu::RET()
{
	POP_R16(regs.PC());
}

/*
	Instruction:	CP A,u8
	Usage:			Compare the value of register A with u8.
	Cost:			2 CPU cycles
*/
void Cpu::CP_U8(const u8 val)
{
	u8 res = regs.A() - val, carryBits = res ^ regs.A() ^ ~val;

	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, val > regs.A());
	regs.PC() += 1;
}

/*
	Instruction:	LD [u16],A
	Usage:			Compare the value of register A into the byte at address u16.
	Cost:			4 CPU cycles
*/
void Cpu::LD_IU16_A(const u16 addr)
{
	bus->Write(addr, regs.A());
	regs.PC() += 2;
}

/*
	Instruction:	JR Z,i8
	Usage:			Relative jump if Z flag is true.
	Cost:			2 CPU cycles
*/
void Cpu::JR_Z_I8(u8 offset)
{
	regs.PC() += 1;
	if (GetFlag(FLAG_Z))
		regs.PC() += (i8)offset;
}

/*
	Instruction:	JR i8
	Usage:			Relative jump.
	Cost:			2 CPU cycles
*/
void Cpu::JR_I8(u8 offset)
{
	regs.PC() += 1;
	regs.PC() += static_cast<i8>(offset);
}

/*
	Instruction:	LD A,($FF00+u8)
	Usage:			Copy the byte at address $ff00 + u8 into register A.
	Cost:			3 CPU cycles
*/
void Cpu::LD_A_IU16(const u8 val)
{
	regs.PC() += 1;
	regs.A() = bus->Read(0xff00 + val);
}

/*
	Instruction:	SUB A,r8
	Usage:			Subtract the value in r8 from A.
	Cost:			1 CPU cycle
*/
void Cpu::SUB_A_R8(u8& r8)
{
	u8 res = regs.A() - r8, carryBits = res ^ regs.A() ^ ~r8;

	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, r8 > regs.A());
	regs.A() = res;
}

/*
	Instruction:	CP A,(HL)
	Usage:			Compare the value in A with the byte pointed to by HL.
	Cost:			2 CPU cycles
*/
void Cpu::CP_A_IHL()
{
	u8 val = bus->Read(regs.HL()), res = regs.A() - val, carryBits = res ^ regs.A() ^ ~val;

	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 1);
	SetFlag(FLAG_H, !NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, val > regs.A());
}

/*
	Instruction:	ADD A,(HL)
	Usage:			Add the byte pointed to by HL to A.
	Cost:			2 CPU cycles
*/
void Cpu::ADD_A_IHL()
{
	u8 val = bus->Read(regs.HL()), res = regs.A() + val;
	u16 carryBits = res ^ regs.A() ^ val;

	regs.A() = regs.A() + val;
	SetFlag(FLAG_Z, !res);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, NTHBIT(carryBits, 4));
	SetFlag(FLAG_C, NTHBIT(carryBits, 8));
}


/*
	Instruction:	NOP
	Usage:			No operation.
	Cost:			1 CPU cycle
*/
void Cpu::NOP()
{

}

/*
	Instruction:	JP u16
	Usage:			Set the PC register to u16
	Cost:			4 CPU cycles
*/
void Cpu::JP_U16(u16 addr)
{
	regs.PC() = addr;
}

/*
	Instruction:	LD (R16),A
	Usage:			Copy the value in register A into the byte pointed
					to by R16.
	Cost:			2 CPU cycles
*/
void Cpu::LD_IR16_A(u16& r16)
{
	bus->Write(r16, regs.A());
}

/*
	Instruction:	RLCA
	Usage:			Rotate regiser A left.
	Cost:			1 CPU cycle
*/
void Cpu::RLCA()
{
	SetFlag(FLAG_Z, 0);
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, 0);
	SetFlag(FLAG_C, NTHBIT(regs.A(), 7));
	regs.A() = (regs.A() << 7) | NTHBIT(regs.A(), 7);
}

/*
	Instruction:	LD (u16),SP
	Usage:			Copy SP & $FF at address u16 and SP >> 8 at address u16 + 1.
	Cost:			5 CPU cycles
*/
void Cpu::LD_IU16_SP(u16 addr)
{
	bus->Write(addr, regs.SP() & 0xFF);
	bus->Write(addr + 1, regs.SP() >> 8);
}

/*
	Instruction:	ADD HL R16
	Usage:			Add the value in r16 to HL.
	Cost:			2 CPU cycles
*/
void Cpu::ADD_HL_R16(u16& r16)
{
	u32 res = regs.HL() + r16;
	u32 carryBits = res ^ regs.HL() ^ r16;

	regs.HL() = res;
	SetFlag(FLAG_N, 0);
	SetFlag(FLAG_H, NTHBIT(carryBits, 12));
	SetFlag(FLAG_C, NTHBIT(carryBits, 16));
}

CpuState Cpu::GetCpuState() const
{
	return state;
}

int Cpu::Step(Rom& rom)
{
	u8 opcode, operandA, operandB;

#ifdef LOGGER_ENABLE
	state.AF.val = regs.AF();
	state.BC = regs.BC();
	state.DE = regs.DE();
	state.HL = regs.HL();
	state.PC = regs.PC();
	state.SP = regs.SP();
	state.romData[0] = rom.Read(regs.PC());
	state.romData[1] = rom.Read(regs.PC() + 1);
	state.romData[2] = rom.Read(regs.PC() + 2);
	state.romData[3] = rom.Read(regs.PC() + 3);
#endif

	opcode = rom.Read(regs.PC());
	regs.PC() += 1;
	operandA = rom.Read(regs.PC());
	operandB = rom.Read(regs.PC() + 1);

	mCycles = 0;
	switch (opcode) {
	case 0x00:		/* NOP */
		NOP();
		break;
	case 0x01:		/* LD BC,u16 */
		LD(regs.BC(), U16(operandA, operandB));
		break;
	case 0x02:		/* LD (BC),A */
		LD_IR16_A(regs.BC());
		break;
	case 0x03:		/* INC BC */
		INC(regs.BC());
		break;
	case 0x04:		/* INC B */
		INC(regs.B());
		break;
	case 0x05:		/* DEC B */
		DEC(regs.B());
		break;
	case 0x06:		/* LD B,u8 */
		LD(regs.B(), operandA);
		break;
	case 0x07:		/* RLCA */
		RLCA();
		break;
	case 0x08:		/* LD (u16),SP */
		LD_IU16_SP(U16(operandA, operandB));
		break;
	case 0x09:		/* ADD HL, BC */
		ADD_HL_R16(regs.BC());
		break;
	case 0x0d:		/* DEC C */
		DEC(regs.C());
		break;
	case 0x0c:		/* INC C */
		INC(regs.C());
		break;
	case 0x0e:		/* LD C,u8 */
		LD(regs.C(), operandA);
		break;
	case 0x11:		/* LD DE,u16 */
		LD(regs.DE(), U16(operandA, operandB));
		break;
	case 0x13:		/* INC DE */
		INC(regs.DE());
		break;
	case 0x15:		/* DEC D */
		DEC(regs.D());
		break;
	case 0x16:		/* LD D,u8 */
		LD(regs.D(), operandA);
		break;
	case 0x17:		/* RLA */
		RLA();
		break;
	case 0x18:		/* JR i8 */
		JR_I8(operandA);
		break;
	case 0x1a:		/* LD A,(DE) */
		LD_A_IR16(regs.DE());
		break;
	case 0x1d:		/* DEC E */
		DEC(regs.E());
		break;
	case 0x1e:		/* LD E,u8 */
		LD(regs.E(), operandA);
		break;
	case 0x3e:		/* LD A,u8 */
		LD(regs.A(), operandA);
		break;
	case 0x20:		/* JR NZ,i8 */
		JR_NZ_I8(operandA);
		break;
	case 0x21:		/* LD HL,u16 */
		LD(regs.HL(), U16(operandA, operandB));
		break;
	case 0x22:		/* LD (HL+),A */
		LD_HLI_A();
		break;
	case 0x23:		/* INC HL */
		INC(regs.HL());
		break;
	case 0x24:		/* INC H */
		INC(regs.H());
		break;
	case 0x28:		/* JR Z,i8 */
		JR_Z_I8(operandA);
		break;
	case 0x2e:		/* LD L,u8 */
		LD(regs.L(), operandA);
		break;
	case 0x31:		/* LD SP,u16 */
		LD(regs.SP(), U16(operandA, operandB));
		break;
	case 0x32:		/* LD (HL-),A */
		LD_HLD_A();
		break;
	case 0x47:		/* LD B,A */
		LD(regs.B(), regs.A());
		break;
	case 0x3d:		/* DEC A */
		DEC(regs.A());
		break;
	case 0x4f:		/* LD C,A */
		LD(regs.C(), regs.A());
		break;
	case 0x57:		/* LD D,A */
		LD(regs.D(), regs.A());
		break;
	case 0x67:		/* LD H,A */
		LD(regs.H(), regs.A());
		break;
	case 0x77:		/* LD (HL),A */
		LD_IHL_R8(regs.A());
		break;
	case 0x78:		/* LD A, B */
		LD(regs.A(), regs.B());
		break;
	case 0x7b:		/* LD A, E */
		LD(regs.A(), regs.E());
		break;
	case 0x7c:		/* LD A, H */
		LD(regs.A(), regs.H());
		break;
	case 0x7d:
		LD(regs.A(), regs.L());
		break;
	case 0x86:
		ADD_A_IHL();
		break;
	case 0x90:		/* SUB A,B */
		SUB_A_R8(regs.B());
		break;
	case 0xaf:		/* XOR A,A */
		XOR_A_R8(regs.A());
		break;
	case 0xbe:		/* CP A,(HL) */
		CP_A_IHL();
		break;
	case 0xc1:		/* POP BC */
		POP_R16(regs.BC());
		break;
	case 0xc3:		/* JP u16 */
		JP_U16(U16(operandA, operandB));
		break;
	case 0xc5:		/* PUSH BC */
		PUSH_R16(regs.BC());
		break;
	case 0xc9:		/* RET */
		RET();
		break;
	case 0xcb:
		switch (operandA) {
		case 0x7c:	/* BIT 7,H */
			BIT_X_R8(7, regs.H());
			break;
		case 0x11:	/* RL C */
			RL_R8(regs.C());
			break;
		default:
			spdlog::error("Opcode invalid - ${:02X} ${:02X}", opcode, operandA);
			return OPCODE_UNKNOWN;
		};
		regs.PC() += 1;
		break;
	case 0xcd:		/* CALL u16 */
		CALL_U16(U16(operandA, operandB));
		break;
	case 0xe0:		/* LD [u16],A */
		LD_IU8_A(operandA, rom);
		break;
	case 0xe2:		/* LD [$FF00+C],A */
		LD_IC_A();
		break;
	case 0xea:		/* LD [u16],A */
		LD_IU16_A(U16(operandA, operandB));
		break;
	case 0xf0:		/* LD A,[$FF00+u8] */
		LD_A_IU16(operandA);
		break;
	case 0xfe:		/* CP A,u8 */
		CP_U8(operandA);
		break;
	default:
		spdlog::error("Opcode invalid - ${:02X}", opcode);
		return OPCODE_UNKNOWN;
	};
	return mCycles;
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