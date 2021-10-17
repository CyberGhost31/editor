#include <stdio.h>
#include <stdlib.h>
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