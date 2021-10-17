all:
	$(shell mkdir -p build)
	cc main.c lines.c editor.c render.c fio.c logic.c -lncursesw -o build/editor