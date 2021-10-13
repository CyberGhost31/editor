all:
	$(shell mkdir -p build)
	gcc main.c -lncursesw -o build/editor
