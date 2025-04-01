CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -lreadline

shield: main.o utils.o
	$(CC) -o shield main.o utils.o $(LDFLAGS)

main.o: main.c utils.h
	$(CC) $(CFLAGS) -c main.c

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c

clean:
	rm -f *.o shield

