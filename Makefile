.POSIX:
.SUFFIXES:
.PHONY: all clean check install uninstall

CFLAGS = -ansi -pedantic -Wall
LDFLAGS =
CC = cc
RM = rm
CP = cp
SH = sh
MKDIR = mkdir
PREFIX = /usr/local

all: bin/snrub

clean:
	$(RM) -f bin/snrub obj/main.o obj/cli.o obj/execute.o obj/parse.o obj/lex.o obj/value.o obj/map.o obj/list.o obj/string.o obj/common.o

check: bin/snrub
	$(SH) test/run.sh bin/snrub

install: bin/snrub
	$(CP) bin/snrub $(DESTDIR)$(PREFIX)/bin/snrub

uninstall:
	$(RM) -f $(DESTDIR)$(PREFIX)/bin/snrub

bin/snrub: bin obj obj/main.o obj/cli.o obj/execute.o obj/parse.o obj/lex.o obj/value.o obj/map.o obj/list.o obj/string.o obj/common.o
	$(CC) $(LDFLAGS) -o bin/snrub obj/main.o obj/cli.o obj/execute.o obj/parse.o obj/lex.o obj/value.o obj/map.o obj/list.o obj/string.o obj/common.o

obj/main.o: src/main.c obj/cli.o obj/execute.o obj/value.o obj/map.o obj/string.o obj/common.o
	$(CC) $(CFLAGS) -c src/main.c -o obj/main.o

obj/cli.o: src/cli.h src/cli.c obj/string.o obj/common.o
	$(CC) $(CFLAGS) -c src/cli.c -o obj/cli.o

obj/execute.o: src/execute.h src/execute.c obj/parse.o obj/lex.o obj/value.o obj/map.o obj/list.o obj/string.o obj/common.o
	$(CC) $(CFLAGS) -c src/execute.c -o obj/execute.o

obj/parse.o: src/parse.h src/parse.c obj/lex.o obj/value.o obj/list.o obj/string.o obj/common.o
	$(CC) $(CFLAGS) -c src/parse.c -o obj/parse.o

obj/lex.o: src/lex.h src/lex.c obj/string.o obj/common.o
	$(CC) $(CFLAGS) -c src/lex.c -o obj/lex.o

obj/value.o: src/value.h src/value.c obj/map.o obj/string.o obj/common.o
	$(CC) $(CFLAGS) -c src/value.c -o obj/value.o

obj/map.o: src/map.h src/map.c obj/string.o obj/common.o
	$(CC) $(CFLAGS) -c src/map.c -o obj/map.o

obj/list.o: src/list.h src/list.c obj/common.o
	$(CC) $(CFLAGS) -c src/list.c -o obj/list.o

obj/string.o: src/string.h src/string.c obj/common.o
	$(CC) $(CFLAGS) -c src/string.c -o obj/string.o

obj/common.o: src/common.h src/common.c
	$(CC) $(CFLAGS) -c src/common.c -o obj/common.o

bin:
	$(MKDIR) bin

obj:
	$(MKDIR) obj
