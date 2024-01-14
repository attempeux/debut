objs = main.o lexer.o
flags = -Wall -Wextra -Wpedantic
SP = debut
LT = lexer_test

$(SP): $(objs)
	gcc -o $(SP) $(objs) -lncurses

$(LT):
	gcc -D LEXER_TEST -o $(LT) *c -lncurses
	mv $(LT) unit-testing/

%.o: %.c
	gcc -c $(flags) $^

clear:
	rm -f $(objs) debut $(SP) $(LT)
