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

#ifndef CORE_H
#define CORE_H

#include <cstdint>
#include <string>

namespace Core
{
    extern bool running;
    extern bool cpuRunning;
    extern uint32_t globalCycles;

    extern uint8_t *rom;
    extern uint32_t romSize;

    bool bootRom(const std::string &path);
    void start();
    void stop();
	
    void schedule(void (*function)(), uint32_t cycles);
}

#endif // CORE_H
