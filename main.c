#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <wchar.h>

struct ln_t
{
    wchar_t * str;
    size_t size;
    size_t length;
    struct ln_t * next;
    struct ln_t * prev;
};

typedef struct ln_t line;

struct ed_st_t
{
    char *filename;
    int virt_y, virt_x;
    int real_y, real_x, saved_real_x;
    int offset_x, saved_offset_x;
    line *root, *top, *current;
    int rerender_flag;
};

typedef struct ed_st_t editor_state;

line *initln(wchar_t *string, size_t size, size_t length)
{
    line * root;
    root = (line*) malloc(sizeof(line));
    root->str = string;
    root->size = size;
    root->length = length;
    root->next = NULL;
    root->prev = NULL;
    return root;
}

line *addln(line *ln, wchar_t *string, size_t size, size_t length)
{
    line *temp, *p;
    temp = (line*) malloc(sizeof(line));
    p = ln->next;
    ln->next = temp;
    temp->str = string;
    temp->next = p;
    temp->prev = ln;
    temp->size = size;
    temp->length = length;
    if (p != NULL)
        p->prev = temp;
    return (temp);
}

line *delln(line *ln)
{
    line *prev, *next;
    prev = ln->prev;
    next = ln->next;
    if (prev != NULL)
        prev->next = ln->next;
    if (next != NULL)
        next->prev = ln->prev;
    free(ln);
    return (prev);
}

wchar_t fget_wc(FILE *file)
{
    unsigned char b1, b2, b3, b4;
    wchar_t code;
    b1 = fgetc(file);
    if (b1 <= 0x7F)
    {
        code = b1;
    }
    else if (b1 >= 0xF8)
        return -1;
    else if (b1 >= 0xF0)
    {
        b2 = fgetc(file);
        b3 = fgetc(file);
        b4 = fgetc(file);
        code = ((b1 & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
    }
    else if (b1 >= 0xE0)
    {
        b2 = fgetc(file);
        b3 = fgetc(file);
        code = ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
    }
    else if (b1 >= 0xC0)
    {
        b2 = fgetc(file);
        code = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
    }
    return code;
}

wchar_t *fget_ws(FILE *file, size_t *size, size_t *length)
{
    size_t k = 1, i = 0;
    wchar_t *str;
    str = (wchar_t*) malloc(256* sizeof(wchar_t));
    unsigned code;
    while ((code = fget_wc(file)) != EOF && code != '\n')
    {
        if (i == 256 *k - 1)
            str = realloc(str, 256 *(++k) *sizeof(wchar_t));
        str[i++] = code;
    }
    str[i] = 0;
    *length = i;
    *size = 256 * k;
    return str;
}

line *readfile(char fname[])
{
    FILE * file;
    file = fopen(fname, "r");
    line *root, *current;
    long pos;
    wchar_t * temp;
    size_t size, length;
    if (file == NULL)
    {
        temp = (wchar_t*) malloc(256* sizeof(wchar_t));
        temp[0] = 0;
        root = initln(temp, 256, 0);
    }
    else
    {
        fseek(file, 0, SEEK_END);
        pos = ftell(file);
        rewind(file);
        if (pos)
        {
            temp = fget_ws(file, &size, &length);
            root = initln(temp, size, length);
            current = root;
            while (!feof(file))
            {
                temp = fget_ws(file, &size, &length);
                current = addln(current, temp, size, length);
            }
        }
        else
        {
            temp = (wchar_t*) malloc(256 * sizeof(wchar_t));
            temp[0] = 0;
            root = initln(temp, 256, 0);
        }
        fclose(file);
    }
    return root;
}

void clrmem(line *root)
{
    line *current, *next;
    current = root;
    do {
        free(current->str);
        next = current->next;
        free(current);
        current = next;
    } while (current != NULL);
}

void print(WINDOW *win, line *a, int offset)
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

void get_virt_x(editor_state *ed)
{
    size_t tabs = 0;
    size_t num_of_chars;
    for (int i = 0; i < ed->real_x - 1; i++)
        if (ed->current->str[i] == 9) tabs++;
    num_of_chars = ed->real_x - tabs + tabs * 4;
    if (ed->saved_offset_x > ed->offset_x)
        ed->offset_x = ed->saved_offset_x;
    if (num_of_chars - ed->offset_x < 1)
    {

        ed->virt_x = num_of_chars % (COLS - 1);
        ed->offset_x = num_of_chars - ed->virt_x;
        ed->rerender_flag = 1;
    }
    else if (num_of_chars - ed->offset_x > COLS - 2)
    {
        ed->virt_x = COLS - 2;
        ed->offset_x = num_of_chars - ed->virt_x;
        ed->rerender_flag = 1;
    }
    else
        ed->virt_x = num_of_chars - ed->offset_x;
}

void render_text(WINDOW *win, editor_state ed)
{
    curs_set(0);
    for (int i = 1; i < LINES - 2; i++)
    {
        wmove(win, i, 1);
        if (ed.top != NULL)
        {
            print(win, ed.top, ed.offset_x);
            ed.top = ed.top->next;
        }
        else
            waddch(win, '~');
        waddch(win, '\n');
    }
    box(win, 0, 0);
    wrefresh(win);
}

void render_interface(editor_state ed, int key)
{
    curs_set(0);
    move(LINES - 1, 1);
    insertln();
    attron(A_REVERSE);
    printw("real: %d/%d; \"%s\"", ed.real_y, ed.real_x, ed.current->str);
    attroff(A_REVERSE);
    refresh();
}

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
}

void insertChr(editor_state *ed, wchar_t chr)
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
}

