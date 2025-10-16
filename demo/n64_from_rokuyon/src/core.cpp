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

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <thread>
#include <vector>

#include "common.h"
#include "core.h"
#include "cpu.h"
#include "cpu_cp0.h"
#include "log.h"
#include "memory.h"
#include "mi.h"
#include "pi.h"
#include "pif.h"
#include "rdp.h"
#include "rsp_cp0.h"
#include "si.h"
#include "vi.h"

struct Task
{
    Task(void (*function)(), uint32_t cycles):
        function(function), cycles(cycles) {}

    void (*function)();
    uint32_t cycles;

    bool operator<(const Task &task) const
    {
        return cycles < task.cycles;
    }
};

namespace Core
{
    std::thread *emuThread;
    std::condition_variable condVar;
    std::mutex waitMutex;

    bool running;
    bool cpuRunning;

    std::vector<Task> tasks;
    uint32_t globalCycles;
    uint32_t cpuCycles;
    uint8_t *rom;
    uint32_t romSize;
	
    void runLoop();
    void resetCycles();
}

bool Core::bootRom(const std::string &path)//[by jim]:<---------------------------
{
#if 1 //[by jim] game rom file read to rom buffer
    // Try to open the specified ROM file
    FILE *romFile = fopen(path.c_str(), "rb");
    if (!romFile) return false;

    // Ensure the emulator is stopped
    stop();
	
    // Load the ROM into memory
    if (rom) delete[] rom;
    fseek(romFile, 0, SEEK_END);
    romSize = ftell(romFile);
    fseek(romFile, 0, SEEK_SET);
    rom = new uint8_t[romSize];
    fread(rom, sizeof(uint8_t), romSize, romFile);
    fclose(romFile);
#endif //[by jim] get rom data and romSize
    // Derive the save path from the ROM path

    // Reset the scheduler
    cpuRunning = true;
    tasks.clear();
    globalCycles = 0;
    cpuCycles = 0;
    schedule(resetCycles, 0x7FFFFFFF);

#if 1 //[by jim] some reset 
    // Reset the emulated components
    Memory::reset();
   // AI::reset();		//[by jim]: audio stream deal
    CPU::reset();       //[by jim]: main cpu interpreter
    CPU_CP0::reset();
    MI::reset();
    PI::reset();
    SI::reset();
    VI::reset();
    PIF::reset();		//[by jim]: about bios
    RDP::reset();
    RSP_CP0::reset();
#endif

    // Start the emulator
    start();
    return true;
}

void Core::start()//[by jim]:<---------------------------
{
    // Start the threads if emulation wasn't running
    if (!running)
    {
        running = true;
        emuThread = new std::thread(runLoop);//[by jim]:<--------------
    }
	printf("finish start child pthread\n");
}

void Core::stop()//[by jim] need 
{
    if (running)
    {
        {
            // Signal for the threads to stop
            std::lock_guard<std::mutex> guard(waitMutex);
            running = false;
            condVar.notify_one();
        }

        // Stop the threads if emulation was running
        emuThread->join();
        delete emuThread;
        RDP::finishThread();
    }
}

void Core::runLoop()//[by jim] need
{
    while (running)
    {
        // Run the CPUs until the next scheduled task
        while (tasks[0].cycles > globalCycles)
        {
            // Run a CPU opcode if ready and schedule the next one
            if (cpuRunning && globalCycles >= cpuCycles)	//[by jim] 1. main cpu run
            {
                CPU::runOpcode();							//[by jim] finish reduce instructions
                cpuCycles = globalCycles + 2;
            }			

            // Jump to the next soonest opcode
            globalCycles = std::min<uint32_t>(cpuRunning ? cpuCycles : -1, -1);
        }

        // Jump to the next scheduled task
        globalCycles = tasks[0].cycles;

        // Run all tasks that are scheduled now
        while (tasks[0].cycles <= globalCycles)
        {
            (*tasks[0].function)();                          // [by jim] 3. task scheduled
            tasks.erase(tasks.begin());
        }
    }
}

void Core::resetCycles()//[by jim] need
{
    // Reset the cycle counts to prevent overflow
    CPU_CP0::resetCycles();
    for (size_t i = 0; i < tasks.size(); i++)
        tasks[i].cycles -= globalCycles;
    cpuCycles -= std::min(globalCycles, cpuCycles);
    globalCycles -= globalCycles;

    // Schedule the next cycle reset
    schedule(resetCycles, 0x7FFFFFFF);
}

void Core::schedule(void (*function)(), uint32_t cycles)//[by jim] need
{
    // Add a task to the scheduler, sorted by least to most cycles until execution
    // Cycles run at 93.75 * 2 MHz
    Task task(function, globalCycles + cycles);
    auto it = std::upper_bound(tasks.cbegin(), tasks.cend(), task);
    tasks.insert(it, task);
}
