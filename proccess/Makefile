EXEC = prod2cons prod2cons_not_debug

all: $(EXEC)
prod2cons: prod2cons.c
	gcc -o crod2cons prod2cons.c
prod2cons_not_debug: prod2cons.c
	gcc -DWITH_NOT_DEBUG -o prod2cons_not_debug prod2cons.c

clean:
	rm -f $(EXEC) *.o *.s *~