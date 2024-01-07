objs = main.o lexer.o
flags = -Wall -Wextra -Wpedantic -Wno-switch

debut: $(objs)
	gcc -o debut $(objs) -lncurses

%.o: %.c
	gcc -c $(flags) $^

clear:
	rm -f $(objs) debut
