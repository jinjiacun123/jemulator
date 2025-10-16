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
#include "rsp_cp0.h"
#include "log.h"
#include "memory.h"
#include "mi.h"
#include "rdp.h"


namespace RSP_CP0
{
    uint32_t memAddr;
    uint32_t dramAddr;
    uint32_t status;
    uint32_t semaphore;
}

void RSP_CP0::reset() //[by jim] need
{
    // Reset the RSP CP0 to its initial state
    memAddr = 0;
    dramAddr = 0;
    status = 0x1;
    semaphore = 0;
}

void RSP_CP0::write(int index, uint32_t value)//[by jim] need
{
    // Write to an RSP CP0 register if one exists at the given index
    switch (index)
    {
        case 0: // SP_MEM_ADDR
			UNIMPLEMENT;

        case 1: // SP_DRAM_ADDR		
			UNIMPLEMENT;
        case 2: // SP_RD_LEN			
			UNIMPLEMENT;	

        case 3: // SP_WR_LEN		
			UNIMPLEMENT;		

        case 4: // SP_STATUS
            // Set or clear the halt flag and update the RSP's state
            if (value & 0x1)
                status &= ~0x1;
            else if (value & 0x2)
                status |= 0x1;

            // Clear the broke flag
            if (value & 0x4)
                status &= ~0x2;

            // Set or clear the remaining status bits
            for (int i = 0; i < 20; i += 2)
            {
                if (value & (1 << (i + 5)))
                    status &= ~(1 << ((i / 2) + 5));
                else if (value & (1 << (i + 6)))
                    status |= (1 << ((i / 2) + 5));
            }

            // Keep track of unimplemented bits that should do something
            if (uint32_t bits = (status & 0x20))
                LOG_WARN("Unimplemented RSP CP0 status bits set: 0x%X\n", bits);
            return;		

        case 7: // SP_SEMAPHORE			
			UNIMPLEMENT;		

        case  8: case  9: case 10: case 11:
        case 12: case 13: case 14: case 15:			
			UNIMPLEMENT;


        default:
            LOG_WARN("Write to unknown RSP CP0 register: %d\n", index);
            return;
    }
}

