#ifndef SMART_CONSOLE_H
#define SMART_CONSOLE_H

#include <stdio.h>

void init_console(int visible, unsigned long cursor_size);
void set_cursor_info(int visible, unsigned long cursor_size);
void set_cursor_pos(int x, int y);
void smart_print(int x, int y, char symbol);

#endif