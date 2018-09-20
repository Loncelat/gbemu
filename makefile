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
CFLAGS := -Wall -Wextra -O2 -s -flto -pedantic -std=c11
LFLAGS := -Wl,--no-warn-search-mismatch
LIBS   := -lmingw32 -lSDL2main -lSDL2

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst %, $(OBJDIR)/%, $(notdir $(SRC:%.c=%.o)))
DEP := $(wildcard $(INCLDIR)/*.h)

# Add Windows-specific options.
ifeq ($(OS), Windows_NT)
	LFLAGS += -Wl,-subsystem,windows
	RES := $(wildcard res/*.res)
endif

$(MAIN): $(OBJ)
	$(CC) $(CFLAGS) $(LFLAGS) $(INCL) $(LIB) $(RES) -o $(PROGDIR)/$@ $^ $(LIBS)
    
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(DEP)
	$(CC) $(CFLAGS) $(INCL) $(LIB) -c $< -o $@