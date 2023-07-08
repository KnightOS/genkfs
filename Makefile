CFLAGS=-Wall -Wextra -pedantic -std=c99 -O2 -g -D_POSIX_C_SOURCE=200809 -D_BSD_SOURCE -D_DEFAULT_SOURCE -D_DARWIN_C_SOURCE

all: bin/genkfs bin/genkfs.1

bin/genkfs:main.o
	mkdir -p bin/
	$(CC) $(CFLAGS) $^ -o $@

bin/genkfs.1:genkfs.1.scdoc
	scdoc < genkfs.1.scdoc > bin/genkfs.1

DESTDIR=/usr/local
BINDIR=$(DESTDIR)/bin/
MANDIR=$(DESTDIR)/share/man/

install: bin/genkfs
	mkdir -p $(BINDIR)
	cp bin/genkfs $(BINDIR)

install_man: bin/genkfs.1
	mkdir -p $(MANDIR)/man1/
	cp bin/genkfs.1 $(MANDIR)/man1/

uninstall:
	$(RM) $(BINDIR)/genkfs $(MANDIR)/man1/genkfs.1

clean:
	$(RM) bin *.o -rv
