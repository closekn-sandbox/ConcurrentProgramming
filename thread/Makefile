PROGS = determinant prod2cons
all: $(PROGS)

1: determinant.c
	gcc -o determinant determinant.c -lpthread

2: prod2cons.c
	gcc -o prod2cons prod2cons.c -lpthread

clean:
	rm -f $(PROGS) *~