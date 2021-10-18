#include <stdlib.h>
#include <ncurses.h>
#include "lines.h"
#include "fio.h"
#include "render.h"
#include "logic.h"

void init_editor(editor_state *a, char *fname)
{
    a->filename = fname;
    a->root = readfile(fname);
    a->current = a->top = a->root;
    a->current = a->root;
    a->virt_y = a->real_y = 1;
    a->virt_x = a->real_x = a->saved_real_x = 1;
    a->offset_x = a->saved_offset_x = 0;
    a->rerender_flag = 1;
    a->exit_flag = 1;
    a->edit_flag = 0;
}

void editor(char *fname)
{
    editor_state ed;
    init_editor(&ed, fname);
    if (!initscr())
    {
        printf("Ncurses initialization error.\n");
        exit(1);
    }
    WINDOW *win = newwin(LINES - 1, COLS, 0, 0);
    noecho();
    keypad(stdscr, TRUE);
    set_escdelay(100);
    raw();
    unsigned curr_lines = LINES;
    int key = 0;
    int rerender_flag = 1;
    while (ed.exit_flag)
    {
        curs_set(0);
        render_interface(ed, key);
        refresh();
        if (ed.rerender_flag)
        {
            render_text(win, ed);
            box(win, 0, 0);
            wrefresh(win);
        }
        ed.rerender_flag = 0;
        move(ed.virt_y, ed.virt_x);
        curs_set(1);
        get_wch(&key);
        process_key(key, win, &ed);
    }
    delwin(win);
    endwin();
    clrmem(ed.root);
}