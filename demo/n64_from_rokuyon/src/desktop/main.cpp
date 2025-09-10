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
#include "../common.h"
#if MY_DEVELOP
#include <unistd.h>
#include "../core.h"
#include "my_lcd.h"

int main(int argc, char *argv[]){
	 myLcd * lcd = new myLcd();
	 lcd->setView();
	if (!Core::bootRom(argv[1]))  //[by jim] load rom
    {
        printf("Make sure the ROM file is accessible and try again.");
        return 0;
    }
	
#if 1	
	for(;;){                     // emulator main loop
		lcd->process();
		lcd->draw();
		sleep(3);
	}
#endif
	delete lcd;
}
#else
#include "ry_app.h"

// Let wxWidgets handle the main function
wxIMPLEMENT_APP(ryApp);
#endif
