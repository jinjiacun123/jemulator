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

#include "rsp.h"
#include "core.h"
#include "log.h"
#include "memory.h"
#include "rsp_cp0.h"

namespace RSP
{
  
    uint32_t programCounter;
}

void RSP::reset()
{
    writePC(0);
}

uint32_t RSP::readPC()	//[by jim] need
{
    // Get the effective bits of the RSP program counter
    return (programCounter + 4) & 0xFFC;
}

void RSP::writePC(uint32_t value) //[by jim] need
{
    // Set the effective bits of the RSP program counter
    programCounter = 0xA4001000 | ((value - 4) & 0xFFC);
}
