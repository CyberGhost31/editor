#pragma once
#include "lines.h"

struct ed_st_t
{
    char *filename;
    size_t virt_y, virt_x;
    size_t real_y, real_x, saved_real_x;
    size_t offset_x;
    line *root, *top, *current;
    short rerender_flag, exit_flag, edit_flag;
};

typedef struct ed_st_t editor_state;

void editor(char *fname);