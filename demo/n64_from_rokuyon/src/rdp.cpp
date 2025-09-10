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
#include <cstring>
#include <mutex>
#include <thread>
#include <vector>

#include "common.h"
#include "rdp.h"
#include "log.h"
#include "memory.h"
#include "mi.h"
#include "settings.h"

namespace RDP
{     
    std::thread *thread;
    bool running; 
}
 
void RDP::reset()
{
}

void RDP::finishThread()//[by jim]need
{
    // Stop the thread if it was running
    if (running)
    {
        running = false;
        thread->join();
        delete thread;
    }	

} 
