#pragma once

#include "common.h"
#include "rom.h"
#include "bus.h"
#include <fstream>

#define FMT_HEADER_ONLY
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

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
	u16& Val() 
	{ 
		return val; 
	}
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
	u16& Val() { return val; }
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
	u16& AF() { return af.Val(); }
	u16& BC() { return bc.Val(); }
	u16& DE() { return de.Val(); }
	u16& HL() { return hl.Val(); }
	u16& SP() { return sp.Val(); }
	u16& PC() { return pc.Val(); }
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
	CpuRegs regs;
	Bus* bus = nullptr;

	void StackPush(u8);
	u8 StackPop();
	void SetZNHC(bool, bool, bool, bool);
	int NOP();
	int JP_U16(u16);
	int RLCA();
	int LD_IU16_SP(u16);
	int ADD_HL_R16(u16&);
	int LD_IR16_A(u16&);
	int LD_R16_U16(u16&, u16);
	int XOR_A_R8(u8&);
	int LD_HLD_A();
	int BIT_X_R8(int, const u8);
	int JR_NZ_I8(u8);
	int INC(u8&);
	int INC(u16&);
	int LD_R8_U8(u8&, u8);
	int LD_IC_A();
	int LD_IHL_R8(u8&);
	int LD_IU8_A(u8, Rom&);
	int LD_R8_R8(u8&, u8&);
	int LD_A_IR16(u16);
	int CALL_U16(u16);
	int PUSH_R16(u16&);
	int RL_R8(u8&);
	int RLA();
	int POP_R16(u16&);
	int DEC_R8(u8&);
	int LD_HLI_A();
	int RET();
	int CP_U8(const u8);
	int LD_IU16_A(const u16);
	int JR_Z_I8(u8);
	int JR_I8(u8);
	int INC_R8(u8&);
	int LD_A_IU16(const u8);
	int SUB_A_R8(u8&);
	int CP_A_IHL();
	int ADD_A_IHL();
protected:
public:
	void SetFlag(CpuFlag flag, bool val);
	bool GetFlag(CpuFlag flag);
	int Step(Rom&, std::shared_ptr<spdlog::logger>);
	Cpu(Bus *);
	~Cpu();
};
