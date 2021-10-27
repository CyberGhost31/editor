#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>

enum c_tk_type_t
{
    multiline_comment,
    oneline_comment,
    preproc_dir,
    keyword,
    type,
    number,
    chararcter,
    string,
    operator,
    empty,
    delimeter,
    undefined
};

typedef enum c_tk_type_t c_token_type;

size_t wstrlen(wchar_t *a)
{
    size_t i = 0;
    while (a[i])
        i++;
    return i;
}

void wadd_token(WINDOW *win, wchar_t *a, size_t left, size_t rigth, size_t *offset, int attr)
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

int is_letter(wchar_t a)
{
    return (a >= 'A' && a <= 'Z') || (a >= 'a' && a <= 'z');
}

int is_number(wchar_t a)
{
    return a >= '0' && a <= '9';
}

int is_square_brackets(wchar_t a)
{
    return a == '[' || a == ']';
}

int is_angle_brackets(wchar_t a)
{
    return a == '<' || a == '>';
}

int is_round_brackets(wchar_t a)
{
    return a == '(' || a == ')';
}

int is_curly_brackets(wchar_t a)
{
    return a == '{' || a == '}';
}

void output_c_line(WINDOW *win, wchar_t* a, size_t len, int start, size_t offset, c_token_type *gl)
{
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, 10, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, 8, COLOR_BLACK);
    size_t i = 0, j;
    size_t left, rigth;
    c_token_type t;
    size_t temp_offset = offset;
    while (i < len)
    {
        left = i;
        t = undefined;
        if (a[i] == '/' && a[i + 1] == '*')
        {
            i += 2;
            *gl = t = multiline_comment;
        }
        else if (a[i] == '*' && a[i + 1] == '/')
        {
            i += 2;
            t = multiline_comment;
            *gl = undefined;
        }
        else if (a[i] == '/' && a[i + 1] == '/')
        {
            i += 2;
            t = oneline_comment;
            if (*gl < t)
                t = *gl;
            else
                *gl = t;
        }
        else if (a[i] == '#')
        {
            while(is_letter(a[i]) || a[i] == '#')
                i++;
            t = preproc_dir;
            if (*gl < t)
                t = *gl;
            else
                *gl = t;
        }
        else if (a[i] <= ' ')
        {
            while (a[i] <= ' ' && a[i] > '\0')
                i++;
            if (*gl < t)
                t = *gl;
        }
        else if (is_letter(a[i]) || a[i] == '_')
        {
            while (is_letter(a[i]) || is_number(a[i]) || a[i] == '_')
                i++;
            if (*gl < t)
                t = *gl;
        }
        else if (a[i] == '\"')
        {
            do
            {
                i++;
            } while (!(a[i] == '\"' && a[i - 1] != '\\') && !(a[i - 1] == '\\' && a[i - 2] == '\\'));
            i++;
            t = string;
            if (*gl < t)
                t = *gl;
        }
        else if (a[i] == '\'')
        {
            do
            {
                i++;
            } while (!(a[i] == '\'' && a[i - 1] != '\\') && !(a[i - 1] == '\\' && a[i - 2] == '\\'));
            i++;
            t = chararcter;
            if (*gl < t)
                t = *gl;
        }
        else if (is_number(a[i]))
        {
            while (is_number(a[i]) || a[i] ==  '.' || is_letter(a[i]))
                i++;
            t = number;
            if (*gl < t)
                t = *gl;
        }
        else if (is_square_brackets(a[i]) || is_round_brackets(a[i]) || is_curly_brackets(a[i]))
        {
            i++;
            if (*gl < t)
                t = *gl;
        }
        else if (is_angle_brackets(a[i]) || a[i] == '&' || a[i] == '|')
        {
            if ((a[i + 1] == '=' && is_angle_brackets(a[i])) || a[i] == a[i + 1])
                i += 2;
            else
                i++;
            t = operator;
            if (*gl < t)
                t = *gl;
        }
        else if (a[i] == '+' || a[i] == '/' || a[i] == '*' || a[i] == '%' || a[i] == '-' || a[i] == '=' || a[i] == '!')
        {
            if (a[i + 1] == '=' || ((a[i] == '+' || a[i] == '-') && a[i] == a[i + 1]) || (a[i] == '-' && a[i + 1] == '>'))
                i += 2;
            else
                i++;
            t = operator;
            if (*gl < t)
                t = *gl;
        }
        else if (a[i] == '~' || a[i] == '^' || a[i] == '?' || a[i] == ':')
        {
            i++;
            t = operator;
            if (*gl < t)
                t = *gl;
        }

        else if (a[i] == '.' || a[i] == ',' || a[i] == ';')
        {
            i++;
            t = delimeter;
            if (*gl < t)
                t = *gl;
        }
        else
        {
            i++;
            if (*gl < t)
                t = *gl;
        }
        rigth = i - 1;
        if (start)
        {
            if (t == multiline_comment || t == oneline_comment)
                wadd_token(win, a, left, rigth, &temp_offset, COLOR_PAIR(7));
            else if (t == preproc_dir)
                wadd_token(win, a, left, rigth, &temp_offset, COLOR_PAIR(2));
            else if (t == number)
                wadd_token(win, a, left, rigth, &temp_offset, COLOR_PAIR(5));
            else if (t == string || t == chararcter)
                wadd_token(win, a, left, rigth, &temp_offset, COLOR_PAIR(6));
            else if (t == operator)
                wadd_token(win, a, left, rigth, &temp_offset, COLOR_PAIR(4));
            else
                wadd_token(win, a, left, rigth, &temp_offset, COLOR_PAIR(1));
        }
    }
    if (*gl == preproc_dir || *gl == oneline_comment)
        *gl = undefined;
}