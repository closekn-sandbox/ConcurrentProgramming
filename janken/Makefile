PROGS = parent child
all: $(PROGS)

parent: parent.c
	gcc -o parent parent.c

child: child.c
	gcc -o child child.c

clean:
	rm -f $(PROGS) *~
