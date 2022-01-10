all:
	$(shell mkdir -p build)
	cc main.c lines.c editor.c render.c fio.c process.c highlight_c.c colors.c -lncursesw -o build/editor
