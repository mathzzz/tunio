obj=tunio.o

CFLAGS=-g -pg

all:$(obj)
	gcc -g -o tunio $^ -rdynamic

#.c.o:

clean:
	rm -f tunio *.o *.out

