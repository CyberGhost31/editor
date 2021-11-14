#include <stddef.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include "lines.h"
#include "editor.h"
#include "fio.h"

void insert_char(editor_state *ed, wchar_t chr)
{
    size_t temp1, temp2;
    if (ed->current->length - 2 == ed->current->size)
    {
        ed->current->size += 256;
        ed->current->str = realloc(ed->current->str, ed->current->size);
    }
    temp1 = chr;
    for (size_t i = ed->real_x - 1; i <= ed->current->length; i++)
    {
        temp2 = ed->current->str[i];
        ed->current->str[i] = temp1;
        temp1 = temp2;
    }
    ed->current->length++;
    ed->edit_flag = 1;
}

void del_char(editor_state *ed)
{
    if (ed->real_x <= ed->current->length)
    {
        for (size_t i = ed->real_x - 1; i <= ed->current->length; i++)
            ed->current->str[i] = ed->current->str[i + 1];
        ed->current->length--;
    }
}

void process_enter(editor_state *ed)
{
    size_t size = 0, length;
    wchar_t *temp;
    if (ed->current->length == 0)
        length = 0;
    else
        length = ed->current->length - ed->real_x + 1;
    do {
        size += 256;
    } while (length >= size);
    temp = (wchar_t*) malloc(size * sizeof(wchar_t));
    for (size_t i = ed->real_x - 1; i < ed->current->length; i++)
        temp[i - ed->real_x + 1] = ed->current->str[i];
    temp[length] = 0;
    ed->current->str[ed->real_x - 1] = 0;
    ed->current->length = ed->real_x - 1;
    addln(ed->current, temp, size, length);
    ed->current = ed->current->next;
    ed->real_x = ed->saved_real_x = 1;
    ed->real_y++;
    if (ed->virt_y < LINES - 3)
        ed->virt_y++;
    else
        ed->top = ed->top->next;
    ed->offset_x = 0;
    ed->edit_flag = 1;
    ed->rerender_flag = 1;
}

void get_virt_x(editor_state *ed)
{
    size_t tabs = 0;
    size_t num_of_chars;
    for (int i = 0; i < ed->real_x - 1; i++)
        if (ed->current->str[i] == 9) tabs++;
    num_of_chars = ed->real_x - tabs + tabs * 4;
    if ((int)(num_of_chars - ed->offset_x) < 1)
    {

        ed->virt_x = num_of_chars % (COLS - 2);
        if (ed->virt_x == 0)
            ed->virt_x = COLS - 2;
        ed->offset_x = num_of_chars - ed->virt_x;
        ed->rerender_flag = 1;
    }
    else if ((int)(num_of_chars - ed->offset_x) > COLS - 2)
    {
        ed->virt_x = COLS - 2;
        ed->offset_x = num_of_chars - ed->virt_x;
        ed->rerender_flag = 1;
    }
    else
        ed->virt_x = num_of_chars - ed->offset_x;
}

void process_up(editor_state *ed)
{
    if (ed->current->prev != NULL)
    {
        ed->real_y--;
        ed->current = ed->current->prev;

        if (ed->current->length < ed->saved_real_x)
            ed->real_x = ed->current->length + 1;
        else
            ed->real_x = ed->saved_real_x;
        if (1 < ed->virt_y)
            ed->virt_y--;
        else
        {
            ed->top = ed->top->prev;
            ed->rerender_flag = 1;
        }
    }
}

void process_down(editor_state *ed)
{
    if (ed->current->next != NULL)
    {
        ed->real_y++;
        ed->current = ed->current->next;
        if (ed->current->length < ed->saved_real_x)
            ed->real_x = ed->current->length + 1;
        else
            ed->real_x = ed->saved_real_x;
        if (ed->virt_y < LINES - 3)
            ed->virt_y++;
        else
        {
            ed->top = ed->top->next;
            ed->rerender_flag = 1;
        }
    }
}

void process_left(editor_state *ed)
{
    if (1 < ed->real_x)
    {
        ed->real_x--;
        ed->saved_real_x = ed->real_x;
        if (ed->virt_x == 1)
        {
            ed->offset_x--;
            ed->rerender_flag = 1;
        }
    }
}

void process_rigth(editor_state *ed)
{
    if (ed->real_x <= ed->current->length)
    {
        ed->real_x++;
        ed->saved_real_x = ed->real_x;
        if (ed->virt_x == COLS - 2)
        {
            ed->offset_x++;
            ed->rerender_flag = 1;
        }
    }
}

