.POSIX:

CC=cc
INCLUDEDIR=include
CFLAGS=$(shell pkg-config --cflags json-c) -I$(INCLUDEDIR)
LIBS=$(shell pkg-config --libs json-c)
SOURCES=src/*.c
BUILDIR=build/src
NAME=among-foss

all: $(SOURCES)
	mkdir -p $(BUILDIR) &> /dev/null
	$(CC) $(CFLAGS) -o $(BUILDIR)/$(NAME) $(SOURCES) $(LIBS)

install: all
	install $(BUILDIR)/$(NAME) $(DESTDIR)/$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)/$(PREFIX)/bin/$(NAME)

clean:
	rm -rf build
