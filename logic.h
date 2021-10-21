#pragma once
#include "editor.h"
#include <ncurses.h>



void get_virt_x(editor_state *ed);

void process_key(int key, WINDOW *win, editor_state *ed);

void enter_name(editor_state *ed);