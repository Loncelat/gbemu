
SRC    := src/*.c
INCL   := -Iinclude -ISDL2/include
SRCDIR := src
OBJDIR := obj
OBJS   := $(SRC:.c=.o)

CC     := gcc
LIB    := -LSDL2/lib
MAIN   := bin/gbemu.exe

CFLAGS := -Wall -Wextra -O2 -s -flto -Wl,--no-warn-search-mismatch -pedantic -std=c11 -Wl,-subsystem,windows
LFLAGS := -lmingw32 -lSDL2main -lSDL2

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst %, $(OBJDIR)/%, $(notdir $(SRC:%.c=%.o)))

$(MAIN): $(OBJ)
	$(CC) $(CFLAGS) $(INCL) $(LIB) -o $@ $^ $(LFLAGS)
    
.phony: debug
debug: $(OBJ)
	$(CC) $(CFLAGS) $(INCL) $(LIB) -o $@ $^ $(LFLAGS)
    
$(OBJDIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(INCL) $(LIB) -c $< -o $@