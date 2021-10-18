#pragma once
#include "lines.h"

struct ed_st_t
{
    char *filename;
    int virt_y, virt_x;
    int real_y, real_x, saved_real_x;
    int offset_x, saved_offset_x;
    line *root, *top, *current;
    int rerender_flag, exit_flag, edit_flag;
};

typedef struct ed_st_t editor_state;

void editor(char *fname);