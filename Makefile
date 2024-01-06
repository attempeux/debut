objs = main.o
flags = -Wall -Wextra -Wpedantic

debut: $(objs)
	gcc -o debut $(objs) -lncurses

%.o: %.c
	gcc -c $(flags) $^

clear:
	rm -f $(objs) debut
