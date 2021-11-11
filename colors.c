#include <ncurses.h>

void set_colors()
{
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, 10, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_BLUE, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_CYAN, COLOR_BLACK);
    init_pair(7, 8, COLOR_BLACK);
    init_pair(8, COLOR_GREEN, COLOR_BLACK);
}