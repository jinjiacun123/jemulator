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
#include "common.h"
#include "cpu_cp0.h"
#include "core.h"
#include "cpu.h"
#include "cpu_cp1.h"
#include "log.h"
#include "memory.h"
#include "mi.h"

namespace CPU_CP0
{
    uint32_t _index;
    uint32_t entryLo0;
    uint32_t entryLo1;
    uint32_t context;
    uint32_t pageMask;
    uint32_t badVAddr;
    uint32_t count;
    uint32_t entryHi;
    uint32_t compare;
    uint32_t status;
    uint32_t cause;
    uint32_t epc;
    uint32_t errorEpc;

    bool irqPending;
    uint32_t startCycles;
    uint32_t endCycles;

    void scheduleCount();
    void updateCount();
    void interrupt();


}

void CPU_CP0::reset()
{
    // Reset the CPU CP0 to its initial state
    _index = 0;
    entryLo0 = 0;
    entryLo1 = 0;
    context = 0;
    pageMask = 0;
    badVAddr = 0;
    count = 0;
    entryHi = 0;
    compare = 0;
    status = 0x400004;
    cause = 0;
    epc = 0;
    errorEpc = 0;
    irqPending = false;
    endCycles = -1;
    scheduleCount();
}

int32_t CPU_CP0::read(int index)
{
	UNIMPLEMENT;
}

void CPU_CP0::write(int index, int32_t value) //[by jim] need
{
    // Write to a CPU CP0 register if one exists at the given index
    switch (index)
    {
        case 0: // Index
			UNIMPLEMENT

        case 2: // EntryLo0
			UNIMPLEMENT

        case 3: // EntryLo1
			UNIMPLEMENT

        case 4: // Context
			UNIMPLEMENT

        case 5: // PageMask
			UNIMPLEMENT

        case 9: // Count
#if 1			
            // Set the count register and reschedule its next update
            count = value;
            scheduleCount();
            return;
#else
			UNIMPLEMENT
#endif

        case 10: // EntryHi
			UNIMPLEMENT

        case 11: // Compare
#if 1			
            // Set the compare register and acknowledge a timer interrupt
            compare = value;
            cause &= ~0x8000;
            
            // Update the count register and reschedule its next update
            count += ((Core::globalCycles - startCycles) >> 2);
            scheduleCount();
            return;
#else
			UNIMPLEMENT
#endif			

        case 12: // Status
			UNIMPLEMENT


        case 13: // Cause
#if 1			
            // Set the software interrupt flags
            cause = (cause & ~0x300) | (value & 0x300);
            checkInterrupts();
            return;
#else
			UNIMPLEMENT
#endif

        case 14: // EPC
			UNIMPLEMENT

        case 30: // ErrorEPC
			UNIMPLEMENT

        default:
            LOG_WARN("Write to unknown CPU CP0 register: %d\n", index);
            return;
    }
}

void CPU_CP0::resetCycles()//[by jim] need
{
    // Adjust the cycle counts for a cycle reset
    startCycles -= Core::globalCycles;
    endCycles -= Core::globalCycles;
}

void CPU_CP0::scheduleCount()//[by jim] need
{
    // Assuming count is updated, schedule its next update
    // This is done as close to match as possible, with a limit to prevent cycle overflow
    startCycles = Core::globalCycles;
    uint32_t cycles = startCycles + std::min<uint32_t>((compare - count) << 2, 0x40000000);
    cycles += (startCycles == cycles) << 2;

    // Only reschedule if the update is sooner than what's already scheduled
    // This helps prevent overloading the scheduler when registers are used excessively
    if (endCycles > cycles)
    {
        Core::schedule(updateCount, cycles - startCycles);
        endCycles = cycles;
    }
}

void CPU_CP0::updateCount()//[by jim] need
{
#if 1
    // Ignore the update if it was rescheduled
    if (Core::globalCycles != endCycles)
        return;

    // Update count and request a timer interrupt if it matches compare
    if ((count += ((endCycles - startCycles) >> 2)) == compare)
    {
        cause |= 0x8000;
        checkInterrupts();
    }

    // Schedule the next update unconditionally
    endCycles = -1;
    scheduleCount();
#else
	UNIMPLEMENT;
#endif
}

void CPU_CP0::checkInterrupts()//[by jim] need
{
#if 1
    // Set the external interrupt bit if any MI interrupt is set
    cause = (cause & ~0x400) | ((bool)(MI::interrupt & MI::mask) << 10);

    // Schedule an interrupt if able and an enabled bit is set
    if (((status & 0x3) == 0x1) && (status & cause & 0xFF00) && !irqPending)
    {
        Core::schedule(interrupt, 2); // 1 CPU cycle
        irqPending = true;
    }
#else
	UNIMPLEMENT;
#endif
}

void CPU_CP0::interrupt()
{
	UNIMPLEMENT;
}

void CPU_CP0::exception(uint8_t type)//[by jim] need
{
#if 1
    // Update registers for an exception and jump to the handler
    // TODO: handle nested exceptions
    status |= 0x2; // EXL
    cause = (cause & ~0x8000007C) | ((type << 2) & 0x7C);
    epc = CPU::programCounter - (type ? 4 : 0);
    CPU::programCounter = ((status & (1 << 22)) ? 0xBFC00200 : 0x80000000) - 4;
    CPU::nextOpcode = 0;

    // Adjust the exception vector based on the type
    if ((type & ~1) != 2) // Not TLB miss
        CPU::programCounter += 0x180;

    // Return to the preceding branch if the exception occured in a delay slot
    if (CPU::delaySlot != -1)
    {
        epc = CPU::delaySlot - 4;
        cause |= (1 << 31); // BD
    }

    // Unhalt the CPU if it was idling
    Core::cpuRunning = true;
#else
	UNIMPLEMENT;
#endif
}

void CPU_CP0::setTlbAddress(uint32_t address)//[by jim] need
{
#if 1
    // Set the address that caused a TLB exception
    badVAddr = address;
    entryHi = address & 0xFFFFE000;
    context = (context & ~0x7FFFF0) | ((address >> 9) & 0x7FFFF0);
#else
	UNIMPLEMENT;
#endif
}

bool CPU_CP0::cpUsable(uint8_t cp)//[by jim] need
{
    // Check if a coprocessor is usable (CP0 is always usable in kernel mode)
    if (!(status & (1 << (28 + cp))) && (cp > 0 || (!(status & 0x6) && (status & 0x18))))
    {
        // Set the coprocessor number bits
        cause = (cause & ~(0x3 << 28)) | ((cp & 0x3) << 28);
        return false;
    }

    return true;
}
