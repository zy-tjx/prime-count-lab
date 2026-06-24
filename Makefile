CC  = gcc
CFLAGS = -O2 -Wall

.PHONY: all clean

all: single multi

single: single.c
	$(CC) $(CFLAGS) -o single single.c -lm

multi: multi.c
	$(CC) $(CFLAGS) -o multi multi.c -lpthread -lm

clean:
	rm -f single multi
