#pragma once

#include "common.h"
#include "rom.h"
#include "bus.h"
#include <fstream>

typedef struct CpuState {
	u16 PC;
	u16 SP;
	u16 BC;
	u16 DE;
	u16 HL;
	union {
		u16 val;
		struct {
			u8 A;
			union {
				u8 F;
				struct {
					u8 unused0 : 1;
					u8 unused1 : 1;
					u8 unused2 : 1;
					u8 unused3 : 1;
					u8 c : 1;
					u8 h : 1;
					u8 n : 1;
					u8 z : 1;
				};
			};
		};
	} AF;
	std::array<u8, 4> romData;		// store 4 next ROM bytes, start from the address PC.
} CpuState;

typedef enum {
	FLAG_Z = (1U << 7),
	FLAG_N = (1U << 6),
	FLAG_H = (1U << 5),
	FLAG_C = (1U << 4),
} CpuFlag;

typedef struct RegisterPair {
private:
	union {
		u16 val;
		struct {
			u8 lowByte;
			u8 highByte;
		};
	};
public:
	RegisterPair& operator=(const u16 other)
	{
		val = other;
		return *this;
	}
	RegisterPair& operator=(const RegisterPair& other)
	{
		this->val = other.val;
		return *this;
	}
	RegisterPair& operator+=(const u8 num)
	{
		val += num;
		return *this;
	}
	RegisterPair& operator^=(const u8 num)
	{
		val ^= num;
		return *this;
	}
	RegisterPair& operator-=(const u8 num)
	{
		val -= num;
		return *this;
	}
	RegisterPair& operator++(int)
	{
		val++;
		return *this;
	}
	RegisterPair& operator--(int)
	{
		val--;
		return *this;
	}
	u8& LB() { return lowByte; }
	u8& HB() { return highByte; }
	u16& Get() { return val; }
} RegPair;

typedef struct AFRegisterPair {
private:
	union {
		u16 val;
		struct {
			u8 a;
			union {
				u8 f;
				struct {
					u8 unused0 : 1;
					u8 unused1 : 1;
					u8 unused2 : 1;
					u8 unused3 : 1;
					u8 c : 1;
					u8 h : 1;
					u8 n : 1;
					u8 z : 1;
				};
			};
		};
	};
public:
	u8& LB() { return f; }
	u8& HB() { return a; }
	u16& Get() { return val; }
} AFRegPair;

typedef struct CpuRegisters {
private:
	AFRegPair af;
	RegPair bc;
	RegPair de;
	RegPair hl;
	RegPair sp;
	RegPair pc;
public:
	u16& AF() { return af.Get(); }
	u16& BC() { return bc.Get(); }
	u16& DE() { return de.Get(); }
	u16& HL() { return hl.Get(); }
	u16& SP() { return sp.Get(); }
	u16& PC() { return pc.Get(); }
	u8& A() { return af.HB(); }
	u8& F() { return af.LB(); }
	u8& B() { return bc.HB(); }
	u8& C() { return bc.LB(); }
	u8& D() { return de.HB(); }
	u8& E() { return de.LB(); }
	u8& H() { return hl.HB(); }
	u8& L() { return hl.LB(); }
} CpuRegs;

class Cpu {
private:
	CpuState state;
	int mCycles;
	CpuRegs regs;
	Bus* bus = nullptr;

	void StackPush(u8);
	u8 StackPop();
	void SetZNHC(bool, bool, bool, bool);
	void NOP();
	void JP_U16(u16);
	void RLCA();
	void LD_IU16_SP(u16);
	void ADD_HL_R16(u16&);
	void LD_IR16_A(u16&);

	void XOR_A_R8(u8&);
	void LD_HLD_A();
	void BIT_X_R8(int, const u8);
	void JR_NZ_I8(u8);
	void INC(u8&);
	void INC(u16&);
	void LD(u8&, u8);
	void LD(u16&, u16);
	void LD_IC_A();
	void LD_IHL_R8(u8&);
	void LD_IU8_A(u8, Rom&);
	void LD_A_IR16(u16);
	void CALL_U16(u16);
	void PUSH_R16(u16&);
	void RL_R8(u8&);
	void RLA();
	void POP_R16(u16&);
	void DEC(u8&);
	void LD_HLI_A();
	void RET();
	void CP_U8(const u8);
	void LD_IU16_A(const u16);
	void JR_Z_I8(u8);
	void JR_I8(u8);
	void LD_A_IU16(const u8);
	void SUB_A_R8(u8&);
	void CP_A_IHL();
	void ADD_A_IHL();
protected:
public:
	CpuState GetCpuState() const;
	void SetFlag(CpuFlag flag, bool val);
	bool GetFlag(CpuFlag flag);
	int Step(Rom&);
	Cpu(Bus *);
	~Cpu();
};
