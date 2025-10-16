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
	//[by jim]: to use by tlb
	uint32_t entryHi;

	
    uint32_t context;
    uint32_t pageMask;
    uint32_t badVAddr;
	
    uint32_t count;
    
    uint32_t compare;
    uint32_t status;
    uint32_t cause;   

    bool irqPending;
    uint32_t startCycles;
    uint32_t endCycles;

    void scheduleCount();
    void updateCount();

}

void CPU_CP0::reset()
{
    // Reset the CPU CP0 to its initial state
	entryHi = 0;

    context = 0;
    pageMask = 0;
    badVAddr = 0;
    count = 0;
    
    compare = 0;
    status = 0x400004;
    irqPending = false;
    endCycles = -1;
    scheduleCount();
}


void CPU_CP0::write(int index, int32_t value) //[by jim] need
{
    // Write to a CPU CP0 register if one exists at the given index
    switch (index)
    {

        case 9: // Count
#if 1			
            // Set the count register and reschedule its next update
            count = value;
            scheduleCount();
            return;
#else
			UNIMPLEMENT
#endif

        case 11: // Compare
#if 1			
            // Set the compare register and acknowledge a timer interrupt
            compare = value;
			
            // Update the count register and reschedule its next update
            count += ((Core::globalCycles - startCycles) >> 2);
            scheduleCount();
            return;
#else
			UNIMPLEMENT
#endif			

        case 13: // Cause
#if 1			
            // Set the software interrupt flags
            return;
#else
			UNIMPLEMENT
#endif

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
		;
    }

    // Schedule the next update unconditionally
    endCycles = -1;
    scheduleCount();
#else
	UNIMPLEMENT;
#endif
}


void CPU_CP0::exception(uint8_t type)//[by jim] need,  where type in (3, 11, 12)
										/**
										* type(or exception code) == 3 => TLB Miss - Store(Thrown when no valid TLB entry is found when translating an address to be used for a store (data access))
										* type == 11 => Coprocessor Unusable (Thrown when a coprocessor instruction is used when that coprocessor is disabled. Note that COP0 is never disabled)
										* type == 12 => Arithmetic Overflow(hrown by arithmetic instructions when their operations overflow.)
										*/
{
#if 1
    // Update registers for an exception and jump to the handler
    // TODO: handle nested exceptions
    status |= 0x2; // EXL
#if 1// [by jim] ??
    CPU::nextOpcode = 0;
#endif	

    // Unhalt the CPU if it was idling
    Core::cpuRunning = true;
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
        return false;
    }

    return true;
}
