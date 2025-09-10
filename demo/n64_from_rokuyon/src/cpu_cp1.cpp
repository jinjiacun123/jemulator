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

#include <cmath>
#include <cstring>
#include <fenv.h>

#include "common.h"
#include "cpu_cp1.h"
#include "cpu.h"
#include "log.h"

namespace CPU_CP1
{
    bool fullMode;
    uint64_t registers[32];
    uint32_t status;

    float &getFloat(int index);
    double &getDouble(int index);
}

void CPU_CP1::reset()	 //[by jim] need
{
    // Reset the CPU CP1 to its initial state
    fullMode = false;
    memset(registers, 0, sizeof(registers));
    status = 0;
}


