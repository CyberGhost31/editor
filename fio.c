#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "editor.h"
#include "lines.h"

wchar_t fget_utf_c(FILE *file)
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

wchar_t *fget_utf_s(FILE *file, size_t *size, size_t *length)
{
    size_t k = 1, i = 0;
    wchar_t *str;
    str = (wchar_t*) malloc(256* sizeof(wchar_t));
    unsigned code;
    while ((code = fget_utf_c(file)) != EOF && code != '\n')
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
            temp = fget_utf_s(file, &size, &length);
            root = initln(temp, size, length);
            current = root;
            while (!feof(file))
            {
                temp = fget_utf_s(file, &size, &length);
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