#pragma once
#include <ncurses.h>

void get_virt_x(editor_state *ed);

void process_key(int key, WINDOW *win, editor_state *ed);