objs = main.o lexer.o
flags = -Wall -Wextra -Wpedantic

debut: $(objs)
	gcc -o debut $(objs)

%.o: %.c
	gcc -c $(flags) $^

clear:
	rm -f $(objs) debut
