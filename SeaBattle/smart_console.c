#include "smart_console.h"
#include <locale.h>
#include <Windows.h>

static HANDLE hStdOutput;

void init_console(int visible, unsigned long cursor_size)
{
	//setlocale(LC_ALL, "Russian");
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	set_cursor_info(visible, cursor_size);
}

void set_cursor_info(int visible, unsigned long cursor_size)
{
	CONSOLE_CURSOR_INFO ci;
	GetConsoleCursorInfo(hStdOutput, &ci);
	ci.bVisible = visible;
	ci.dwSize = cursor_size;
	SetConsoleCursorInfo(hStdOutput, &ci);
}

void set_cursor_pos(int x, int y)
{
	const COORD c = {x, y};
	SetConsoleCursorPosition(hStdOutput, c);	
}

void smart_print(int x, int y, char symbol)
{
	set_cursor_pos(x, y);
	printf("%c", symbol);
}