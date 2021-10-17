#pragma once
#ifndef H_FIO
#define H_FIO
#include "editor.h"
#endif

line *readfile(char fname[]);

void save(editor_state *ed);