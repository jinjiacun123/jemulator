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

#include <atomic>
#include <cstring>
#include <queue>
#include <thread>
#include <vector>
#include "common.h"
#include "ai.h"
#include "core.h"
#include "log.h"
#include "memory.h"
#include "mi.h"
#include "settings.h"

#define MAX_BUFFERS 4
#define SAMPLE_COUNT 1024
#define OUTPUT_RATE 48000
#define OUTPUT_SIZE SAMPLE_COUNT * sizeof(uint32_t)

struct Samples
{
    uint32_t address;
    uint32_t count;
};

namespace AI	//[by jim] audio interface
{
    uint32_t bufferOut[SAMPLE_COUNT];
    std::atomic<bool> ready;

    Samples samples[2];
    std::queue<std::vector<uint32_t>> buffers;
    uint32_t offset;

    uint32_t dramAddr;
    uint32_t control;
    uint32_t frequency;
    uint32_t status;

    void createBuffer();
    void submitBuffer();
    void processBuffer();
}

void AI::fillBuffer(uint32_t *out)//[by jim] need use
{
#if 1
    // Try to wait until a buffer is ready, but don't stall the audio callback too long
    std::chrono::steady_clock::time_point waitTime = std::chrono::steady_clock::now();
    while (!ready.load())
    {
        if (std::chrono::steady_clock::now() - waitTime > std::chrono::microseconds(1000000 / 60))
        {      
            // If a buffer isn't ready in time, fill the output with the last played sample
            for (int i = 0; i < SAMPLE_COUNT; i++)
                out[i] = bufferOut[SAMPLE_COUNT - 1];
            return;
        }
    }

    // Output the buffer and mark it as used
    memcpy(out, bufferOut, OUTPUT_SIZE);
    ready.store(false);
#else
	printf("unimplement [%s:%d]-->%s\n", __FUNCTION__, __LINE__, __FILE__);
	exit(-1);
#endif
}

void AI::reset()//[by jim] need use
{
    // Reset the AI to its initial state
    dramAddr = 0;
    control = 0;
    frequency = 0;
    status = 0;

    // Schedule the first audio buffer to output
    Core::schedule(createBuffer, (uint64_t)SAMPLE_COUNT * (93750000 * 2) / OUTPUT_RATE);
}

void AI::write(uint32_t address, uint32_t value)//[by jim] need use
{
#if 1
    // Write to an I/O register if one exists at the given address
    switch (address)
    {
        case 0x4500000: // AI_DRAM_ADDR
			UNIMPLEMENT;


        case 0x4500004: // AI_LENGTH
			UNIMPLEMENT;

        case 0x4500008: // AI_CONTROL
			UNIMPLEMENT;


        case 0x450000C: // AI_STATUS
#if 1        
            // Acknowledge an AI interrupt
            MI::clearInterrupt(2);
            return;
#else
		printf("unimplement [%s:%d]-->%s\n", __FUNCTION__, __LINE__, __FILE__);
			exit(-1);
#endif

        case 0x4500010: // AI_DAC_RATE
			UNIMPLEMENT;


        default:
            LOG_WARN("Unknown AI register write: 0x%X\n", address);
            return;
    }
#else
	printf("unimplement [%s:%d]-->%s\n", __FUNCTION__, __LINE__, __FILE__);
	exit(-1);
#endif
}

void AI::createBuffer()//[by jim] need use
{
    // Mark the buffer as ready and schedule the next one
    ready.store(true);
    Core::schedule(createBuffer, (uint64_t)SAMPLE_COUNT * (93750000 * 2) / OUTPUT_RATE);
}

void AI::submitBuffer()//[by jim] modify
{

    // Schedule the logical completion of the AI DMA based on sample count and frequency
    Core::schedule(processBuffer, (uint64_t)samples[0].count * (93750000 * 2) / frequency);
}
