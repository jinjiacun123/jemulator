/*
    Copyright 2022-2024 Hydr8gon

    This file is part of rokuyon.

    rokuyon is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    rokuyon is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with rokuyon. If not, see <https://www.gnu.org/licenses/>.
*/

#include <cstring>
#include <vector>
#include <algorithm>

#include "common.h"
#include "cpu.h"
#include "core.h"
#include "cpu_cp0.h"
#include "cpu_cp1.h"
#include "log.h"
#include "memory.h"

// _mul128 / _umul128
#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_mul128)
#pragma intrinsic(_umul128)
#endif

namespace CPU
{
    uint64_t registersR[33];
    uint64_t *registersW[32];
    uint64_t hi, lo;
    uint32_t programCounter;
    uint32_t nextOpcode;
    uint32_t delaySlot;

    extern void (*immInstrs[])(uint32_t);
    extern void (*regInstrs[])(uint32_t);
    extern void (*extInstrs[])(uint32_t);
#if 1
	void (*fun_immInstrs)(uint32_t);
#if 0
	#define INST_STR(name) call_immInstrs_instructions(name){}
#else
	#define INST_STR(name) cpu_i_##name
//	#define INST_STR(name) name
#endif
	//void call_instructions(fun_immInstrs name);
#endif

  
    void INST_STR(jal)(uint32_t opcode);
    void INST_STR(beq)(uint32_t opcode);
    void INST_STR(bne)(uint32_t opcode);
    void INST_STR(addi)(uint32_t opcode);
    void INST_STR(addiu)(uint32_t opcode);
    void INST_STR(andi)(uint32_t opcode);
    void INST_STR(ori)(uint32_t opcode);
    void INST_STR(lui)(uint32_t opcode);
    void INST_STR(bnel)(uint32_t opcode);
    void INST_STR(lh)(uint32_t opcode);
    void INST_STR(lw)(uint32_t opcode);   
    void INST_STR(sh)(uint32_t opcode);
    void INST_STR(sw)(uint32_t opcode);   
    void INST_STR(cache)(uint32_t opcode);   
    void INST_STR(sll)(uint32_t opcode);   
    void INST_STR(sllv)(uint32_t opcode);
    void INST_STR(srlv)(uint32_t opcode);   
    void INST_STR(jr)(uint32_t opcode);   
    void INST_STR(mflo)(uint32_t opcode);    
    void INST_STR(multu)(uint32_t opcode);    
    void INST_STR(add)(uint32_t opcode);
    void INST_STR(addu)(uint32_t opcode);
    void INST_STR(subu)(uint32_t opcode);
    void INST_STR(and_)(uint32_t opcode);
    void INST_STR(or_)(uint32_t opcode);
    void INST_STR(xor_)(uint32_t opcode);
    void INST_STR(sltu)(uint32_t opcode);
    
	void INST_STR(bgezal)(uint32_t opcode);
    void mfc0(uint32_t opcode);
    void mtc0(uint32_t opcode);
    void INST_STR(cop0)(uint32_t opcode);
    void unk(uint32_t opcode);
}

// Immediate-type CPU instruction lookup table, using opcode bits 26-31
void (*CPU::immInstrs[0x40])(uint32_t) =
{
   nullptr,	 	     nullptr,             nullptr,    INST_STR(jal),      INST_STR(beq),      INST_STR(bne),      nullptr,  nullptr,  		 // 0x00-0x07
   INST_STR(addi),   INST_STR(addiu),     nullptr,    nullptr,            INST_STR(andi),     INST_STR(ori),      nullptr,  INST_STR(lui),   // 0x08-0x0F
   INST_STR(cop0),   nullptr,             nullptr,    nullptr,            nullptr,            INST_STR(bnel),     nullptr,  nullptr,  		 // 0x10-0x17
   nullptr, 		 nullptr,             nullptr,    nullptr,            nullptr,            nullptr,            nullptr,  nullptr,  		 // 0x18-0x1F
   nullptr, 		 INST_STR(lh),        nullptr,    INST_STR(lw),       nullptr,            nullptr,            nullptr,  nullptr,  		 // 0x20-0x27
   nullptr, 		 INST_STR(sh),        nullptr,    INST_STR(sw),       nullptr,            nullptr,            nullptr,  INST_STR(cache), // 0x28-0x2F
   nullptr,          nullptr,             nullptr,    nullptr,            nullptr,            nullptr,            nullptr,  nullptr, 		 // 0x30-0x37
   nullptr, 		 nullptr,             nullptr,    nullptr,            nullptr,            nullptr,            nullptr,  nullptr,  		 // 0x38-0x3F
};


