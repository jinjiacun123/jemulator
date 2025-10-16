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
#include "mi.h"
#include "cpu_cp0.h"
#include "log.h"
#include <stdlib.h>

namespace MI //[by jim] MIPS Interface
{
    uint32_t mask;
}

void MI::reset()    //[by jim] need use
{
    // Reset the MI to its initial state   
    mask = 0;
}

void MI::write(uint32_t address, uint32_t value)	//[by jim] need use
{
    // Write to an I/O register if one exists at the given address
    switch (address)
    {
        case 0x4300000: // MI_MODE        	
            // Acknowledge a DP interrupt when bit 11 is set
            return;

        case 0x430000C: // MI_MASK
           /*
			   0x0430000C - MI_INTR_MASK_REG (Read / Write)
			   This register sets up a mask. If (MI_INTR_REG & MI_INTR_MASK_REG) != 0, then a MIPS interrupt is raised.
			   
			   Writes
			   Bit Explanation
			   0   Clear SP Mask
			   1   Set SP Mask
			   2   Clear SI Mask
			   3   Set SI Mask
			   4   Clear AI Mask
			   5   Set AI Mask
			   6   Clear VI Mask
			   7   Set VI Mask
			   8   Clear PI Mask
			   9   Set PI Mask
			   10  Clear DP Mask
			   11  Set DP Mask
           */
            // For each set bit, set or clear a mask bit appropriately
            for (int i = 0; i < 12; i += 2)
            {
                if (value & (1 << i))			//odd
                    mask &= ~(1 << (i / 2));	//[by jim]:set i/2 bit where to 0,other is container. clear i/2 bit
                else if (value & (1 << (i + 1)))//even
                    mask |= (1 << (i / 2));     //[by jim]: set 1 to i/2 bit
            }
            return;

        default:
            LOG_WARN("Unknown MI register write: 0x%X\n", address);
            return;
    }	
}
