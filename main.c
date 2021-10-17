#include <locale.h>
#include "editor.h"

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    editor(argv[1]);
    return 0;
}