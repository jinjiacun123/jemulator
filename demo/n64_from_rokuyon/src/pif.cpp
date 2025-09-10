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

#include "common.h"
#include "pif.h"
#include "core.h"
#include "cpu.h"
#include "log.h"
#include "memory.h"
#include "settings.h"

namespace PIF
{
    uint8_t memory[0x800]; // 2KB-64B PIF ROM + 64B PIF RAM
    uint16_t eepromMask;
    uint8_t eepromId;

    uint8_t command;
    uint16_t buttons;
    int8_t stickX;
    int8_t stickY;

    extern void (*pifCommands[])(int);

    uint32_t crc32(uint8_t *data, size_t size);
    void joybusProtocol(int bit);
    void verifyChecksum(int bit);
    void clearMemory(int bit);
    void unknownCmd(int bit);
}

// Small command lookup table for PIF command bits
void (*PIF::pifCommands[7])(int) =
{
    joybusProtocol, unknownCmd,     unknownCmd, unknownCmd, // 0-3
    unknownCmd,     verifyChecksum, clearMemory             // 4-6
};

uint32_t PIF::crc32(uint8_t *data, size_t size)//[by jim] need
{
#if 1
    uint32_t r = 0xFFFFFFFF;

    // Calculate a CRC32 value for the given data
    for (size_t i = 0; i < size; i++)
    {
        r ^= data[i];
        for (int j = 0; j < 8; j++)
        {
            uint32_t t = ~((r & 1) - 1);
            r = (r >> 1) ^ (0xEDB88320 & t);
        }
    }

    return ~r;
#else
	UNIMPLEMENT;
#endif
}

void PIF::reset() //[by jim] need
{
    // Reset the PIF to its initial state
    clearMemory(0);
    command = 0;
    buttons = 0;
    stickX = 0;
    stickY = 0;

    // Set a mask and ID for 0.5KB/2KB EEPROM, or disable EEPROM
    switch (Core::saveSize)
    {
        case 0x200: eepromMask = 0x1FF; eepromId = 0x80; break;
        case 0x800: eepromMask = 0x7FF; eepromId = 0xC0; break;
        default:    eepromMask = 0x000; eepromId = 0x00; break;
    }

    // Set the CIC seed based on which bootcode is detected
    // This value is used during boot to calculate a checksum
    switch (uint32_t value = crc32(&Core::rom[0x40], 0x1000 - 0x40))
    {
        case 0x6170A4A1: // 6101
            LOG_INFO("Detected CIC chip 6101\n");
            Memory::write<uint8_t>(0xBFC007E6, 0x3F);
            break;

        case 0x90BB6CB5: // 6102
            LOG_INFO("Detected CIC chip 6102\n");
            Memory::write<uint8_t>(0xBFC007E6, 0x3F);
            break;

        case 0x0B050EE0: // 6103
            LOG_INFO("Detected CIC chip 6103\n");
            Memory::write<uint8_t>(0xBFC007E6, 0x78);
            break;

        case 0x98BC2C86: // 6105
            LOG_INFO("Detected CIC chip 6105\n");
            Memory::write<uint8_t>(0xBFC007E6, 0x91);
            break;

        case 0xACC8580A: // 6106
            LOG_INFO("Detected CIC chip 6106\n");
            Memory::write<uint8_t>(0xBFC007E6, 0x85);
            break;

        default:
            LOG_WARN("Unknown IPL3 CRC32 value: 0x%08X\n", value);
            break;
    }

    if (FILE *pifFile = fopen("pif_rom.bin", "rb"))
    {
        // Load the PIF ROM into memory if it exists
        fread(memory, sizeof(uint8_t), 0x7C0, pifFile);
        fclose(pifFile);
    }
    else
    {
        // Set CPU registers as if the PIF ROM was executed
        // Values from https://github.com/mikeryan/n64dev/blob/master/src/boot/pif.S
        *CPU::registersW[1]  = 0x0000000000000000;
        *CPU::registersW[2]  = 0xFFFFFFFFD1731BE9;
        *CPU::registersW[3]  = 0xFFFFFFFFD1731BE9;
        *CPU::registersW[4]  = 0x0000000000001BE9;
        *CPU::registersW[5]  = 0xFFFFFFFFF45231E5;
        *CPU::registersW[6]  = 0xFFFFFFFFA4001F0C;
        *CPU::registersW[7]  = 0xFFFFFFFFA4001F08;
        *CPU::registersW[8]  = 0x00000000000000C0;
        *CPU::registersW[9]  = 0x0000000000000000;
        *CPU::registersW[10] = 0x0000000000000040;
        *CPU::registersW[11] = 0xFFFFFFFFA4000040;
        *CPU::registersW[12] = 0xFFFFFFFFD1330BC3;
        *CPU::registersW[13] = 0xFFFFFFFFD1330BC3;
        *CPU::registersW[14] = 0x0000000025613A26;
        *CPU::registersW[15] = 0x000000002EA04317;
        *CPU::registersW[16] = 0x0000000000000000;
        *CPU::registersW[17] = 0x0000000000000000;
        *CPU::registersW[18] = 0x0000000000000000;
        *CPU::registersW[19] = 0x0000000000000000;
        *CPU::registersW[20] = 0x0000000000000001;
        *CPU::registersW[21] = 0x0000000000000000;
        *CPU::registersW[22] = Memory::read<uint8_t>(0xBFC007E6);
        *CPU::registersW[23] = 0x0000000000000006;
        *CPU::registersW[24] = 0x0000000000000000;
        *CPU::registersW[25] = 0xFFFFFFFFD73F2993;
        *CPU::registersW[26] = 0x0000000000000000;
        *CPU::registersW[27] = 0x0000000000000000;
        *CPU::registersW[28] = 0x0000000000000000;
        *CPU::registersW[29] = 0xFFFFFFFFA4001FF0;
        *CPU::registersW[30] = 0x0000000000000000;
        *CPU::registersW[31] = 0xFFFFFFFFA4001554;

        // Copy the IPL3 from ROM to DMEM and jump to the start address
        for (uint32_t i = 0; i < 0x1000; i++)
            Memory::write(0xA4000000 + i, Core::rom[i]);
        CPU::programCounter = 0xA4000040 - 4;
    }

    // Set the memory size to 4MB
    // TODO: I think IPL3 is supposed to set this, but stubbing RI_SELECT_REG to 1 skips it
    Memory::write<uint32_t>(0xA0000318, Settings::expansionPak ? 0x800000 : 0x400000);
}

 void PIF::clearMemory(int bit)//[by jim] need
{
#if 1
    // Clear the 64 bytes of PIF RAM
    memset(&memory[0x7C0], 0, 0x40);
#else
	UNIMPLEMENT;
#endif
}

