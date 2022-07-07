/*
    XenoIPS for Nintendo DS
    Copyright (C) 2018-2020 JeffRuLz
    Copyright (C) 2022-present lifehackerhansol

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "main.h"
#include "menu.h"
#include "message.h"
#include <time.h>

#define VERSION "0.0.1"

PrintConsole topScreen;
PrintConsole bottomScreen;

enum {
	MAIN_MENU_PATCH,
	MAIN_MENU_EXIT
};

static void _setupScreens()
{
	REG_DISPCNT = MODE_FB0;
	VRAM_A_CR = VRAM_ENABLE;

	videoSetMode(MODE_0_2D);
	videoSetModeSub(MODE_0_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	consoleInit(&topScreen,    3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, true,  true);
	consoleInit(&bottomScreen, 3, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);

	clearScreen(&bottomScreen);

	VRAM_A[100] = 0xFFFF;
}

static int _mainMenu(int cursor)
{
	//top screen
	clearScreen(&topScreen);

	iprintf("\tXenoIPS for NDS\n");
	iprintf("\nversion %s\n", VERSION);
	iprintf("\x1b[22;0HJeff - 2018-2019");
	iprintf("\x1b[23;0Hlifehackerhansol - 2022");

	//menu
	Menu* m = newMenu();
	setMenuHeader(m, "MAIN MENU");

	addMenuItem(m, "Patch IPS file", NULL, 0);
	addMenuItem(m, "Shut Down", NULL, 0);

	m->cursor = cursor;

	//bottom screen
	printMenu(m);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
			printMenu(m);

		if (keysDown() & KEY_A)
			break;
	}

	int result = m->cursor;
	freeMenu(m);

	return result;
}

int main(int argc, char **argv)
{
	srand(time(0));
	_setupScreens();

	//setup sd card access
	if (!fatInitDefault())
	{
		messageBox("fatInitDefault()...\x1B[31mFailed\n\x1B[47m");
		return 0;
	}

	//main menu
	bool programEnd = false;
	int cursor = 0;

	while (!programEnd)
	{
		cursor = _mainMenu(cursor);

		switch (cursor)
		{
			case MAIN_MENU_PATCH:
				patchSelectMenu();
				break;

			case MAIN_MENU_EXIT:
				programEnd = true;
				break;
		}
	}

	return 0;
}

void clearScreen(PrintConsole* screen)
{
	consoleSelect(screen);
	consoleClear();
}
