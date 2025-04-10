# === Paths ===
LIBUSB_INC = /usr/include/libusb-1.0
LIBUSB_LIB = /usr/lib/x86_64-linux-gnu
NKHOME     = $(PWD)/..

# === Compiler & Flags ===
CC        = gcc
COPTS     = -fPIC -Wall
LDFLAGS   = -O2 -Wall
LIBS      = -L$(LIBUSB_LIB) -lusb-1.0 -L$(NKHOME)/lib -lNoticeLTE -lm
INCLUDES  = -I$(NKHOME)/include -I$(LIBUSB_INC)
OutOpt    = -o

# === Find all source_*.c and generate corresponding names
MAINS     := $(wildcard source_*.c)
EXES      := $(patsubst source_%.c,%.exe,$(MAINS))
OBJS      := $(patsubst source_%.c,source_%.o,$(MAINS))

INSTALLDIR = $(NKHOME)/bin

# === Targets ===
all: $(EXES)

# Rule to compile source_foo.o → foo.exe
%.exe: source_%.o
	$(CC) $^ $(LIBS) $(OutOpt)$@ $(LDFLAGS)
	@echo "$@ done"

# Compile .o from source_*.c
source_%.o: source_%.c
	$(CC) $(COPTS) $(INCLUDES) -c -o $@ $<

# Install
install: all
	install $(EXES) $(INSTALLDIR)

# Clean
clean:
	rm -f $(OBJS) $(EXES)