void delChr(editor_state *ed)
{
    if (ed->real_x <= ed->current->length)
    {
        for (size_t i = ed->real_x - 1; i <= ed->current->length; i++)
            ed->current->str[i] = ed->current->str[i + 1];
        ed->current->length--;
    }
}

/*void enter(editor_state *ed)
{
    size_t size = 0, length;
    wchar_t *temp;
    
    length = ed->current->length - ed->real_x + 1;
    
    while (length <= size)
        size += 256;
    temp = (wchar_t*) malloc(size* sizeof(wchar_t));
    for (size_t i = 0; i <= length; i++)
        temp[i] = ed->current->str[ed->real_x + i - 1];
    ed->current->str[ed->real_x - 1] = 0;
    ed->current->length = ed->real_x - 1;
    addln(ed->current, temp, size, length);
    ed->rerender_flag = 1;
    ed->current = ed->current->next;
    ed->real_x = ed->saved_real_x = 1;
    ed->real_y++;
    if (ed->virt_y < LINES - 3)
        ed->virt_y++;
    else
        ed->top = ed->top->next;
    ed->offset_x = ed->saved_offset_x = 0;

}*/

void enter(editor_state *ed)
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
    ed->rerender_flag = 1;
    ed->current = ed->current->next;
    ed->real_x = ed->saved_real_x = 1;
    ed->real_y++;
    if (ed->virt_y < LINES - 3)
        ed->virt_y++;
    else
        ed->top = ed->top->next;
    ed->offset_x = ed->saved_offset_x = 0;
}


void save(editor_state *ed)
{
    FILE * file;
    file = fopen(ed->filename, "w");
    line *current;
    current = ed->root;
    do {
        for (int i = 0; i < current->length; i++)
            fputwc(current->str[i], file);
        if ((current = current->next) != NULL)
            fputwc((wchar_t)'\n', file);
    } while (current != NULL);
    fclose(file);

}

