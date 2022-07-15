PROJECT=ttt

$(PROJECT): main.c
	cc main.c -pedantic -Wshadow -Werror -o $(PROJECT)

run:
	make && ./$(PROJECT)
