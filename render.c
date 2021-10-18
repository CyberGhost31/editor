#include <ncurses.h>
#include "editor.h"
#include "lines.h"

void output_line(WINDOW *win, line *a, int offset)
{
    size_t c = 1;
    wchar_t b[2];
    b[1] = 0;
    for (size_t i = 0; i < a->length; i++)
    {
        if (a->str[i] != '\t')
        {
            if (offset)
                offset--;
            else
            {
                b[0] = a->str[i];
                waddwstr(win, b);
                c++;
            }
        }
        else
        {
            if (offset >= 4)
                offset -= 4;
            else
            {
                for (short j = 0; j < 4 - offset; j++)
                {
                    waddch(win, (wchar_t)
                        ' ');
                    c++;
                }
            }
        }
        if (c == COLS - 1)
            break;
    }
}

void render_text(WINDOW *win, editor_state ed)
{
    for (int i = 1; i < LINES - 2; i++)
    {
        wmove(win, i, 1);
        if (ed.top != NULL)
        {
            output_line(win, ed.top, ed.offset_x);
            ed.top = ed.top->next;
        }
        else
            waddch(win, '~');
        waddch(win, '\n');
    }
}

void render_interface(editor_state ed, int key)
{
    move(LINES - 1, 1);
    insertln();
    attron(A_REVERSE);
    printw("real: %d/%d; (%d)", ed.real_y, ed.real_x, key);
    attroff(A_REVERSE);
}