#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

struct ln_t
{
    char *str;
    unsigned size;
    unsigned length;
    struct ln_t *next;
    struct ln_t *prev;
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

line *initln(char *string, unsigned size, unsigned length)
{
    line *root;
    root = (line *)malloc(sizeof(line));
    root->str = string;
    root->size = size;
    root->length = length;
    root->next = NULL;
    root->prev = NULL;
    return (root);
}

line *addln(line *ln, char *string, unsigned size, unsigned length)
{
    line *temp, *p;
    temp = (line *)malloc(sizeof(line));
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

char *myfgets(FILE *file, unsigned *size, unsigned *length)
{
    int k = 1, i = 0;
    char *str;
    str = (char *)malloc(256 * sizeof(char));
    int c;
    while ((c = fgetc(file)) != EOF && c != '\n')
    {
        if (i == 256 * k - 1)
            str = realloc(str, 256 * (++k));
        str[i++] = c;
    }
    str[i] = 0;
    *length = i;
    *size = 256 * k;

    if (feof(file))
        return NULL;
    else
        return str;
}

line *readfile(char fname[])
{
    FILE *file;
    file = fopen(fname, "r");
    line *root, *current;
    long pos;
    char *temp;
    unsigned size, length;
    if (file == NULL)
    {
        temp = (char *)malloc(256 * sizeof(char));
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
            temp = myfgets(file, &size, &length);
            root = initln(temp, size, length);
            current = root;
            while (NULL != (temp = myfgets(file, &size, &length)))
                current = addln(current, temp, size, length);
        }
        else
        {
            temp = (char *)malloc(256 * sizeof(char));
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
    do
    {
        free(current->str);
        next = current->next;
        free(current);
        current = next;

    } while (current != NULL);
}

void print(WINDOW *win, line *a, int offset)
{
    int c = 1;
    for (int i = 0; i < a->length; i++)
    {
        if(a->str[i] != '\t')
        {
            if (offset)
                offset--;
            else
            {
                waddch(win, (unsigned) a->str[i]);
                c++;
            }
        }
        else
        {
            if (offset >= 4)
                offset-=4;
            else
            {
                for (int j = 0; j < 4 - offset; j++)
                {    
                    waddch(win, (unsigned) ' ');
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
    curs_set(0);
    for (int i = 1; i < LINES - 2; i++)
    {
        wmove(win, i, 1);
        if (ed.top != NULL)
        {
            print(win, ed.top, ed.offset_x);
            waddch(win, '\n');
            ed.top= ed.top->next;
        }
        else
            break;
    }
    box(win, 0, 0);
    wrefresh(win);
}

void get_virt_x(editor_state *ed)
{
    int tabs = 0;
    int num_of_chars;
    for (int i = 0; i < ed->real_x - 1; i++)
        if (ed->current->str[i] == 9) tabs++;
    num_of_chars =  ed->real_x - tabs + tabs * 4;
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

void render_interface(editor_state ed)
{
    curs_set(0);
    move(LINES - 1, 1);
    insertln();
    attron(A_REVERSE);
    printw("real: %d/%d; virt: %d/%d; offset = %d; rerender = %d", ed.real_y, ed.real_x, ed.virt_y, ed.virt_x, ed.offset_x, ed.rerender_flag);
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

void process_key(int key, editor_state *ed)
{
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
        else if ((key == KEY_DOWN) && (ed->current->next->next != NULL))
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
    int curr_cols = COLS, curr_lines = LINES;
    int key = 0;
    int rerender_flag = 1;
    while (key != 27)
    {
        render_interface(ed);
        if (COLS != curr_cols || LINES != curr_lines)
            ed.rerender_flag = 1;
        if (ed.rerender_flag)
            render_text(win, ed);
        ed.rerender_flag = 0;
        move(ed.virt_y, ed.virt_x);
        curs_set(1);
        key = getch();
        process_key(key, &ed);
    }
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
        printf("Usage : editor [filename]\n");
        exit(1);
    }
    return 0;
}