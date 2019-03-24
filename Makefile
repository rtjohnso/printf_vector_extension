
CFLAGS=-Wall -Wno-format -g -I.
LDFLAGS=-lm

example: example.o printf_vector.o

clean:
	rm example.o printf_vector.o
