#pragma once
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

line *initln(wchar_t *string, size_t size, size_t length);

line *addln(line *ln, wchar_t *string, size_t size, size_t length);

line *delln(line *ln);

void clrmem(line *root);