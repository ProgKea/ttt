PROJECT=ttt

$(PROJECT): main.c
	cc main.c -Wall -pedantic -Wshadow -Werror -o $(PROJECT)

run:
	make && ./$(PROJECT)
