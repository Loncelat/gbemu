# Define directories.
SRCDIR  := src
OBJDIR  := obj
INCLDIR := include
PROGDIR := bin

CC     := gcc
LIB    := -LSDL2/lib
INCL   := -I$(INCLDIR) -ISDL2/include
MAIN   := gbemu

# Define flags
override CFLAGS := -Wall -Wextra $(CFLAGS)

$(MAIN):     CFLAGS += -O2 -s -flto -pedantic -std=c11 -ffast-math
disassemble: CFLAGS += -O2 -g -flto -pedantic -std=c11 -ffast-math
debug:       CFLAGS += -g -DDEBUG

LFLAGS := -Wl,--no-warn-search-mismatch
LIBS   := -lmingw32 -lSDL2main -lSDL2

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst %, $(OBJDIR)/%, $(notdir $(SRC:%.c=%.o)))
DEP := $(wildcard $(INCLDIR)/*.h)

# Add Windows-specific options.
ifeq ($(OS), Windows_NT)

$(MAIN): LFLAGS += -Wl,-subsystem,windows
RES := $(wildcard res/*.res)

endif

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEP)
	$(CC) $(CFLAGS) $(INCL) $(LIB) -c $< -o $@

$(MAIN): $(OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) $(INCL) $(LIB) $(RES) -o $(PROGDIR)/$(MAIN) $^ $(LIBS)
    
disassemble: $(OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) $(INCL) $(LIB) $(RES) -o $(PROGDIR)/$(MAIN) $^ $(LIBS)

debug: $(OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) $(INCL) $(LIB) $(RES) -o $(PROGDIR)/$(MAIN) $^ $(LIBS)