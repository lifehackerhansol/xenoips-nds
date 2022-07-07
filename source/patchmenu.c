#include "main.h"
#include "menu.h"
#include "message.h"
#include "storage.h"
#include "xenoips.h"
#include <dirent.h>

enum {
	INSTALL_MENU_INSTALL,
	INSTALL_MENU_BACK
};

static char currentDir[512] = "";

static void generateList(Menu* m);
static void printItem(Menu* m);
static int subMenu(int menuNumber);

static void _setHeader(Menu* m)
{
	if (!m) return;
	if (currentDir[0] == '\0')
		setMenuHeader(m, "/");
	else
		setMenuHeader(m, currentDir);
}

bool runIPSpatcher(char* targetFile, char* patchFile) {
	clearScreen(&bottomScreen);
	char resultFile[PATH_MAX];
	sprintf(resultFile, "%s_patch", targetFile);
	FILE* origin = fopen(targetFile, "rb");
	if(getFileSize(origin) > 16777216) {
		iprintf("File to patch is too big!\nIPS does not support >16MB\nfiles.\n");
		return false;
	}
	iprintf("Patching file.\n");
	copyFile(targetFile, resultFile);
	FILE* target = fopen(resultFile, "r+b");
	FILE* ips = fopen(patchFile, "rb");
	int ret = _ipspatch(target, ips);
	fclose(ips);
	fclose(target);
	switch(ret) {
		case 0:
			return true;
		default:
			return false;
	}
}

void targetSelectMenu(char* patchFile)
{
	Menu* m = newMenu();
	_setHeader(m);
	generateList(m);
	consoleSelect(&topScreen);
	iprintf("Select a file to patch");
	consoleSelect(&bottomScreen);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
		{
			if (m->changePage != 0)
				generateList(m);

			printMenu(m);
			printItem(m);
		}

		//back
		if (keysDown() & KEY_B)
		{
			char* ptr = strrchr(currentDir, '/');

			if (ptr)
			{
				*ptr = '\0';
				_setHeader(m);
				resetMenu(m);
				generateList(m);
				printMenu(m);
			}
			else
			{
				break;
			}
		}

		else if (keysDown() & KEY_X)
			break;

		//selection
		else if (keysDown() & KEY_A)
		{
			if (m->itemCount > 0)
			{
				if (m->items[m->cursor].directory == false)
				{
					//nds file
					switch (subMenu(1))
					{
						case INSTALL_MENU_INSTALL:
							if(runIPSpatcher(m->items[m->cursor].value, patchFile)) {
								iprintf("\x1B[42m");	//green
								iprintf("\nPatch complete.\n");
							} else {
								iprintf("\x1B[31m");	//red
								iprintf("\nPatch fail.\n");
							}
							iprintf("\x1B[47m");	//white
							iprintf("Back - [B]\n");
							keyWait(KEY_A | KEY_B);

							freeMenu(m);
							return;

						case INSTALL_MENU_BACK:					
							break;
					}
				}
				else
				{
					//directory
					sprintf(currentDir, "%s", m->items[m->cursor].value);
					_setHeader(m);
					resetMenu(m);
					generateList(m);
				}

				printMenu(m);
			}
		}
	}

	freeMenu(m);
}

void patchSelectMenu()
{
	Menu* m = newMenu();
	_setHeader(m);
	generateList(m);
	consoleSelect(&topScreen);
	iprintf("Select a patch file");
	consoleSelect(&bottomScreen);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
		{
			if (m->changePage != 0)
				generateList(m);

			printMenu(m);
			printItem(m);
		}

		//back
		if (keysDown() & KEY_B)
		{
			char* ptr = strrchr(currentDir, '/');

			if (ptr)
			{
				*ptr = '\0';
				_setHeader(m);
				resetMenu(m);
				generateList(m);
				printMenu(m);
			}
			else
			{
				break;
			}
		}

		else if (keysDown() & KEY_X)
			break;

		//selection
		else if (keysDown() & KEY_A)
		{
			if (m->itemCount > 0)
			{
				if (m->items[m->cursor].directory == false)
				{
					//nds file
					switch (subMenu(0))
					{
						case INSTALL_MENU_INSTALL:
							targetSelectMenu(m->items[m->cursor].value);
							freeMenu(m);
							return;

						case INSTALL_MENU_BACK:					
							break;
					}
				}
				else
				{
					//directory
					sprintf(currentDir, "%s", m->items[m->cursor].value);
					_setHeader(m);
					resetMenu(m);
					generateList(m);
				}

				printMenu(m);
			}
		}
	}

	freeMenu(m);
}

static void generateList(Menu* m)
{
	if (!m) return;

	//reset menu
	clearMenu(m);

	m->page += sign(m->changePage);
	m->changePage = 0;

	bool done = false;

	struct dirent* ent;
	DIR* dir = NULL;

	if (currentDir[0] == '\0')
		dir = opendir("/");
	else
		dir = opendir(currentDir);	

	if (dir)
	{
		int count = 0;

		//scan /dsi/
		while ( (ent = readdir(dir)) && !done)
		{
			if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0)
				continue;

			if (ent->d_type == DT_DIR)
			{
				if (count < m->page * ITEMS_PER_PAGE)
						count += 1;
			
				else
				{
					if (m->itemCount >= ITEMS_PER_PAGE)
						done = true;
					
					else
					{
						char* fpath = (char*)malloc(strlen(currentDir) + strlen(ent->d_name) + 8);
						sprintf(fpath, "%s/%s", currentDir, ent->d_name);

						addMenuItem(m, ent->d_name, fpath, 1);
					}
				}
			}
			else
			{
				if (count < m->page * ITEMS_PER_PAGE)
					count += 1;
				
				else
				{
					if (m->itemCount >= ITEMS_PER_PAGE)
						done = true;
					
					else
					{
						char* fpath = (char*)malloc(strlen(currentDir) + strlen(ent->d_name) + 8);
						sprintf(fpath, "%s/%s", currentDir, ent->d_name);

						addMenuItem(m, ent->d_name, fpath, 0);

						free(fpath);
					}
				}
			}
		}
	}

	closedir(dir);

	m->nextPage = done;

	if (m->cursor >= m->itemCount)
		m->cursor = m->itemCount - 1;

	printItem(m);
	printMenu(m);
}

static void printItem(Menu* m)
{
	if (!m) return;
	if (m->itemCount <= 0) return;

	if (m->items[m->cursor].directory)
		clearScreen(&topScreen);
}

static int subMenu(int menuNumber)
{
	int result = -1;

	Menu* m = newMenu();

	addMenuItem(m, menuNumber ? "Use Patch" : "Apply Patch", NULL, 0);
	addMenuItem(m, "Back - [B]", NULL, 0);

	printMenu(m);

	while (1)
	{
		swiWaitForVBlank();
		scanKeys();

		if (moveCursor(m))
			printMenu(m);

		if (keysDown() & KEY_B)
		{
			result = -1;
			break;
		}

		else if (keysDown() & KEY_A)
		{
			result = m->cursor;
			break;
		}
	}

	freeMenu(m);
	return result;
}
