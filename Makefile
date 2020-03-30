CFLAGS = -ansi -pedantic -Wall
CC = gcc
RM = rm
CP = cp
EXE = snrub
SRC = src
OBJ = obj
BIN = bin
PREFIX = /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin
OBJS = $(OBJ)/main.o \
       $(OBJ)/execute.o \
	   $(OBJ)/parse.o \
	   $(OBJ)/lex.o \
	   $(OBJ)/common.o

.PHONY: all clean install uninstall

all: $(BIN)/$(EXE)

clean:
	$(RM) -f $(BIN)/$(EXE) $(OBJS)

install: $(BIN)/$(EXE)
	$(CP) $(BIN)/$(EXE) $(BINDIR)/$(EXE)

uninstall:
	$(RM) -f $(BINDIR)/$(EXE)

$(BIN)/$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
