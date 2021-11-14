#include <ncurses.h>
#include <stddef.h>
#include <stdlib.h>
#include <wchar.h>
#include "render.h"

enum c_tk_type_t
{
    multiline_comment = 1,
    oneline_comment = 1,
    preproc_dir = 2,
    keyword = 3,
    type = 4,
    number = 5,
    string_or_chararcter = 6,
    operator = 7,
    other = 8,
    undefined = 9
};

typedef enum c_tk_type_t c_token_type;

size_t wstrlen(wchar_t *a)
{
    size_t i = 0;
    while (a[i])
        i++;
    return i;
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

c_token_type get_c_token_type(wchar_t *a, size_t left, size_t rigth)
{
    char types[8][256] = {
        "char", "int",
        "float", "double",
        "void", "struct",
        "union", "enum"
    };
    char modifiers[4][256] = {
        "signed", "unsigned",
        "long", "short"
    };
    char keywords[][256] = {
        "auto",     "break",    "case",     "const",
        "continue", "default",  "do",       "else",
        "extern",   "for",      "goto",     "if",
        "inline",   "register", "restrict", "return",
        "sizeof",   "static",   "switch",   "typedef",
        "volatile", "while"
    };
    size_t j;
    for (size_t i = 0; i <= 7; i++)
    {
        j = 0;
        while (types[i][j] != 0 && left + j <= rigth && types[i][j] == a[left + j])
	    {
		    j++;
	    }
	    if (left + j == rigth + 1 && types[i][j] == 0)
	    {
		    return type;
	    }
    }
    for (size_t i = 0; i <= 3; i++)
    {
        j = 0;
	    while(modifiers[i][j] != 0 && left + j <= rigth && modifiers[i][j] == a[left + j])
	    {
		    j++;
	    }
	    if (left + j == rigth + 1 && modifiers[i][j] == 0)
	    {
		    return type;
	    }
    }
    for (size_t i = 0; i <= 22; i++)
    {
        j = 0;
	    while(keywords[i][j] != 0 && left + j <= rigth && keywords[i][j] == a[left + j])
	    {
		    j++;
	    }
	    if (left + j == rigth + 1 && keywords[i][j] == 0)
	    {
		    return keyword;
	    }
    }
    return undefined;
}

void output_c_line(WINDOW *win, wchar_t* a, size_t len, int start, size_t offset, c_token_type *gl)
{
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
            t = other;
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
        else if (a[i] == '\"' || a[i] == '\'')
        {
            do
            {
                i++;
            } while
            (
                !((a[i] == '\"' || a[i] == '\'') && a[i - 1] != '\\') &&
                !(a[i - 1] == '\\' && a[i - 2] == '\\') &&
                i < len
            );
            if (i < len) i++;
            t = string_or_chararcter;
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
        else if
        (
            a[i] == '&' || a[i] == '|' || a[i] == '<' || a[i] == '>' ||
            a[i] == '+' || a[i] == '-' || a[i] == '*' || a[i] == '/' ||
            a[i] == '%' || a[i] == '=' || a[i] == '~' || a[i] == '^' ||
            a[i] == '?' || a[i] == ':' || a[i] == '!'
        )
        {
            i++;
            if
            (
                a[i] == a[i - 1] &&
                (
                    a[i] == '+' || a[i] == '-' ||
                    a[i] == '<' || a[i] == '>' ||
                    a[i] == '=' || a[i] == '&' ||
                    a[i] == '|'
                ) ||
                a[i] == '=' &&
                (
                    a[i - 1] == '&' || a[i - 1] == '|' ||
                    a[i - 1] == '+' || a[i - 1] == '-' ||
                    a[i - 1] == '*' || a[i - 1] == '/' ||
                    a[i - 1] == '%' || a[i - 1] == '=' ||
                    a[i - 1] == '<' || a[i - 1] == '>' ||
                    a[i - 1] == '!'
                ) ||
                a[i] == '>' && a[i - 1] == '-'
            )
                i++;
            if (a[i] == '=' && (a[i - 1] == '<' && a[i - 1] == '>') && a[i - 1] == a[i - 2])
                i++;
            t = operator;
            if (*gl < t)
                t = *gl;
        }

        else if (a[i] == '.' || a[i] == ',' || a[i] == ';')
        {
            i++;
            t = other;
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
	if (t == undefined)
            t = get_c_token_type(a, left, rigth);
        if (start)
        {
            switch (t)
            {
            case oneline_comment:       waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(7));  break;
            case preproc_dir:           waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(2));  break;
            case number:                waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(5));  break;
            case string_or_chararcter:  waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(6));  break;
            case operator:              waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(4));  break;
            case type:                  waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(3));  break;
            case keyword:               waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(8));  break;
            default:                    waddstrfrag(win, a, left, rigth, &temp_offset, COLOR_PAIR(1));  break;
            };
        };
    };
    if (*gl == preproc_dir || *gl == oneline_comment)
        *gl = undefined;
}