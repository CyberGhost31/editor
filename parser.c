#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fio.h"

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
    wchar_t *str;
    token_type type;
    struct tk_t *next;
};

typedef struct tk_t token;

void wstrcpy(wchar_t *target, wchar_t *source)
{
    size_t i = 0;
    while (source[i])
    {
        target[i] = source[i];
        i++;
    }
}

size_t wstrlen(wchar_t *a)
{
    size_t i = 0;
    while (a[i])
        i++;
    return i;
}

void put_ws(wchar_t *a)
{
    size_t i = 0;
    while (a[i])
        putchar(a[i++]);
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

token *get_tokens(wchar_t* a)
{
    int i = 0, j;
    int len = wstrlen(a);
    printf("len = %d\n", len);
    wchar_t *buf = (wchar_t*) malloc(len * sizeof(wchar_t));
    int bufpos;
    token *root, *current, *prev;
    if (len)
        current = root = prev = NULL;
    else
    {
        return NULL;
    }
    while (i < len)
    {
        bufpos = 0;
        if (a[i] == '#')
        {
            while(is_letter(a[i]) || a[i] == '#')
                buf[bufpos++] = a[i++];

        }
        else if (a[i] <= ' ')
        {
            while (a[i] <= ' ')
                buf[bufpos++] = a[i++];
        }
        else if (is_letter(a[i]) || a[i] == '_')
        {
            while (is_letter(a[i]) || is_number(a[i]) || a[i] == '_')
                buf[bufpos++] = a[i++];
        }
        else if (a[i] == '\"')
        {
            do
            {
                buf[bufpos++] = a[i++];
            } while (!(a[i] == '\"' && a[i - 1] != '\\') && !(a[i - 1] == '\\' && a[i - 2] == '\\'));
            buf[bufpos++] = a[i++];
        }
        else if (is_number(a[i]))
        {
            while (is_number(a[i]) || a[i] ==  '.' || is_letter(a[i]))
                buf[bufpos++] = a[i++];
        }
        else if (is_square_brackets(a[i]) || is_round_brackets(a[i]) || is_curly_brackets(a[i]))
            buf[bufpos++] = a[i++];
        else if (is_angle_brackets(a[i]) || a[i] == '&' || a[i] == '|')
        {
            if ((a[i + 1] == '=' && is_angle_brackets(a[i])) || a[i] == a[i + 1])
            {
                buf[bufpos++] = a[i++];
                buf[bufpos++] = a[i++];
            }
            else
                buf[bufpos++] = a[i++];
        }
        else if (a[i] == '+' || a[i] == '/' || a[i] == '*' || a[i] == '%' || a[i] == '-' || a[i] == '=' || a[i] == '!')
        {
            if (a[i + 1] == '=' || ((a[i] == '+' || a[i] == '-') && a[i] == a[i + 1]) || (a[i] == '-' && a[i + 1] == '>'))
            {
                buf[bufpos++] = a[i++];
                buf[bufpos++] = a[i++];
            }
            else
                buf[bufpos++] = a[i++];
        }
        else if (a [i] == '~' || a[i] == '^' || a[i] == '.' || a[i] == ',' || a[i] == ';')
            buf[bufpos++] = a[i++];
        buf[bufpos] = 0;

        current = (token *)malloc(sizeof(token));
        if (root == NULL)
            root = current;
        if (prev != NULL)
            prev->next = current;
        current->str = (wchar_t *) malloc((bufpos) * sizeof(wchar_t));
        wstrcpy(current->str, buf);
        prev = current;
    }
    free(buf);
    return root;
}

void main()
{
    line *root_line = readfile("parser.c"), *curr_line;
    curr_line = root_line;
    token *curr_token, *root_token;
    while (curr_line != NULL)
    {
        root_token = get_tokens(curr_line->str);
        curr_token = root_token;
        printf("!!!\n");
        while (curr_token != NULL)
        {
            putchar('[');
            put_ws(curr_token->str);
            putchar(']');
            curr_token = curr_token->next;
        }
        putchar('\n');
        curr_line = curr_line->next;
    }
}