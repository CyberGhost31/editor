#pragma once
#include <ncurses.h>
#include "editor.h"

void waddstrfrag(WINDOW *win, wchar_t *a, size_t left, size_t rigth, size_t *offset, int attr);

void render_text(WINDOW *win, editor_state ed);

void render_interface(editor_state ed, int key);