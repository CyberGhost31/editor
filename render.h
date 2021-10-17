#pragma once
#include <ncurses.h>

void render_text(WINDOW *win, editor_state ed);

void render_interface(editor_state ed, int key);