// Register-type CPU instruction lookup table, using opcode bits 0-5
void (*CPU::regInstrs[0x40])(uint32_t) =
{
   INST_STR(sll), nullptr,             nullptr,    		nullptr,   		INST_STR(sllv),     nullptr,  		INST_STR(srlv),     nullptr,  // 0x00-0x07
   INST_STR(jr),  nullptr,             nullptr,    		nullptr,   		nullptr,  			nullptr,  		nullptr,  			nullptr,  // 0x08-0x0F
   nullptr,       nullptr,             INST_STR(mflo),  nullptr,   		nullptr,  			nullptr,  		nullptr,  			nullptr,  // 0x10-0x17
   nullptr,       INST_STR(multu),     nullptr,    		nullptr,   		nullptr,  			nullptr,  		nullptr,  			nullptr,  // 0x18-0x1F
   INST_STR(add), INST_STR(addu),      nullptr,       	INST_STR(subu), INST_STR(and_),     INST_STR(or_),  INST_STR(xor_),     nullptr,  // 0x20-0x27
   nullptr,       nullptr,             nullptr,         INST_STR(sltu), nullptr,  			nullptr, 		nullptr, 		    nullptr,  // 0x28-0x2F
   nullptr,       nullptr, 			   nullptr,         nullptr,        nullptr,            nullptr,        nullptr,            nullptr,  // 0x30-0x37
   nullptr,       nullptr, 			   nullptr,         nullptr,        nullptr,            nullptr,        nullptr,            nullptr,  // 0x38-0x3F
};

// Extra-type CPU instruction lookup table, using opcode bits 16-20
void (*CPU::extInstrs[0x20])(uint32_t) =
{ 
   nullptr, nullptr, 			nullptr,    nullptr,   nullptr,  nullptr,  nullptr,  nullptr,  // 0x00-0x07
   nullptr,	 nullptr, 			nullptr,    nullptr,   nullptr,  nullptr,  nullptr,  nullptr,  // 0x08-0x0F
   nullptr, INST_STR(bgezal), 	nullptr,    nullptr,   nullptr,  nullptr,  nullptr,  nullptr,  // 0x10-0x17
   nullptr, nullptr, 			nullptr,    nullptr,   nullptr,  nullptr,  nullptr,  nullptr,  // 0x18-0x1F
};

void CPU::reset()//[by jim] need
{
    // Map the writable registers so that writes to r0 are redirected
    registersW[0] = &registersR[32];
    for (int i = 1; i < 32; i++)
        registersW[i] = &registersR[i];

    // Reset the CPU to its initial state
    memset(registersR, 0, sizeof(registersR));
    hi = lo = 0;
    programCounter = 0xBFC00000 - 4;
    nextOpcode = 0;
    delaySlot = -1;
}

void CPU::runOpcode()//[by jim] need
{
    // Move an opcode through the pipeline
    // TODO: unaligned address exception
    uint32_t opcode = nextOpcode;
	static uint32_t line = 0;
//	printf("%d\t0x%02x:0x%02x\n", line++, programCounter, opcode);
//	if(line > 100)exit(-1);
    nextOpcode = Memory::read<uint32_t>(programCounter += 4);//[by jim] init program counter is 0xBFC00000
    bool clear = (delaySlot != -1);

    // Look up and execute an instruction
    switch (opcode >> 26)
    {
        default: (*immInstrs[opcode >> 26])(opcode);
			break;
        case 0: (*regInstrs[opcode & 0x3F])(opcode);         
			break;
        case 1:(*extInstrs[(opcode >> 16) & 0x1F])(opcode);
			break;
    }

    // Clear the delay slot address after it executes
    if (clear)
        delaySlot = -1;
}

void CPU::INST_STR(jal)(uint32_t opcode)
{
    // Save the return address and jump to an immediate value
    delaySlot = programCounter;
    *registersW[31] = programCounter + 4;
    programCounter = ((programCounter & 0xF0000000) | ((opcode & 0x3FFFFFF) << 2)) - 4;
}

void CPU::INST_STR(beq)(uint32_t opcode)
{
    // If an idle loop is detected, halt the CPU until an exception occurs
    // This is common since the CPU doesn't have an actual halt function
    if (opcode == 0x1000FFFF && !nextOpcode)
        Core::cpuRunning = false;

    // Add a 16-bit offset to the program counter if two registers are equal
    delaySlot = programCounter;
    if (registersR[(opcode >> 21) & 0x1F] == registersR[(opcode >> 16) & 0x1F])
        programCounter += ((int16_t)opcode << 2) - 4;
}

