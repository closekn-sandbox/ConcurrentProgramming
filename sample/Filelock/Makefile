PROGS = flocktest-lock flocktest-nolock
all: $(PROGS)

flocktest-lock: flocktest.c
	gcc -DWITH_FLOCK -o flocktest-lock flocktest.c

flocktest-nolock: flocktest.c
	gcc -o flocktest-nolock flocktest.c

clean:
	rm -f $(PROGS) *~
