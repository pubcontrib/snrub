CFLAGS = -ansi -pedantic -Wall
CC = gcc
RM = rm
CP = cp
SH = sh
MKDIR = mkdir
EXE = snrub
SRC = src
OBJ = obj
BIN = bin
TEST = test
PREFIX = /usr/local
EXEC_PREFIX = $(PREFIX)
BINDIR = $(EXEC_PREFIX)/bin
OBJS = $(OBJ)/main.o \
       $(OBJ)/execute.o \
	   $(OBJ)/parse.o \
	   $(OBJ)/lex.o \
	   $(OBJ)/common.o

.PHONY: all clean check install uninstall

all: $(BIN)/$(EXE)

clean:
	$(RM) -f $(BIN)/$(EXE) $(OBJS)

check: $(BIN)/$(EXE)
	$(SH) $(TEST)/run.sh $(BIN)/$(EXE)

install: $(BIN)/$(EXE)
	$(CP) $(BIN)/$(EXE) $(BINDIR)/$(EXE)

uninstall:
	$(RM) -f $(BINDIR)/$(EXE)

$(BIN)/$(EXE): $(OBJS)
	$(MKDIR) -p $(BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ)/%.o: $(SRC)/%.c
	$(MKDIR) -p $(OBJ)
	$(CC) $(CFLAGS) -c $< -o $@
