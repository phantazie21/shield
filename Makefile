CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lreadline

shield: main.o utils.o
	$(CC) -o shield main.o utils.o builtin.c $(LDFLAGS)

main.o: main.c utils.h
	$(CC) $(CFLAGS) -c main.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

builtin.o: builtin.c builtin.h
	$(CC) $(CFLAGS) -c builtin.c

clean:
	rm -f *.o shield

