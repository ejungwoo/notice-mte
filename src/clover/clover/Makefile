NKHOME = /home/cho/asgard_daq/notice

LIBUSB_INC = /usr/include/libusb-1.0
LIBUSB_LIB = /usr/lib64
NOTICE_LIB = $(NKHOME)/lib
NOTICE_INC = $(NKHOME)/include
INSTALL_LIB = $(NKHOME)/lib
INSTALL_INC = $(NKHOME)/include

MODULE = NoticeCLOVER

OutPutOpt = -o #
CFLAGS = -I. -I$(LIBUSB_INC) -Wall -Wundef -Wunused -Wstrict-prototypes -Werror-implicit-function-declaration -Wno-pointer-sign -Wshadow -O2 -fPIC -m64

#SOFLAGS = -shared -Wall,-soname -nostartfiles -s -m64
SOFLAGS = -shared -Wall
LDFLAGS = $(SOFLAGS)
LDLIBS = -L$(LIBUSB_LIB) -lusb-1.0
CC = gcc
LD = gcc

NKUSBS = $(MODULE).c
NKUSBH = $(MODULE).h 
NKUSBO = $(MODULE).o
NKUSBSO = lib$(MODULE).so

PROGRAMS = $(NKUSBSO)

all: $(PROGRAMS)
clean:
	@rm -rf $(NKUSBO) $(PROGRAMS)

$(NKUSBSO): $(NKUSBO)
	$(LD) $^ $(LDLIBS) $(OutPutOpt) $@ $(EXPLLINKLIBS) $(LDFLAGS)

install: $(NKUSBSO) $(NKUSBH)
	#/bin/rm -f $(INSTALL_LIB)/$<
	#/bin/rm -f $(INSTALL_INC)/$(NKUSBH)
	install $< $(INSTALL_LIB)
	install $(NKUSBH) $(INSTALL_INC)

#dependicies
$(NKUSBO):$(NKUSBH)