void CPU::INST_STR(bne)(uint32_t opcode)
{
    // Add a 16-bit offset to the program counter if two registers aren't equal
    delaySlot = programCounter;
    if (registersR[(opcode >> 21) & 0x1F] != registersR[(opcode >> 16) & 0x1F])
        programCounter += ((int16_t)opcode << 2) - 4;
}

void CPU::INST_STR(addi)(uint32_t opcode)
{
    // Add a signed 16-bit immediate to a register and store the lower result
    // On overflow, trigger an integer overflow exception
    int32_t op1 = registersR[(opcode >> 21) & 0x1F];
    int32_t op2 = (int16_t)opcode;
    int32_t value = op1 + op2;
    if (!((op1 ^ op2) & (1 << 31)) && ((op1 ^ value) & (1 << 31)))
        return CPU_CP0::exception(12);
    *registersW[(opcode >> 16) & 0x1F] = value;
}

void CPU::INST_STR(addiu)(uint32_t opcode)
{
    // Add a signed 16-bit immediate to a register and store the lower result
    int32_t value = registersR[(opcode >> 21) & 0x1F] + (int16_t)opcode;
    *registersW[(opcode >> 16) & 0x1F] = value;
}

void CPU::INST_STR(andi)(uint32_t opcode)
{
    // Bitwise and a register with a 16-bit immediate and store the result
    uint64_t value = registersR[(opcode >> 21) & 0x1F] & (opcode & 0xFFFF);
    *registersW[(opcode >> 16) & 0x1F] = value;
}

void CPU::INST_STR(ori)(uint32_t opcode)
{
    // Bitwise or a register with a 16-bit immediate and store the result
    uint64_t value = registersR[(opcode >> 21) & 0x1F] | (opcode & 0xFFFF);
    *registersW[(opcode >> 16) & 0x1F] = value;
}

void CPU::INST_STR(lui)(uint32_t opcode)
{
    // Load a 16-bit immediate into the upper 16 bits of a register
    *registersW[(opcode >> 16) & 0x1F] = (int16_t)opcode << 16;
}

void CPU::INST_STR(bnel)(uint32_t opcode)
{
    // Add a 16-bit offset to the program counter if two registers aren't equal
    // Otherwise, discard the delay slot opcode
    delaySlot = programCounter;
    if (registersR[(opcode >> 21) & 0x1F] != registersR[(opcode >> 16) & 0x1F])
        programCounter += ((int16_t)opcode << 2) - 4;
    else
        nextOpcode = 0;
}

void CPU::INST_STR(lh)(uint32_t opcode)
{
    // Load a signed half-word from memory at base register plus immeditate offset
    // TODO: unaligned address exception
    uint32_t address = registersR[(opcode >> 21) & 0x1F] + (int16_t)opcode;
    *registersW[(opcode >> 16) & 0x1F] = (int16_t)Memory::read<uint16_t>(address);
}

void CPU::INST_STR(lw)(uint32_t opcode)
{
    // Load a signed word from memory at base register plus immeditate offset
    // TODO: unaligned address exception
    uint32_t address = registersR[(opcode >> 21) & 0x1F] + (int16_t)opcode;
    *registersW[(opcode >> 16) & 0x1F] = (int32_t)Memory::read<uint32_t>(address);
}

void CPU::INST_STR(sh)(uint32_t opcode)
{
    // Store a half-word to memory at base register plus immeditate offset
    // TODO: unaligned address exception
    uint32_t address = registersR[(opcode >> 21) & 0x1F] + (int16_t)opcode;
    Memory::write<uint16_t>(address, registersR[(opcode >> 16) & 0x1F]);
}

void CPU::INST_STR(sw)(uint32_t opcode)
{
    // Store a word to memory at base register plus immeditate offset
    // TODO: unaligned address exception
    uint32_t address = registersR[(opcode >> 21) & 0x1F] + (int16_t)opcode;
    Memory::write<uint32_t>(address, registersR[(opcode >> 16) & 0x1F]);
}

void CPU::INST_STR(cache)(uint32_t opcode)
{
    // The cache isn't emulated, so just warn about an unknown operation
    LOG_WARN("Unknown cache operation: 0x%02X\n", (opcode >> 16) & 0x1F);
}

