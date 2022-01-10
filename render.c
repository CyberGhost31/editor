#include <ncurses.h>
#include "editor.h"
#include "lines.h"
#include "highlight_c.h"
#include "fio.h"

void waddstrfrag(WINDOW *win, wchar_t *a, size_t left, size_t rigth, size_t *offset, int attr)
{
    for (size_t i = left; i <= rigth; i++)
    {
	if (a[i] != '\t')
        {
            if (*offset > 0)
                (*offset)--;
            else
                waddch(win, a[i] | attr);
        }
        else
        {
            if (*offset >= 4)
                *offset -= 4;
            else
            {
                for (short j = 0; j < 4 - *offset; j++)
                    waddch(win, ' ');
                *offset = 0;
            }
        }
    }
}

void output_text_line(WINDOW *win, line *a, size_t offset)
{
    if (a->length != 0)
        waddstrfrag(win, a->str, 0, a->length - 1, &offset, COLOR_PAIR(1));
}

void render_text(WINDOW *win, editor_state ed)
{
    size_t i = 1;
    int t = get_file_type(ed.filename);
    if (t == 0)
    {
        for (i; i < LINES - 2; i++)
        {
            wmove(win, i, 1);
            if (ed.top != NULL)
            {
                output_text_line(win, ed.top, ed.offset_x);
                ed.top = ed.top->next;
            }
            else
                waddch(win, '~');
            waddch(win, '\n');
        }
    }
    else if (t == 1)
    {
        int start_render = 0;
        c_token_type gl = undefined;
        while (i < LINES - 2)
        {
            wmove(win, i, 1);
            if (ed.root != NULL)
            {
                if (ed.root == ed.top)
                    start_render = 1;
                output_c_line(win, ed.root->str, ed.root->length, start_render, ed.offset_x, &gl);
                ed.root = ed.root->next;
            }
            else
                waddch(win, '~');
            waddch(win, '\n');
            if (start_render == 1)
                i++;
        }
    }
    
}

void render_interface(editor_state ed)
{
    size_t y, x;
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    move(LINES - 1, 1);
    insertln();
    attron(A_REVERSE | COLOR_PAIR(1));
    printw("[%s][%c] %d/%d", (ed.filename == NULL) ? "UNTITLED" : ed.filename, (ed.edit_flag == 1) ? '+' : ' ', ed.real_y, ed.real_x);
    getyx(stdscr, y, x);
    for (int i = x; i < COLS; i++)
        addch(' ');
    attroff(A_REVERSE);
}