void process_backspace(editor_state *ed)
{
    if (1 < ed->real_x)
    {
        ed->real_x--;
        ed->saved_real_x = ed->real_x;
        if (ed->virt_x == 1)
            ed->offset_x--;
        del_char(ed);
    }
    else if (ed->real_x == 1 && ed->current->prev != NULL)
    {
        ed->real_y--;    
        while (ed->current->length + ed->current->prev->length + 1 >= ed->current->prev->size)
            ed->current->prev->size += 16 * sizeof(wchar_t);
        ed->current->prev->str = realloc(ed->current->prev->str, ed->current->prev->size * sizeof(wchar_t));
        for (size_t i = 0; i < ed->current->length; i++)
            ed->current->prev->str[ed->current->prev->length + i] = ed->current->str[i];
        ed->real_x = ed->saved_real_x = ed->current->prev->length + 1;
        ed->current->prev->length += ed->current->length;
        ed->current->prev->str[ed->current->prev->length] = 0;
        ed->current = ed->current->prev;
        free(ed->current->next->str);
        delln(ed->current->next);
        if (1 < ed->virt_y)
            ed->virt_y--;
        else
            ed->top = ed->top->prev;
    }
    ed->edit_flag = 1;
    ed->rerender_flag = 1;
}

void process_delete(editor_state *ed)
{
    if (ed->real_x <= ed->current->length)
        del_char(ed);
    else if (ed->real_x == ed->current->length + 1 && ed->current->next != NULL)
    {
        while (ed->current->length + ed->current->next->length + 1 >= ed->current->size)
            ed->current->size += 16 * sizeof(wchar_t);
        ed->current->str = realloc(ed->current->str, ed->current->size * sizeof(wchar_t));
        for (size_t i = 0; i < ed->current->next->length; i++)
            ed->current->str[ed->current->length + i] = ed->current->next->str[i];
        ed->current->length += ed->current->next->length;
        ed->current->str[ed->current->length] = 0;
        free(ed->current->next->str);
        delln(ed->current->next);
    }
    ed->edit_flag = 1;
    ed->rerender_flag = 1;
}

void process_symbolic(editor_state *ed, int key)
{
    insert_char(ed, key);
    ed->real_x++;
    ed->saved_real_x = ed->real_x;
    if (ed->virt_x == COLS - 2)
        ed->offset_x++;
    ed->edit_flag = 1;
    ed->rerender_flag = 1;
}

void process_change_term_size(editor_state *ed, WINDOW *win)
{
    wresize(win, LINES - 1, COLS);
    box(win, 0, 0);
    if (ed->virt_y > LINES - 3)
    {
        while (ed->virt_y > LINES - 3 && ed->virt_y > 1)
        {
            ed->virt_y--;
            ed->top = ed->top->next;
        }
    }
    ed->rerender_flag = 1;
}

void process_Home(editor_state *ed)
{
    ed->saved_real_x = ed->real_x = 1;
}

void process_End(editor_state *ed)
{
    ed->saved_real_x = ed->real_x = ed->current->length + 1;
}

void process_PgUp(editor_state *ed)
{
    for (size_t i = 1; (i < LINES - 2) && (ed->top->prev != NULL); i++)
    {
        ed->top = ed->top->prev;
        ed->real_y--;
    }
    ed->current = ed->top;
    ed->virt_y = 1;
    if (ed->current->length < ed->saved_real_x)
        ed->real_x = ed->current->length + 1;
    else
        ed->real_x = ed->saved_real_x;
    ed->rerender_flag = 1;

}

void process_PgDown(editor_state *ed)
{
    for (size_t i = 1; (i < LINES - 2) && (ed->top->next != NULL); i++)
    {
        ed->top = ed->top->next;
        ed->real_y++;
    }
    ed->current = ed->top;
    ed->virt_y = 1;
    if (ed->current->length < ed->saved_real_x)
        ed->real_x = ed->current->length + 1;
    else
        ed->real_x = ed->saved_real_x;
    ed->rerender_flag = 1;

}

void process_key(int key, WINDOW *win, editor_state *ed)
{
    switch (key)
    {
        case 'Q' & 0x1F:        ed->exit_flag = 0;                  break;
        case 'S' & 0xF1:        save(ed);                           break;
        case 'R' & 0x1F:        enter_name(ed);                     break;
        case KEY_UP:            process_up(ed);                     break;
        case KEY_DOWN:          process_down(ed);                   break;
        case KEY_LEFT:          process_left(ed);                   break;
        case KEY_RIGHT:         process_rigth(ed);                  break;
        case 127:               process_backspace(ed);              break;
        case KEY_DC:            process_delete(ed);                 break;
        case '\n':              process_enter(ed);                  break;
        case KEY_RESIZE:        process_change_term_size(ed, win);  break;
        case KEY_PPAGE:         process_PgUp(ed);                   break;
        case KEY_NPAGE:         process_PgDown(ed);                 break;
        case KEY_HOME:          process_Home(ed);                   break;
        case KEY_END:           process_End(ed);                    break;
        default: if (key >= ' ' || key == '\t') process_symbolic(ed, key);
    }
    get_virt_x(ed);
}