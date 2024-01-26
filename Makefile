# ------------------------------------------#
# File created by Attempeux on Jan 24 2024. #
# ------------------------------------------#
objs = main.o
flags = -Wall -Wextra -Wpedantic

termsp: $(objs)
	gcc -o termsp $(objs) -lncurses

%.o: %.c
	gcc -c $^ $(flags)

clear:
	rm $(objs) termsp
