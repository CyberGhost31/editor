#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <curses.h>
#include <wchar.h>

struct ln_t
{
    unsigned *str;
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

line *initln(unsigned *string, unsigned size, unsigned length)
{
    line *root;
    root = (line *)malloc(sizeof(line));
    root->str = string;
    root->size = size;
    root->length = length;
    root->next = NULL;
    root->prev = NULL;
    return root;
}

line *addln(line *ln, unsigned *string, unsigned size, unsigned length)
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

unsigned *myfgets(FILE *file, unsigned *size, unsigned *length, int *eof_flag)
{
    int k = 1, i = 0;
    unsigned *str;
    str = (unsigned *)malloc(256 * sizeof(unsigned));
    unsigned char b1, b2, b3, b4;
    unsigned code;
    while ((b1 = fgetc(file)) != EOF && b1 != '\n')
    {
        code = 0;
        if(b1 <= 0x7F)
        {
            code = b1;
        }
        else if(b1 >= 0xF8)
            break;
        else if(b1 >= 0xF0)
        {
            b2 = fgetc(file);
            b3 = fgetc(file);
            b4 = fgetc(file);
            code = ((b1 & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
        }
        else if(b1 >= 0xE0)
        {
            b2 = fgetc(file);
            b3 = fgetc(file);
            code = ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
        }
        else if(b1 >= 0xC0)
        {
            b2 = fgetc(file);
            code = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
        }

	/*if(s[i]<=0x7F)
	{
		sim_code=s[i];
	}
        else if(s[i]>=0xF8)
	    break
        else if(s[i]>=0xF0)
        {
	    code = ((b1 & 0x07) << 18) | ((b2 & 0x3F) << 12) | ((b3 & 0x3F) << 6) | (b4 & 0x3F);
        }
        else if(s[i]>=0xE0)
	{
	code = ((b1 & 0x0F) << 12) | ((b2 & 0x3F) << 6) | (b3 & 0x3F);
	}
        else if(s[i]>=0xC0)
	{
	    code = ((b1 & 0x1F) << 6) | (b2 & 0x3F);
	}*/

        if (i == 256 * k - 1)
            str = realloc(str, 256 * (++k));
        str[i++] = code;
    }
    str[i] = 0;
    *length = i;
    *size = 256 * k;
    if (feof(file))
        *eof_flag = 1;
    return str;
}

line *readfile(char fname[])
{
    FILE *file;
    file = fopen(fname, "r");
    line *root, *current;
    long pos;
    unsigned *temp;
    unsigned size, length;
    int eof_flag = 0;
    if (file == NULL)
    {
        temp = (unsigned *)malloc(256 * sizeof(unsigned));
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
            temp = myfgets(file, &size, &length, &eof_flag);
            root = initln(temp, size, length);
            current = root;
            while (eof_flag != 1)
            {
                temp = myfgets(file, &size, &length, &eof_flag);
                current = addln(current, temp, size, length);
            }
        }
        else
        {
            temp = (unsigned *)malloc(256 * sizeof(unsigned));
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
    unsigned b[2];
    b[1] = 0;
    for (int i = 0; i < a->length; i++)
    {
        if(a->str[i] != '\t')
        {
            if (offset)
                offset--;
            else
            {
		b[0]=a->str[i];
                waddwstr(win, b);
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
            ed.top = ed.top->next;
        }
        else
            waddch(win, '~');
        waddch(win, '\n');
        
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

void render_interface(editor_state ed, int key)
{
    curs_set(0);
    move(LINES - 1, 1);
    insertln();
    attron(A_REVERSE);
    printw("real: %d/%d; (%d) = \'%c\'", ed.real_y, ed.real_x, key, key);
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

void insertChr(editor_state *ed, unsigned chr)
{
    unsigned temp1, temp2;
    if (ed->current->length - 2 == ed->current->size)
    {
        ed->current->size += 256;
        realloc(ed->current->str, ed->current->size);
    }
    temp1 = chr;
    for (unsigned i = ed->real_x - 1; i <= ed->current->length; i++)
    {
        temp2 = ed->current->str[i];
        ed->current->str[i] = temp1;
        temp1 = temp2;
    }
    ed->current->length++;
}

void delChr(editor_state *ed, unsigned chr)
{
    if (ed->real_x <= ed->current->length)
    {
        for (unsigned i = ed->real_x - 1; i <= ed->current->length; i++)
            ed->current->str[i] = ed->current->str[i + 1];
        ed->current->length--;
    }
}

void save(editor_state *ed)
{
    FILE *file;
    file = fopen(ed->filename, "w");
    line *current;
    current = ed->root;
    do
    {
        for (int i = 0; i < current->length; i++)
           fputwc(current->str[i], file);
        if ((current = current->next) != NULL)
            fputc('\n', file);

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
        else if ((key == '\t' || key == KEY_BACKSPACE || key == 127) && (1 < ed->real_x))
        {
            ed->real_x--;
            ed->saved_real_x = ed->real_x;
            if (ed->virt_x == 1)
            {
                ed->offset_x--;
                ed->saved_offset_x = ed->offset_x;
            }
            delChr(ed, key);
            ed->rerender_flag = 1;

        }
        else if (key == KEY_DC)
        {
            delChr(ed, key);
            ed->rerender_flag = 1;
        }
        else if ((key >= ' ') && !(key == KEY_DC || key == KEY_BACKSPACE || key == '\t' || key == 127 || key == KEY_UP || key == KEY_DOWN || key == KEY_LEFT || key == KEY_RIGHT))
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

int getch_unicode()
{
    unsigned b1, b2, b3, b4;
    b1 = getch();
    int code = 0;
    if(b1 <= 0x7F)
    {
        code = b1;
    }
    else if(b1 >= 0xF0)
    {
    b2 = getch();
    b3 = getch();
    b4 = getch();
       code = (b1 << 24) | (b2 << 16) | (b3 << 8) | b4;
    }
    else if(b1 >= 0xE0)
    {
        b2 = getch();
        b3 = getch();
        code = (b1 << 16) | (b2 << 8) | b3;
    }
    else if(b1 >= 0xC0)
    {
        b2 = getch();
        code = (b1 << 8) | b2;
    }
    return code;
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
    int curr_lines = LINES;
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
        // key = get_wch();
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
        printf("Usage : editor [filename]\n");
        exit(1);
    }
    return 0;
}
