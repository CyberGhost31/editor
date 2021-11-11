all:
	$(shell mkdir -p build)
	cc main.c lines.c editor.c render.c fio.c logic.c highligth_c.c colors.c -lncursesw -o build/editor
