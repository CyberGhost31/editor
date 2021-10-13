all:
        $(shell if ! [ -d build ]; then mkdir build; fi;)
        gcc main.c -lncursesw -o build/editor
