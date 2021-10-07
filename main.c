#include <locale.h>
#include <stdio.h>
#include <stddef.h>
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

enum tk_type_t
{
    keyword,
    number,
    chararcter,
    string,
    preproc_dir,
    operators,
    type

};

typedef enum tk_type_t token_type;

struct tk_t
{
    char *str;
    token_type type;
    struct token *next;
};

typedef struct tk_t token;

line *init(char *string, unsigned size, unsigned length)
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
        root = init(temp, 256, 0);
    }
    else
    {
        fseek(file, 0, SEEK_END);
        pos = ftell(file);
        rewind(file);
        if (pos)
        {
            temp = myfgets(file, &size, &length);
            root = init(temp, size, length);
            current = root;
            while (NULL != (temp = myfgets(file, &size, &length)))
                current = addln(current, temp, size, length);
        }
        else
        {
            temp = (char *)malloc(256 * sizeof(char));
            temp[0] = 0;
            root = init(temp, 256, 0);
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

void print(line *a, int offset)
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
                printw("%c", a->str[i]);
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
                    printw(" ");
                    c++;
                }
            }
        }
        if (c == COLS - 1)
            break;
    }
}

void render_text(WINDOW *win, line *top_line, int offsety, int offsetx)
{
    curs_set(0);
    box(win, 0, 0);
    for (int i = 1; i < LINES - 2; i++)
    {
        move(i, 1);
        if (top_line != NULL)
        {
            print(top_line, offsetx);
            // printw("%s", top_line->str);
            top_line = top_line->next;
        }
        else
            break;
    }
    wrefresh(win);
}

void get_virt_x(line *current, int real_x, int *offset, int *virt_x)
{
    int tabs = 0;
    int num_of_chars;
    for (int i = 0; i < real_x - 1; i++)
        if (current->str[i] == 9) tabs++;
    num_of_chars =  real_x - tabs + tabs * 4;
    if (num_of_chars < *offset)
    {

        *virt_x = num_of_chars % (COLS - 1);
        *offset = num_of_chars = num_of_chars - *virt_x;
    }
    else
        *virt_x = num_of_chars - *offset;
}

void render_interface(int real_y, int real_x, int virt_y, int virt_x, int offset)
{
    curs_set(0);
    move(LINES - 1, 1);
    insertln();
    attron(A_REVERSE);
    printw("real: %d/%d; virt: %d/%d; offset = %d; LINES = %d; COLS = %d", real_y, real_x, virt_y, virt_x, offset, LINES, COLS);
    attroff(A_REVERSE);
    refresh();
}

void editor(char *fname)
{
    line *root, *top, *current;
    root = readfile(fname);
    top = root;
    current = root;
    if (!initscr())
    {
        printf("Ошибка инициализации ncurses.\n");
        exit(1);
    }
    noecho();
    int key = 0;
    int virt_y = 1, virt_x = 1, real_y = 1, real_x = 1, saved_real_x = 1;
    int maxx = COLS;
    int maxy = LINES;
    keypad(stdscr, TRUE);
    int width = COLS;
    int height = LINES;
    int offsety = 0, offsetx = 0, saved_offsetx = 0;
    int tabs;
    WINDOW *win = newwin(LINES - 1, COLS, 0, 0);
    while (1)
    {
        render_interface(real_y, real_x, virt_y, virt_x, offsetx);
        render_text(win, top, offsety, offsetx);
        move(virt_y, virt_x);
        curs_set(1);
        key = getch();
        if ((key == KEY_UP) && ((top->prev != NULL) || (current->prev != NULL)))
        {
            if ((top->prev != NULL) || (current->prev != NULL))
            {
                real_y--;
                current = current->prev;

                if (current->length < saved_real_x)
                    real_x = current->length + 1;
                else
                    real_x = saved_real_x;
                if (offsetx == 0)
                    offsetx = saved_offsetx;
                get_virt_x(current, real_x, &offsetx, &virt_x);
                if (1 < virt_y)
                    move(--virt_y, virt_x);
                else
                    top = top->prev;
            }
        }
        else if ((key == KEY_DOWN) && (current->next != NULL))
        {
            if (current->next != NULL)
            {
                real_y++;
                current = current->next;
                if (current->length < saved_real_x)
                    real_x = current->length + 1;
                else
                    real_x = saved_real_x;
                if (offsetx == 0)
                    offsetx = saved_offsetx;
                get_virt_x(current, real_x, &offsetx, &virt_x);
                if (virt_y < maxy - 3)
                    move(++virt_y, virt_x);
                else
                    top = top->next;
            }
        }
        else if ((key == KEY_LEFT) && (1 < real_x))
        {
            if (1 < real_x)
            {
                real_x--;
                saved_real_x = real_x;
                if (virt_x == 1)
                {
                    offsetx--;
                    saved_offsetx = offsetx;
                }
                get_virt_x(current, real_x, &offsetx, &virt_x);
                move(virt_y, virt_x);
            }
        }
        else if ((key == KEY_RIGHT) && (real_x <= current->length))
        {
            if (real_x <= current->length)
            {
                real_x++;
                saved_real_x = real_x;
                if (virt_x == COLS - 1)
                {
                    offsetx++;
                    saved_offsetx = offsetx;
                }
                get_virt_x(current, real_x, &offsetx, &virt_x);
                move(virt_y, virt_x);
            }
        }
        else if (key == 27)
            break;
    }
    endwin();

    clrmem(root);
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
