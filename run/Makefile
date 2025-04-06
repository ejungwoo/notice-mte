# === Paths ===
LIBUSB_INC = /usr/include/libusb-1.0
LIBUSB_LIB = /usr/lib/x86_64-linux-gnu
NKHOME     = $(PWD)/../

# === Compiler & Flags ===
CC        = gcc
COPTS     = -fPIC -Wall
LDFLAGS   = -O2 -Wall
LIBS      = -L$(LIBUSB_LIB) -lusb-1.0 -L$(NKHOME)/lib -lNoticeLTE -lm
INCLUDES  = -I$(NKHOME)/include -I$(LIBUSB_INC)
OutPutOpt = -o

# === Targets ===
TARGETS   = set_LTE.exe run_CLOVER.exe
OBJS      = set_LTE.o run_CLOVER.o

INSTALLDIR = $(NKHOME)/bin

# === Rules ===
all: $(TARGETS)

%.exe: %.o
	$(CC) $^ $(LIBS) $(OutPutOpt)$@ $(LDFLAGS)
	@echo "$@ done"

%.o: %.c
	$(CC) $(COPTS) $(INCLUDES) -c -o $@ $<

clean:
	/bin/rm -f $(OBJS) $(TARGETS)

install: all
	install set_LTE.exe $(INSTALLDIR)
	install run_CLOVER.exe $(INSTALLDIR)
