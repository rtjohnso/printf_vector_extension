
CFLAGS=-Wall -Wno-format -g -I.

example: example.o printf_vector.o

clean:
	rm example.o printf_vector.o
