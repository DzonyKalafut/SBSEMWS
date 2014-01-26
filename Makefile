CC=gcc
CFLAGS=-w -g
webserver: webserver.c -lpthread 
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o core
