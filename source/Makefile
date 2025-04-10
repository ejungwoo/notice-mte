LIBUSB_INC = /usr/include/libusb-1.0
LIBUSB_LIB = /usr/lib/x86_64-linux-gnu
NKHOME := $(shell realpath $(PWD)/..)

INSTALL_LIB = $(NKHOME)/lib
INSTALL_INC = $(NKHOME)/include

MODULE  = NoticeLTE
OutPutOpt = -o #
CFLAGS  = -I. -I$(LIBUSB_INC) -Wall -Wundef -Wunused -Wstrict-prototypes -Werror-implicit-function-declaration -Wno-pointer-sign -Wshadow -O2 -fPIC -m64

SOFLAGS = -shared -Wall
LDFLAGS = $(SOFLAGS)
LDLIBS  = -L$(LIBUSB_LIB) -lusb-1.0
CC      = gcc
LD      = gcc

INCLUDE = $(MODULE).h
OBJECT  = $(MODULE).o
LIBRARY = lib$(MODULE).so

PROGRAMS = $(LIBRARY)

# === Default target includes generate-env ===
all: generate-env $(PROGRAMS)

clean:
	@rm -rf $(OBJECT) $(PROGRAMS) notice-mte.sh

# === Shared library build rule ===
$(LIBRARY): $(OBJECT)
	$(LD) $(LDFLAGS) $^ $(LDLIBS) $(OutPutOpt) $@ $(EXPLLINKLIBS)

# === Installation ===
install: $(LIBRARY) $(INCLUDE)
	install $< $(INSTALL_LIB)
	install $(INCLUDE) $(INSTALL_INC)

# === Auto-generate environment script ===
generate-env:
	echo 'export LD_LIBRARY_PATH=$(NKHOME)/lib:$$LD_LIBRARY_PATH' > $(NKHOME)/notice-mte.sh
	chmod +x $(NKHOME)/notice-mte.sh

# === Dependencies ===
$(OBJECT): $(INCLUDE)
