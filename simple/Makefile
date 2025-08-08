.PHONY: all clean
all: server client
CFLAGS = $(shell pkg-config --cflags glib-2.0 gio-2.0)
LDFLAGS = $(shell pkg-config --libs glib-2.0 gio-2.0)

server: server.c
	gcc $(CFLAGS) server.c -o server   $(LDFLAGS)

client: client.c
	gcc $(CFLAGS) client.c -o client  $(LDFLAGS)

clean:
	rm -f server client
