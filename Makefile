objs = main.o lexer.o parse.o
flags = -Wall -Wextra -Wpedantic

debut: $(objs)
	gcc -o debut $(objs) -lncurses -lm

%.o: %.c
	gcc -c $(flags) $^

clear:
	rm -f $(objs) debut
