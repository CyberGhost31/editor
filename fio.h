#pragma once
#ifndef H_FIO
#define H_FIO
#include "editor.h"
#endif

line *readfile(char fname[]);

int get_file_type(char *filename);

void enter_name(editor_state *ed);

void save(editor_state *ed);