void process_key(int key, editor_state *ed)
{
    if (key == 27)
        save(ed);
    if ((key == KEY_UP) && (ed->current->prev != NULL))
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
    else if ((key == KEY_DOWN) && (ed->current->next != NULL))
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
    else if ((key == KEY_LEFT) && (1 < ed->real_x))
    {
        ed->real_x--;
        ed->saved_real_x = ed->real_x;
        if (ed->virt_x == 1)
        {
            ed->offset_x--;
            ed->saved_offset_x = ed->offset_x;
            ed->rerender_flag = 1;
        }
    }
    else if ((key == KEY_RIGHT) && (ed->real_x <= ed->current->length))
    {
        ed->real_x++;
        ed->saved_real_x = ed->real_x;
        if (ed->virt_x == COLS - 2)
        {
            ed->offset_x++;
            ed->saved_offset_x = ed->offset_x;
            ed->rerender_flag = 1;
        }
    }
    else if ((key == KEY_BACKSPACE || key == 127 || key == '\b'))
    {
        if (1 < ed->real_x)
        {
            ed->real_x--;
            ed->saved_real_x = ed->real_x;
            if (ed->virt_x == 1)
            {
                ed->offset_x--;
                ed->saved_offset_x = ed->offset_x;
            }
            delChr(ed);
        }
        else if (ed->real_x == 1 && ed->current->prev != NULL)
        {
            ed->real_y--;
            
            while (ed->current->length + ed->current->prev->length + 1 >= ed->current->prev->size)
                ed->current->prev->size += 256;
            ed->current->prev->str = realloc(ed->current->prev->str, ed->current->prev->size * sizeof(wchar_t));
            
            for (size_t i = 0; i < ed->current->length; i++)
                ed->current->prev->str[ed->current->prev->length + i] = ed->current->str[i];
            ed->real_x = ed->current->prev->length + 1;
            ed->current->prev->length += ed->current->length;
            ed->current->prev->str[ed->current->prev->length] = 0;
            ed->current = ed->current->prev;
            delln(ed->current->next);
            if (1 < ed->virt_y)
                ed->virt_y--;
            else
                ed->top = ed->top->prev;
        }
        ed->rerender_flag = 1;
    }
    else if (key == KEY_DC)
    {
        if (ed->real_x <= ed->current->length)
        {
            delChr(ed);
        }
        else if (ed->real_x == ed->current->length + 1 && ed->current->next != NULL)
        {
            while (ed->current->length + ed->current->next->length + 1 >= ed->current->size)
                ed->current->size += 256;
            ed->current->str = realloc(ed->current->str, ed->current->size * sizeof(wchar_t));
            for (size_t i = 0; i < ed->current->next->length; i++)
                ed->current->str[ed->current->length + i] = ed->current->next->str[i];
            ed->current->length += ed->current->next->length;
            ed->current->prev->str[ed->current->prev->length] = 0;
            delln(ed->current->next);
        }
        ed->rerender_flag = 1;
    }
    else if (key == '\n' || key == KEY_ENTER)
    {
        enter(ed);
    }
    else if (((key >= ' ') || key == '\t') && !(key == KEY_DC || key == KEY_BACKSPACE || key == 127 || key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT))
    {
        insertChr(ed, key);
        ed->real_x++;
        ed->saved_real_x = ed->real_x;
        if (ed->virt_x == COLS - 2)
        {
            ed->offset_x++;
            ed->saved_offset_x = ed->offset_x;
        }
        ed->rerender_flag = 1;
    }
    get_virt_x(ed);
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
    unsigned curr_lines = LINES;
    int key = 0;
    int rerender_flag = 1;
    while (key != 27)
    {
        render_interface(ed, key);
        if (key == 410)
        {
            if (LINES != curr_lines)
            {
                wresize(win, LINES - 1, COLS);

                if (ed.virt_y > LINES - 3)
                {
                    while (ed.virt_y > LINES - 3)
                    {
                        ed.virt_y--;
                        ed.top = ed.top->next;
                    }
                }
                curr_lines = LINES;
            }
            ed.rerender_flag = 1;
        }
        if (ed.rerender_flag)
            render_text(win, ed);
        ed.rerender_flag = 0;
        move(ed.virt_y, ed.virt_x);
        curs_set(1);
        get_wch(&key);
        process_key(key, &ed);
    }
    delwin(win);
    endwin();
    clrmem(ed.root);
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    if (argc > 1)
    {
        editor(argv[1]);
    }
    else
    {
        printf("Usage : editor[filename], %d\n", EOF);
        exit(1);
    }
    return 0;
}