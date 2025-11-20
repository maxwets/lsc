NAME     := lsc

CC64     := x86_64-w64-mingw32-gcc -m64
CC86     := i686-w64-mingw32-gcc -m32

CFLAGS   += -Iinclude
CFLAGS   += -std=c23
CFLAGS   += -fPIC
CFLAGS   += -s
CFLAGS   += -Os
CFLAGS   += -Wall
CFLAGS   += -Werror
CFLAGS   += -Wno-array-bounds

CFLAGS   += -fcf-protection=none
CFLAGS   += -fmerge-all-constants
CFLAGS   += -fno-asynchronous-unwind-tables
CFLAGS   += -fno-delete-null-pointer-checks
CFLAGS   += -fno-exceptions
CFLAGS   += -fno-ident
CFLAGS   += -fno-jump-tables
CFLAGS   += -fno-stack-check
CFLAGS   += -fno-unwind-tables
CFLAGS   += -fomit-frame-pointer
CFLAGS   += -fpack-struct=8
CFLAGS   += -mno-stack-arg-probe

CFLAGS   += -fdata-sections
CFLAGS   += -ffunction-sections
CFLAGS   += -falign-functions=1:0:0
CFLAGS   += -falign-jumps=1:0:0
CFLAGS   += -falign-loops=1:0:0
CFLAGS   += -falign-labels=1:0:0

LDFLAGS  += -Wl,--gc-sections
LDFLAGS  += -Wl,--no-seh
LDFLAGS  += -Wl,--disable-auto-import
LDFLAGS  += -Wl,--no-insert-timestamp
LDFLAGS  += -Wl,--enable-stdcall-fixup
LDFLAGS  += -Wl,--disable-dynamicbase
LDFLAGS  += -Wl,--strip-all

LDFLAGS  += -nostartfiles
LDFLAGS  += -nodefaultlibs
LDFLAGS  += -nostdlib

LDFLAGS  += -lgcc
LDFLAGS  += -lmsvcrt
LDFLAGS  += -lkernel32

EP64     += -e mainCRTStartup
EP86     += -e _mainCRTStartup


SRCDIR   := src
OBJDIR   := obj
BINDIR   := bin
SRC      := $(wildcard $(SRCDIR)/*.c)

OBJ64    := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.x64.o,$(SRC)) 
OBJ86    := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.x86.o,$(SRC))

EXE64    := $(BINDIR)/$(NAME)64.exe
EXE86    := $(BINDIR)/$(NAME)86.exe


all: x64 x86

x64: $(EXE64)

x86: $(EXE86)

$(EXE64): $(OBJ64) | $(BINDIR)
	$(CC64) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(EP64)

$(EXE86): $(OBJ86) | $(BINDIR)
	$(CC86) $^ -o $@ $(CFLAGS) $(LDFLAGS) $(EP86)

$(OBJDIR)/%.x64.o: $(SRCDIR)/%.c | $(OBJDIR)
	@ mkdir -p $(dir $@)
	$(CC64) -c $< -o $@ $(CFLAGS)

$(OBJDIR)/%.x86.o: $(SRCDIR)/%.c | $(OBJDIR)
	@ mkdir -p $(dir $@)
	$(CC86) -c $< -o $@ $(CFLAGS)

$(OBJDIR) $(BINDIR):
	@ mkdir -p $@

clean:
	@ rm -rf $(OBJDIR) $(BIN) $(EXEC64) $(EXEC86)

.PHONY: all x64 x86 clean
