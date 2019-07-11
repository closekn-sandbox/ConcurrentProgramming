PROGS = server-lock server-nolock client
all: $(PROGS)

server-lock: server.c
	gcc -DWITH_FLOCK -o server-lock server.c

server-nolock: server.c
	gcc -o server-nolock server.c

client: client.c
	gcc -o client client.c

clean:
	rm -f $(PROGS) *~