void CPU::INST_STR(sll)(uint32_t opcode)
{
    // Shift a register left by a 5-bit immediate and store the lower result
    int32_t value = registersR[(opcode >> 16) & 0x1F] << ((opcode >> 6) & 0x1F);
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(sllv)(uint32_t opcode)
{
    // Shift a register left by a register and store the lower result
    int32_t value = registersR[(opcode >> 16) & 0x1F] << (registersR[(opcode >> 21) & 0x1F] & 0x1F);
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(srlv)(uint32_t opcode)
{
    // Shift a register right by a register and store the lower result
    int32_t value = (uint32_t)registersR[(opcode >> 16) & 0x1F] >> (registersR[(opcode >> 21) & 0x1F] & 0x1F);
    *registersW[(opcode >> 11) & 0x1F] = value;
}


void CPU::INST_STR(jr)(uint32_t opcode)
{
    // Jump to an address stored in a register
    delaySlot = programCounter;
    programCounter = registersR[(opcode >> 21) & 0x1F] - 4;
}


void CPU::INST_STR(mflo)(uint32_t opcode)
{
    // Copy the low word of the mult/div result to a register
    *registersW[(opcode >> 11) & 0x1F] = lo;
}


void CPU::INST_STR(multu)(uint32_t opcode)
{
    // Multiply two 32-bit unsigned registers and set the 64-bit result
    uint64_t value = (uint64_t)(uint32_t)registersR[(opcode >> 16) & 0x1F] *
        (uint32_t)registersR[(opcode >> 21) & 0x1F];
    hi = value >> 32;
    lo = (int32_t)value;
}


void CPU::INST_STR(add)(uint32_t opcode)
{
    // Add a register to a register and store the lower result
    // On overflow, trigger an integer overflow exception
    int32_t op1 = registersR[(opcode >> 21) & 0x1F];
    int32_t op2 = registersR[(opcode >> 16) & 0x1F];
    int32_t value = op1 + op2;
    if (!((op1 ^ op2) & (1 << 31)) && ((op1 ^ value) & (1 << 31)))
        return CPU_CP0::exception(12);
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(addu)(uint32_t opcode)
{
    // Add a register to a register and store the result
    int32_t value = registersR[(opcode >> 21) & 0x1F] + registersR[(opcode >> 16) & 0x1F];
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(subu)(uint32_t opcode)
{
    // Subtract a register from a register and store the result
    int32_t value = registersR[(opcode >> 21) & 0x1F] - registersR[(opcode >> 16) & 0x1F];
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(and_)(uint32_t opcode)
{
    // Bitwise and a register with a register and store the result
    uint64_t value = registersR[(opcode >> 21) & 0x1F] & registersR[(opcode >> 16) & 0x1F];
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(or_)(uint32_t opcode)
{
    // Bitwise or a register with a register and store the result
    uint64_t value = registersR[(opcode >> 21) & 0x1F] | registersR[(opcode >> 16) & 0x1F];
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(xor_)(uint32_t opcode)
{
    // Bitwise exclusive or a register with a register and store the result
    uint64_t value = registersR[(opcode >> 21) & 0x1F] ^ registersR[(opcode >> 16) & 0x1F];
    *registersW[(opcode >> 11) & 0x1F] = value;
}


void CPU::INST_STR(sltu)(uint32_t opcode)
{
    // Check if a register is less than another register, and store the result
    bool value = registersR[(opcode >> 21) & 0x1F] < registersR[(opcode >> 16) & 0x1F];
    *registersW[(opcode >> 11) & 0x1F] = value;
}

void CPU::INST_STR(bgezal)(uint32_t opcode)
{
    // Add a 16-bit offset to the program counter if a register is greater or equal to zero
    // Also, save the return address
    delaySlot = programCounter;
    *registersW[31] = programCounter + 4;
    if ((int64_t)registersR[(opcode >> 21) & 0x1F] >= 0)
        programCounter += ((int16_t)opcode << 2) - 4;
}


void CPU::mfc0(uint32_t opcode)
{
    // Copy a 32-bit CP0 register value to a CPU register
    *registersW[(opcode >> 16) & 0x1F] = CPU_CP0::read((opcode >> 11) & 0x1F);
}

void CPU::mtc0(uint32_t opcode)
{
    // Copy a 32-bit CPU register value to a CP0 register
    CPU_CP0::write((opcode >> 11) & 0x1F, registersR[(opcode >> 16) & 0x1F]);
}
void CPU::INST_STR(cop0)(uint32_t opcode)
{
    // Trigger a CP0 unusable exception if CP0 is disabled
    if (!CPU_CP0::cpUsable(0))
        return CPU_CP0::exception(11);

    // Look up a CP0 instruction
    switch ((opcode >> 21) & 0x1F)
    {
        case 0x00: return mfc0(opcode);
        case 0x04: return mtc0(opcode);
        case 0x10: {
			UNIMPLEMENT;
        }
        default:   return unk(opcode);
    }
}


void CPU::unk(uint32_t opcode)
{
    // Warn about unknown instructions
    LOG_CRIT("Unknown CPU opcode: 0x%08X @ 0x%X\n", opcode, programCounter - 4);
}