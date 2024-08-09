#include "NoticeCLOVER.h"

enum EManipInterface {kInterfaceClaim, kInterfaceRelease};

struct dev_open {
   libusb_device_handle *devh;
   uint16_t vendor_id;
   uint16_t product_id;
   int serial_id;
   struct dev_open *next;
} *ldev_open = 0;

// internal functions *********************************************************************************
static int is_device_open(libusb_device_handle *devh);
static unsigned char get_serial_id(libusb_device_handle *devh);
static void add_device(struct dev_open **list, libusb_device_handle *tobeadded, int sid);
static int handle_interface_id(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type);
static void remove_device_id(struct dev_open **list, int sid);
libusb_device_handle* nkusb_get_device_handle(int sid);
int USB3Reset(int sid);
int USB3Read_i(int sid, int mid, unsigned long count, unsigned long addr, unsigned char *data);
int USB3Write(int sid, int mid, unsigned long addr, unsigned long data);
unsigned long USB3Read(int sid, int mid, unsigned long addr);
int USB3Read_Block(int sid, unsigned long count, unsigned long addr, unsigned char *data);
void CLOVERreset_REFCLK(int sid, int mid); 
void CLOVERreset_ADC(int sid, int mid); 
void CLOVERsetup_ADC(int sid, int mid, int addr, int data);
unsigned long CLOVERread_ADC_ALIGN(int sid, int mid, int ch); 
void CLOVERwrite_ADC_DLY(int sid, int mid, int ch, unsigned long data); 
void CLOVERwrite_ADC_BITSLIP(int sid, int mid, int ch);
void CLOVERwrite_ADC_IMUX(int sid, int mid, int ch, unsigned long data);
void CLOVERstart_DRAM(int sid, int mid);
unsigned long CLOVERread_DRAM_READY(int sid, int mid);
void CLOVERwrite_DRAM_TEST(int sid, int mid, unsigned long data); 
unsigned long CLOVERread_DRAM_ALIGN(int sid, int mid, int ch); 
void CLOVERwrite_DRAM_DLY(int sid, int mid, int ch, unsigned long data);
void CLOVERwrite_DRAM_BITSLIP(int sid, int mid, int ch);
void CLOVERwrite_DAC8(int sid, int mid, unsigned long data);
void CLOVERwrite_DAC32(int sid, int mid, int ch, unsigned long data);
void CLOVERwrite_DAC2(int sid, int mid, unsigned long data);
void CLOVERwrite_RISE_TIME(int sid, int mid, int ch, unsigned long data); 
void CLOVERwrite_FLAT_TOP(int sid, int mid, int ch, unsigned long data); 
void CLOVERwrite_DECAY_TIME(int sid, int mid, int ch, unsigned long data); 
void CLOVERwrite_TRAPEZOIDAL_SCALE(int sid, int mid, int ch, unsigned long data); 
unsigned long CLOVERread_TRAPEZOIDAL_SCALE(int sid, int mid, int ch); 
void CLOVERwrite_RC_COEF(int sid, int mid, unsigned long data);
void CLOVERwrite_CR_GAIN(int sid, int mid, unsigned long data);
void CLOVERwrite_CR2_GAIN(int sid, int mid, unsigned long data);

static int is_device_open(libusb_device_handle *devh)
{
  struct dev_open *curr = ldev_open;
  libusb_device *dev, *dev_curr;
  int bus, bus_curr, addr, addr_curr;

  while (curr) {
    dev_curr = libusb_get_device(curr->devh);
    bus_curr = libusb_get_bus_number(dev_curr);
    addr_curr = libusb_get_device_address(dev_curr);

    dev = libusb_get_device(devh);
    bus = libusb_get_bus_number(dev);
    addr = libusb_get_device_address(dev);

    if (bus == bus_curr && addr == addr_curr) return 1;
    curr = curr->next;
  }

  return 0;
}

static unsigned char get_serial_id(libusb_device_handle *devh)
{
  int ret;
  if (!devh) {
    return 0;
  }
  unsigned char data[1];
  ret = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, 0xD2, 0, 0, data, 1, 1000);

  if (ret < 0) {
    fprintf(stdout, "Warning: get_serial_id: Could not get serial id.\n");
    return 0;
  }

  return data[0];
}

static void add_device(struct dev_open **list, libusb_device_handle *tobeadded, int sid)
{
  struct dev_open *curr;

  curr = (struct dev_open *)malloc(sizeof(struct dev_open));
  curr->devh = tobeadded;
  curr->vendor_id = CLOVER_VID;
  curr->product_id = CLOVER_PID;
  curr->serial_id = sid;
  curr->next  = *list;
  *list = curr;
}

static int handle_interface_id(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type)
{
  int ret = 0;
  if (!*list) {
    ret = -1;
    return ret;
  }

  struct dev_open *curr = *list;
  struct libusb_device_descriptor desc;
  libusb_device *dev;

  while (curr) {
    dev =libusb_get_device(curr->devh);
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
      fprintf(stdout, "Warning: remove_device: could not get device device descriptior."
                          " Ignoring.\n");
      continue;
    }
    if (desc.idVendor == CLOVER_VID && desc.idProduct == CLOVER_PID
    && (sid == 0xFF || sid == get_serial_id(curr->devh))) { 
      if (manip_type == kInterfaceClaim) {
        if ((ret = libusb_claim_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not claim interface (%d) on device (%u, %u, %u)\n",
                  interface, CLOVER_VID, CLOVER_PID, sid);
        }
      }
      else if (manip_type == kInterfaceRelease) {
        if ((ret =libusb_release_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not release interface (%d) on device (%u, %u, %u)\n",
                  interface, CLOVER_VID, CLOVER_PID, sid);
        }
      }
      else {
        fprintf(stderr, "Error: handle_interface_id: Unknown interface handle request: %d\n.",
                manip_type);
              
        ret = -1;
        return ret;
      }
    }

    curr = curr->next;
  }

  return ret;
}

static void remove_device_id(struct dev_open **list, int sid)
{
  if (!*list) return;

  struct dev_open *curr = *list;
  struct dev_open *prev = 0;
  struct libusb_device_descriptor desc;
  libusb_device *dev;

  while (curr) {
    dev =libusb_get_device(curr->devh);
    if (libusb_get_device_descriptor(dev, &desc) < 0) {
      fprintf(stdout, "Warning, remove_device: could not get device device descriptior." " Ignoring.\n");
      continue;
    }
    if (desc.idVendor == CLOVER_VID && desc.idProduct == CLOVER_PID
    && (sid == 0xFF || sid == get_serial_id(curr->devh))) { 
      if (*list == curr) { 
        *list = curr->next;
        libusb_close(curr->devh);
        free(curr); 
        curr = *list;
      }
      else {
        prev->next = curr->next;
        libusb_close(curr->devh);
        free(curr); 
        curr = prev->next;
      }
    }
    else {
      prev = curr;
      curr = curr->next;
    }    
  }
}

libusb_device_handle* nkusb_get_device_handle(int sid) 
{
  struct dev_open *curr = ldev_open;
  while (curr) {
    if (curr->vendor_id == CLOVER_VID && curr->product_id == CLOVER_PID) {
      if (sid == 0xFF)
        return curr->devh;
      else if (curr->serial_id == sid)
        return curr->devh;
    }

    curr = curr->next;
  }

  return 0;
}

int USB3Reset(int sid)
{
  const unsigned int timeout = 1000;
  unsigned char data;
  int stat = 0;
  
  libusb_device_handle *devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "USB3WriteControl: Could not get device handle for the device.\n");
    return -1;
  }
  
  if ((stat = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, 
                                      0xD6, 0, 0, &data, 0, timeout)) < 0) {
    fprintf(stderr, "USB3WriteControl:  Could not make write request; error = %d\n", stat);
    return stat;
  }
  
  return stat;
}

int USB3Read_i(int sid, int mid, unsigned long count, unsigned long addr, unsigned char *data)
{
  const unsigned int timeout = 1000; // Wait forever
  int length = 12;
  int transferred;
  unsigned char *buffer;
  int stat = 0;
  int nbulk;
  int remains;
  int loop;
  int size = 16384; // 16 kB

  nbulk = count / 4096;
  remains = count % 4096;

  if (!(buffer = (unsigned char *)malloc(size))) {
    fprintf(stderr, "USB3Read: Could not allocate memory (size = %d\n)", size);
    return -1;
  }
  
  buffer[0] = count & 0xFF;
  buffer[1] = (count >> 8)  & 0xFF;
  buffer[2] = (count >> 16)  & 0xFF;
  buffer[3] = (count >> 24)  & 0xFF;
  
  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8)  & 0xFF;
  buffer[6] = (addr >> 16)  & 0xFF;
  buffer[7] = (addr >> 24)  & 0x7F;
  buffer[7] = buffer[7] | 0x80;

  buffer[8] = mid & 0xFF;
  buffer[9] = 0;
  buffer[10] = 0;
  buffer[11] = 0;

  libusb_device_handle *devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "USB3Read: Could not get device handle for the device.\n");
    return -1;
  }

  if ((stat = libusb_bulk_transfer(devh, 0x06, buffer, length, &transferred, timeout)) < 0) {
    fprintf(stderr, "USB3Read: Could not make write request; error = %d\n", stat);
    USB3Reset(sid);
    free(buffer);
    return stat;
  }

  for (loop = 0; loop < nbulk; loop++) {
    if ((stat = libusb_bulk_transfer(devh, 0x82, buffer, size, &transferred, timeout)) < 0) {
      fprintf(stderr, "USB3Read: Could not make read request; error = %d\n", stat);
      USB3Reset(sid);
      return 1;
    }
    memcpy(data + loop * size, buffer, size);
  }

  if (remains) {
    if ((stat = libusb_bulk_transfer(devh, 0x82, buffer, remains * 4, &transferred, timeout)) < 0) {
      fprintf(stderr, "USB3Read: Could not make read request; error = %d\n", stat);
      USB3Reset(sid);
      return 1;
    }

    memcpy(data + nbulk * size, buffer, remains * 4);
  }

  free(buffer);
  
  return 0;
}

int USB3Write(int sid, int mid, unsigned long addr, unsigned long data)
{
  int transferred = 0;  
  const unsigned int timeout = 1000;
  int length = 12;
  unsigned char *buffer;
  int stat = 0;
  
  if (!(buffer = (unsigned char *)malloc(length))) {
    fprintf(stderr, "USB3Write: Could not allocate memory (size = %d\n)", length);
    return -1;
  }
  
  buffer[0] = data & 0xFF;
  buffer[1] = (data >> 8)  & 0xFF;
  buffer[2] = (data >> 16)  & 0xFF;
  buffer[3] = (data >> 24)  & 0xFF;
  
  buffer[4] = addr & 0xFF;
  buffer[5] = (addr >> 8)  & 0xFF;
  buffer[6] = (addr >> 16)  & 0xFF;
  buffer[7] = (addr >> 24)  & 0x7F;

  buffer[8] = mid & 0xFF;
  buffer[9] = 0;
  buffer[10] = 0;
  buffer[11] = 0;
  
  libusb_device_handle *devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "USB3Write: Could not get device handle for the device.\n");
    return -1;
  }
  
  if ((stat = libusb_bulk_transfer(devh, 0x06, buffer, length, &transferred, timeout)) < 0) {
    fprintf(stderr, "USB3Write: Could not make write request; error = %d\n", stat);
    USB3Reset(sid);
    free(buffer);
    return stat;
  }
  
  free(buffer);

  usleep(1000);

  return stat;
}

unsigned long USB3Read(int sid, int mid, unsigned long addr)
{
  unsigned char data[4];
  unsigned long value;
  unsigned long tmp;

  USB3Read_i(sid, mid, 1, addr, data);

  value = data[0] & 0xFF;
  tmp = data[1] & 0xFF;
  value = value + (unsigned long)(tmp << 8);
  tmp = data[2] & 0xFF;
  value = value + (unsigned long)(tmp << 16);
  tmp = data[3] & 0xFF;
  value = value + (unsigned long)(tmp << 24);

  return value;
}

int USB3Read_Block(int sid, unsigned long count, unsigned long addr, unsigned char *data)
{
  return USB3Read_i(sid, 0, count, addr, data);
}

void CLOVERreset_REFCLK(int sid, int mid)
{
  USB3Write(sid, mid, 0x20000030, 0);
}

void CLOVERreset_ADC(int sid, int mid)
{
  USB3Write(sid, mid, 0x20000031, 0);
}

void CLOVERsetup_ADC(int sid, int mid, int addr, int data)
{
  unsigned long value;
  value = ((addr & 0xFFFF) << 16) | (data & 0xFFFF);
  USB3Write(sid, mid, 0x20000032, value);
}

unsigned long CLOVERread_ADC_ALIGN(int sid, int mid, int ch)
{
  unsigned long addr;
  addr = 0x20000032 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

void CLOVERwrite_ADC_DLY(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;
  addr = 0x20000033 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

void CLOVERwrite_ADC_BITSLIP(int sid, int mid, int ch)
{
  unsigned long addr;
  addr = 0x20000034 + (ch  << 16);
  USB3Write(sid, mid, addr, 0);
}

void CLOVERwrite_ADC_IMUX(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;
  addr = 0x2000003B + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

void CLOVERstart_DRAM(int sid, int mid)
{
  unsigned long ready;

  ready = USB3Read(sid, mid, 0x20000035);

  if (ready)
    USB3Write(sid, mid, 0x20000035, 0);

  USB3Write(sid, mid, 0x20000035, 1);
  
  ready = 0;
  while (!ready)
    ready = USB3Read(sid, mid, 0x20000035);
}

unsigned long CLOVERread_DRAM_READY(int sid, int mid)
{
  return USB3Read(sid, mid, 0x20000035);
}

void CLOVERwrite_DRAM_TEST(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000036, data);
}

unsigned long CLOVERread_DRAM_ALIGN(int sid, int mid, int ch)
{
  unsigned long addr;
  addr = 0x20000036 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

void CLOVERwrite_DRAM_DLY(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;
  addr = 0x20000037 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

void CLOVERwrite_DRAM_BITSLIP(int sid, int mid, int ch)
{
  unsigned long addr;
  addr = 0x20000038 + (ch  << 16);
  USB3Write(sid, mid, addr, 0);
}

void CLOVERwrite_DAC8(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000039, data);
}

void CLOVERwrite_DAC32(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;
  addr = 0x20000039 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

void CLOVERwrite_DAC2(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x2000003A, data);
}

void CLOVERwrite_RISE_TIME(int sid, int mid, int ch, unsigned long data) 
{
  unsigned long addr;

  addr = 0x20000004 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

void CLOVERwrite_FLAT_TOP(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000005 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

void CLOVERwrite_DECAY_TIME(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000006 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

void CLOVERwrite_TRAPEZOIDAL_SCALE(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000007 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

unsigned long CLOVERread_TRAPEZOIDAL_SCALE(int sid, int mid, int ch)
{
  unsigned long addr;

  addr = 0x20000007 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

void CLOVERwrite_RC_COEF(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000008, data);
}

void CLOVERwrite_CR_GAIN(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000009, data);
}
void CLOVERwrite_CR2_GAIN(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x2000000A, data);
}

// ******************************************************************************************************

// initialize libusb library
void USB3Init(void)
{
  if (libusb_init(0) < 0) {
    fprintf(stderr, "failed to initialise libusb\n");
    exit(1);
  }
}

// de-initialize libusb library
void USB3Exit(void)
{
  libusb_exit(0); 
}

// open DAQ
int CLOVERopen(int sid)
{
  struct libusb_device **devs;
  struct libusb_device *dev;
  struct libusb_device_handle *devh;
  size_t i = 0;
  int nopen_devices = 0; 
  int r;
  int interface = 0;
  int sid_tmp;
  int speed;
  int status;

  if (libusb_get_device_list(0, &devs) < 0) 
    fprintf(stderr, "Error: open_device: Could not get device list\n");

  fprintf(stdout, "Info: open_device: opening device Vendor ID = 0x%X, Product ID = 0x%X, Serial ID = %u\n", CLOVER_VID, CLOVER_PID, sid);

  while ((dev = devs[i++])) {
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      fprintf(stdout, "Warning, open_device: could not get device device descriptior." " Ignoring.\n");
      continue;
    }

    if (desc.idVendor == CLOVER_VID && desc.idProduct == CLOVER_PID)  {
      r = libusb_open(dev, &devh);
      if (r < 0) {
        fprintf(stdout, "Warning, open_device: could not open device." " Ignoring.\n");
        continue;
      }

      if (is_device_open(devh)) {
        fprintf(stdout, "Info, open_device: device already open." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      if (libusb_claim_interface(devh, interface) < 0) {
        fprintf(stdout, "Warning, open_device: could not claim interface 0 on the device." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      sid_tmp = get_serial_id(devh);

      if (sid == 0xFF || sid == sid_tmp) {
        add_device(&ldev_open, devh, sid_tmp);
        nopen_devices++;
  
        speed = libusb_get_device_speed(dev);
        switch (speed) {
          case 4:
            fprintf(stdout, "Info: open_device: super speed device opened");
            break;
          case 3:
            fprintf(stdout, "Info: open_device: high speed device opened");
            break;
          case 2:
            fprintf(stdout, "Info: open_device: full speed device opened");
            break;
          case 1:
            fprintf(stdout, "Info: open_device: low speed device opened");
            break;
          case 0:
            fprintf(stdout, "Info: open_device: unknown speed device opened");
            break;
        }
        
        fprintf(stdout, " (bus = %d, address = %d, serial id = %u).\n",
                    libusb_get_bus_number(dev), libusb_get_device_address(dev), sid_tmp);
        libusb_release_interface(devh, interface);
        break;
      }
      else {
        libusb_release_interface(devh, interface);
        libusb_close(devh);
      }
    }
  }

  libusb_free_device_list(devs, 1);

  handle_interface_id(&ldev_open, sid, 0, kInterfaceClaim);

  if (!nopen_devices)
    return -1;

  devh = nkusb_get_device_handle(sid);
  if (!devh) {
    fprintf(stderr, "Could not get device handle for the device.\n");
    return -1;
  }

   return status;
}

// close DAQ
void CLOVERclose(int sid)
{
  handle_interface_id(&ldev_open, sid, 0, kInterfaceRelease);
  remove_device_id(&ldev_open, sid);
}

// reset timer
void CLOVERresetTIMER(int sid)
{
  USB3Write(sid, 0, 0x20000000, 1);
}

// reset DAQ
void CLOVERreset(int sid)
{
  USB3Write(sid, 0, 0x20000000, 4);
}

// start DAQ
void CLOVERstart(int sid)
{
  USB3Write(sid, 0, 0x20000000, 8);
}

void CLOVERstop(int sid)
{
  USB3Write(sid, 0, 0x20000000, 0);
}

unsigned long CLOVERread_RUN(int sid)
{
  return USB3Read(sid, 0, 0x20000000);
}

void CLOVERwrite_CW(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000001, data);
}

unsigned long CLOVERread_CW(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x20000001);
}

void CLOVERwrite_RUN_NUMBER(int sid, unsigned long data)
{
  USB3Write(sid, 0, 0x20000002, data);
}

unsigned long CLOVERread_RUN_NUMBER(int sid) 
{
  return USB3Read(sid, 0, 0x20000002);
}

void CLOVERwrite_MTHR(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000003, data);
}

unsigned long CLOVERread_MTHR(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x20000003);
}

void CLOVERsend_TRIG(int sid)
{
  USB3Write(sid, 0, 0x20000004, 4);
}

unsigned long CLOVERread_LINK_STATUS(int sid) 
{
  return USB3Read(sid, 0, 0x20000005);
}

void CLOVERwrite_TRIG_ENABLE(int sid, unsigned long data)
{
  USB3Write(sid, 0, 0x20000006, data);
}

unsigned long CLOVERread_TRIG_ENABLE(int sid) 
{
  return USB3Read(sid, 0, 0x20000006);
}

void CLOVERwrite_PTRIG_INTERVAL(int sid, unsigned long data)
{
  USB3Write(sid, 0, 0x20000007, data);
}

unsigned long CLOVERread_PTRIG_INTERVAL(int sid) 
{
  return USB3Read(sid, 0, 0x20000007);
}

void CLOVERwrite_ACQUISITION_TIME(int sid, unsigned long data)
{
  USB3Write(sid, 0, 0x20000008, data);
}

unsigned long CLOVERread_ACQUISITION_TIME(int sid)
{
  return USB3Read(sid, 0, 0x20000008);
}

unsigned long CLOVERread_TCB_MID(int sid)
{
  return USB3Read(sid, 0, 0x20000009);
}

unsigned long CLOVERread_DATA_SIZE(int sid, int mid)
{
  unsigned long addr;
  addr = 0x30000000 + (mid  << 16);
  return USB3Read(sid, 0, addr);
}

void CLOVERread_DATA(int sid, int mid, unsigned long data_size, char *data)
{
  unsigned long addr;
  unsigned long count;
  addr = 0x40000000 + (mid  << 16);
  count = data_size * 256;
  USB3Read_Block(sid, count, addr, data);
}

void CLOVERwrite_POL(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000002, data);
}

unsigned long CLOVERread_POL(int sid, int mid)
{
  return USB3Read(sid, mid, 0x20000002);
}

unsigned long CLOVERread_RISE_TIME(int sid, int mid, int ch) 
{
  unsigned long addr;

  addr = 0x20000004 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

unsigned long CLOVERread_FLAT_TOP(int sid, int mid, int ch)
{
  unsigned long addr;

  addr = 0x20000005 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

unsigned long CLOVERread_DECAY_TIME(int sid, int mid, int ch)
{
  unsigned long addr;

  addr = 0x20000006 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

float CLOVERread_RC_COEF(int sid, int mid)
{
  unsigned long data;
  float fval;

  data = USB3Read(sid, mid, 0x20000008);
  fval = data;
  fval = fval / 1024.0;
  return fval;
}

float CLOVERread_CR_GAIN(int sid, int mid)
{
  unsigned long data;
  float fval;

  data = USB3Read(sid, mid, 0x20000009);
  fval = data;
  fval = fval / 1024.0;
  return fval;
}

float CLOVERread_CR2_GAIN(int sid, int mid)
{
  unsigned long data;
  float fval;

  data = USB3Read(sid, mid, 0x2000000A);
  fval = data;
  fval = fval / 1024.0;
  return fval;
}

void CLOVERmeasure_PED(int sid, int mid)
{
  USB3Write(sid, mid, 0x2000000B, 0);
}

unsigned long CLOVERread_PED(int sid, int mid, int ch) 
{
  unsigned long addr;

  addr = 0x2000000B + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

void CLOVERwrite_THR(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x2000000C + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

unsigned long CLOVERread_THR(int sid, int mid, int ch) 
{
  unsigned long addr;

  addr = 0x2000000C + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

void CLOVERwrite_TRIG_MODE(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x2000000D, data);
}

unsigned long CLOVERread_TRIG_MODE(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x2000000D);
}

void CLOVERwrite_GATE_WIDTH(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x2000000E, data);
}

unsigned long CLOVERread_GATE_WIDTH(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x2000000E);
}

void CLOVERwrite_PRESCALE(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x2000000F, data);
}

unsigned long CLOVERread_PRESCALE(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x2000000F);
}

void CLOVERwrite_DLY(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000010, data);
}

unsigned long CLOVERread_DLY(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x20000010);
}

void CLOVERwrite_ZERO_SUP(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000011, data);
}

unsigned long CLOVERread_ZERO_SUP(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x20000011);
}

void CLOVERwrite_WAVEFORM_SEL(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000012, data);
}

unsigned long CLOVERread_WAVEFORM_SEL(int sid, int mid) 
{
  return USB3Read(sid, mid, 0x20000012);
}

void CLOVERwrite_PEAKSUM_WIDTH(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000013 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

unsigned long CLOVERread_PEAKSUM_WIDTH(int sid, int mid, int ch)
{
  unsigned long addr;

  addr = 0x20000013 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

void CLOVERwrite_PEAKSUM_DLY(int sid, int mid, int ch, unsigned long data)
{
  unsigned long addr;

  addr = 0x20000014 + (ch  << 16);
  USB3Write(sid, mid, addr, data);
}

unsigned long CLOVERread_PEAKSUM_DLY(int sid, int mid, int ch)
{
  unsigned long addr;

  addr = 0x20000014 + (ch  << 16);
  return USB3Read(sid, mid, addr);
}

void CLOVERwrite_ENABLE_FASTDAQ(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000015, data);
}

unsigned long CLOVERread_ENABLE_FASTDAQ(int sid, int mid)
{
  return USB3Read(sid, mid, 0x20000015);
}

void CLOVERwrite_PEAK_MODE(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x20000016, data);
}

unsigned long CLOVERread_PEAK_MODE(int sid, int mid)
{
  return USB3Read(sid, mid, 0x20000016);
}

void CLOVERset_TRAPEZOIDAL_FILTER(int sid, int mid, int ch, unsigned long rise_time, unsigned long flat_top, unsigned long decay_time)
{
  unsigned long data;
  unsigned long srise_time;
  unsigned long sdecay_time;
  float fval;
  
  fval = rise_time;
  fval = fval / 32.0 + 0.5;
  data = (unsigned long)(fval);
  data = data * 32;
  CLOVERwrite_RISE_TIME(sid, mid, ch, data); 

  fval = flat_top;
  fval = fval / 32.0 + 0.5;
  data = (unsigned long)(fval);
  data = data * 32;
  CLOVERwrite_FLAT_TOP(sid, mid, ch, data); 

  fval = decay_time;
  fval = fval / 512.0 + 0.5;
  data = (unsigned long)(fval);
  data = data * 512;
  CLOVERwrite_DECAY_TIME(sid, mid, ch, data); 
  
  if (ch)
    srise_time = CLOVERread_RISE_TIME(sid, mid, ch) / 32;
  else
    srise_time = CLOVERread_RISE_TIME(sid, mid, 1) / 32;
  if (ch)
    sdecay_time = CLOVERread_DECAY_TIME(sid, mid, ch) / 512;
  else 
    sdecay_time = CLOVERread_DECAY_TIME(sid, mid, 1) / 512;
  data = 256 * 65536 / sdecay_time / srise_time;
  CLOVERwrite_TRAPEZOIDAL_SCALE(sid, mid, ch, data); 
}

void CLOVERset_RCCR2_FILTER(int sid, int mid, float rc_coef, float cr_gain, float cr2_gain)
{
  unsigned long data;
  float fval;
  
  fval = rc_coef * 1024 + 0.5;
  data = (fval);
  CLOVERwrite_RC_COEF(sid, mid, data);

  fval = cr_gain * 1024 + 0.5;
  data = (fval);
  CLOVERwrite_CR_GAIN(sid, mid, data);

  fval = cr2_gain * 1024 + 0.5;
  data = (fval);
  CLOVERwrite_CR2_GAIN(sid, mid, data);
}

void CLOVERwrite_ECHO(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x2000003E, data);
}

unsigned long CLOVERread_ECHO(int sid, int mid)
{
  return USB3Read(sid, mid, 0x2000003E);
}

void CLOVERwrite_ID(int sid, int mid, unsigned long data)
{
  USB3Write(sid, mid, 0x2000003F, data);
}

unsigned long CLOVERread_ID(int sid, int mid)
{
  return USB3Read(sid, mid, 0x2000003F);
}

void CLOVERinit_ADC(int sid, int mid)
{
  unsigned long id;
  int retry;
  int all_aligned;
  int ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  unsigned long gdly;
  int bitslip;
  int aligned;

  // read id
  id = CLOVERread_ID(sid, mid) & 0x80;

  // for ADC10
  if (id == 0) {
    // reset input delay ref clock
    CLOVERreset_REFCLK(sid, mid); 
  
    // reset ADC
    CLOVERsetup_ADC(sid, mid, 0x00, 0x80);
    
    // turn on ADC ch
    CLOVERsetup_ADC(sid, mid, 0x01, 0x00);
    
    for (retry = 0; retry < 2; retry++) {
      all_aligned = 0;
      
      // set test pattern
      CLOVERsetup_ADC(sid, mid, 0x02, 0x05);
      
      for (ch = 0; ch < 5; ch++) {
        count = 0;
        sum = 0;
        flag = 0;
        aligned = 0;

        // set bit sync test pattern
        CLOVERsetup_ADC(sid, mid, 0x03, 0xCC);
        CLOVERsetup_ADC(sid, mid, 0x04, 0xCC);

        // search delay
        for (dly = 0; dly < 32; dly++) {
          // set delay
          CLOVERwrite_ADC_DLY(sid, mid, ch, dly); 

          // read bit_alignment status
          value = CLOVERread_ADC_ALIGN(sid, mid, ch) & 0x1;

          if(value) {
            count = count + 1;
            sum = sum + dly;
            if(count > 12) 
              flag = 1;
          }
          else{
            if (flag) 
              dly = 32;
            else {
              count = 0;
              sum = 0;
            }
          }
        }
        
        // get good center
        if (count > 12) {
          gdly = sum / count;
          aligned = 1;
        }
        else
          gdly = 0;
    
        // set good delay
        CLOVERwrite_ADC_DLY(sid, mid, ch, gdly); 

        // read word sync test pattern
        CLOVERsetup_ADC(sid, mid, 0x03, 0xC0);
        CLOVERsetup_ADC(sid, mid, 0x04, 0xC0);

        // get bitslip
        for (bitslip = 0; bitslip < 4; bitslip++) {
          value = CLOVERread_ADC_ALIGN(sid, mid, ch) & 0x2;

          if (value) {
            aligned = aligned + 1;
            bitslip = 4;
          }
          else 
            CLOVERwrite_ADC_BITSLIP(sid, mid, ch);
        }
      
        all_aligned = all_aligned + aligned;
      }

      if (all_aligned == 10) 
        retry = 2;
    }
        
    // set ADC in normal output mode
    CLOVERsetup_ADC(sid, mid, 0x02, 0x01);

    if (all_aligned == 10)
      printf("ADC is aligned.\n");
    else
      printf("Fail to align ADC!\n");
  }
  
  // for ADC32
  if (id == 0x80) {
    // reset input delay ref clock
    CLOVERreset_REFCLK(sid, mid); 
  
    // reset ADC
    CLOVERsetup_ADC(sid, mid, 0x00, 0x80);
    
    // set ADC registers
    CLOVERsetup_ADC(sid, mid, 0x00, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x01, 0x0010);
    CLOVERsetup_ADC(sid, mid, 0xD1, 0x0240);
    CLOVERsetup_ADC(sid, mid, 0x02, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x0F, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x14, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x1C, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x24, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x28, 0x8100);
    CLOVERsetup_ADC(sid, mid, 0x29, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x2A, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x2B, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x38, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x42, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x45, 0x0000);
    CLOVERsetup_ADC(sid, mid, 0x46, 0x8801);
    
    for (retry = 0; retry < 2; retry++) {
      all_aligned = 0;
      
      for (ch = 0; ch < 4; ch++) {
        count = 0;
        sum = 0;
        flag = 0;
        aligned = 0;

        // set test pattern
        CLOVERsetup_ADC(sid, mid, 0x26, 0x3330);
        CLOVERsetup_ADC(sid, mid, 0x25, 0x0013);
      
        // search delay
        for (dly = 0; dly < 32; dly++) {
          // set delay
          CLOVERwrite_ADC_DLY(sid, mid, ch, dly); 

          // read bit_alignment status
          value = CLOVERread_ADC_ALIGN(sid, mid, ch) & 0x1;

          if(value) {
            count = count + 1;
            sum = sum + dly;
            if(count > 12) 
              flag = 1;
          }
          else{
            if (flag) 
              dly = 32;
            else {
              count = 0;
              sum = 0;
            }
          }
        }
        
        // get good center
        if (count > 12) {
          gdly = sum / count;
          aligned = 1;
        }
        else
          gdly = 0;
    
        // set good delay
        CLOVERwrite_ADC_DLY(sid, mid, ch, gdly); 

        // read word sync test pattern
        CLOVERsetup_ADC(sid, mid, 0x26, 0x0000);
        CLOVERsetup_ADC(sid, mid, 0x25, 0x0012);

        // get bitslip
        CLOVERwrite_ADC_IMUX(sid, mid, ch, 0);
        for (bitslip = 0; bitslip < 8; bitslip++) {
          if (bitslip == 4)
            CLOVERwrite_ADC_IMUX(sid, mid, ch, 1);
            
          value = CLOVERread_ADC_ALIGN(sid, mid, ch) & 0x2;

          if (value) {
            aligned = aligned + 1;
            bitslip = 8;
          }
          else 
            CLOVERwrite_ADC_BITSLIP(sid, mid, ch);
        }
      
        all_aligned = all_aligned + aligned;
      }

      if (all_aligned == 8) 
        retry = 2;
    }
        
    // set ADC in normal output mode
    CLOVERsetup_ADC(sid, mid, 0x25, 0x0000);

    if (all_aligned == 8)
      printf("ADC is aligned.\n");
    else
      printf("Fail to align ADC!\n");
  }      
}

void CLOVERinit_DRAM(int sid, int mid)
{
  int retry;
  int all_aligned;
  int ch;
  unsigned long dly;
  unsigned long value;
  int flag;
  int count;
  int sum;
  int aflag;
  unsigned long gdly;
  int bitslip;
  int aligned;
  
  // turn on DRAM    
  CLOVERstart_DRAM(sid, mid);
  
  // enter DRAM test mode
  CLOVERwrite_DRAM_TEST(sid, mid, 1); 

  for (retry = 0; retry < 2; retry++) {
    all_aligned = 0;

    // send reset to iodelay  
    CLOVERreset_REFCLK(sid, mid); 

    // fill DRAM test pattern
    CLOVERwrite_DRAM_TEST(sid, mid, 2); 

    for (ch = 0; ch < 8; ch++) {
      count = 0;
      sum = 0;
      flag = 0;
      aligned = 0;

      // search delay
      for (dly = 0; dly < 32; dly++) {
        // set delay
        CLOVERwrite_DRAM_DLY(sid, mid, ch, dly);

        // read DRAM test pattern
        CLOVERwrite_DRAM_TEST(sid, mid, 3); 
        value = CLOVERread_DRAM_ALIGN(sid, mid, ch); 

        aflag = 0;
        if (value == 0xFFAA5500)
          aflag = 1;
        else if (value == 0xAA5500FF)
          aflag = 1;
        else if (value == 0x5500FFAA)
          aflag = 1;
        else if (value == 0x00FFAA55)
          aflag = 1;
    
        if (aflag) {
          count = count + 1;
          sum = sum + dly;
          if (count > 8)
            flag = 1; 
        }
        else {
          if (flag) {
            dly = 32;
            aligned = 1;
          }
          else {
            count = 0;
            sum = 0;
          }
        }
      }

      // get good delay center
      if (count)
        gdly = sum / count;
      else
        gdly = 9;

      // set delay
      CLOVERwrite_DRAM_DLY(sid, mid, ch, gdly);
  
      // get bitslip
      for (bitslip = 0; bitslip < 4; bitslip++) {
        // read DRAM test pattern
        CLOVERwrite_DRAM_TEST(sid, mid, 3); 
        value = CLOVERread_DRAM_ALIGN(sid, mid, ch); 

        if (value == 0xFFAA5500) {
          aligned = aligned + 1;
          bitslip = 4;
        }
        else 
          CLOVERwrite_DRAM_BITSLIP(sid, mid, ch);
      }
      
      all_aligned = all_aligned + aligned;
    }
    
    if (all_aligned == 16) 
      retry = 2;
  }

  if (all_aligned == 16)
    printf("DRAM is aligned.\n");
  else
    printf("Fail to align DRAM!\n");
   
  // exit DRAM test mode
  CLOVERwrite_DRAM_TEST(sid, mid, 0); 
}

void CLOVERinit_DAC(int sid, int mid)
{
  unsigned long adc_type;
  
  adc_type = CLOVERread_ID(sid, mid) & 0x80;
  
  if (adc_type) {
    CLOVERwrite_DAC32(sid, mid, 0, 0x9000);
    CLOVERwrite_DAC32(sid, mid, 0, 0x8004);
    CLOVERwrite_DAC32(sid, mid, 1, 0x9000);
    CLOVERwrite_DAC32(sid, mid, 1, 0x8004);
    CLOVERwrite_DAC32(sid, mid, 2, 0x9000);
    CLOVERwrite_DAC32(sid, mid, 2, 0x8004);
    CLOVERwrite_DAC32(sid, mid, 3, 0x9000);
    CLOVERwrite_DAC32(sid, mid, 3, 0x8004);
  }
  else {
    CLOVERwrite_DAC8(sid, mid, 0x090A0000);
    CLOVERwrite_DAC8(sid, mid, 0x060000FF);
    CLOVERwrite_DAC8(sid, mid, 0x040000FF);
    CLOVERwrite_DAC2(sid, mid, 0x00380001);
    CLOVERwrite_DAC2(sid, mid, 0x00300001);
    CLOVERwrite_DAC2(sid, mid, 0x00020003);
    CLOVERwrite_DAC2(sid, mid, 0x00200003);
  }
}

void CLOVERwrite_OFFSET(int sid, int mid, int ch, unsigned long data)
{
  unsigned long adc_type;
  unsigned long value;
  unsigned long pol;
  unsigned long dac_val;
  unsigned long dac_sel;
  unsigned long dac_ch;

  adc_type = CLOVERread_ID(sid, mid) & 0x80;
  
  if (adc_type) {
    if (ch == 0) {
      for (dac_sel = 0; dac_sel < 4; dac_sel++) {
        for (dac_ch = 0; dac_ch < 8; dac_ch++) {
          dac_val = dac_ch * 4096 + data / 16;
          CLOVERwrite_DAC32(sid, mid, dac_sel, dac_val);
        }
      }
    }
    else {
      dac_sel = ((ch - 1) >> 3) & 0x3;
      dac_ch = (ch - 1) & 0x7;
      if (dac_ch == 1) 
        dac_val = 0x5000 + data / 16;
      else if (dac_ch == 2)
        dac_val = 0x6000 + data / 16;
      else if (dac_ch == 3)
        dac_val = 0x7000 + data / 16;
      else if (dac_ch == 4)
        dac_val = 0x3000 + data / 16;
      else if (dac_ch == 5)
        dac_val = 0x2000 + data / 16;
      else if (dac_ch == 6)
        dac_val = 0x1000 + data / 16;
      else if (dac_ch == 7)
        dac_val = 0x0000 + data / 16;
      else
        dac_val = 0x4000 + data / 16;
      
      CLOVERwrite_DAC32(sid, mid, dac_sel, dac_val);
    }
  }
  else {
    pol = CLOVERread_POL(sid, mid); 
    if (pol)
      dac_val = 65535 - data;
    else 
      dac_val = data;

    if (ch == 0) {
      value = 0x03000000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x03200000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x03400000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x03600000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x03100000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x03300000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x03500000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x03700000 | ((dac_val & 0xFFFF) << 4);
      CLOVERwrite_DAC8(sid, mid, value);

      value = 0x00180000 | (dac_val & 0xFFFF);
      CLOVERwrite_DAC2(sid, mid, value);

      value = 0x00190000 | (dac_val & 0xFFFF);
      CLOVERwrite_DAC2(sid, mid, value);
    }
    else {
      if (ch == 1) {
        value = 0x03000000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 2) {
        value = 0x03200000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 3) {
        value = 0x03400000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 4) {
        value = 0x03600000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 5) {
        value = 0x03100000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 6) {
        value = 0x03300000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 7) {
        value = 0x03500000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 8) {
        value = 0x03700000 | ((dac_val & 0xFFFF) << 4);
        CLOVERwrite_DAC8(sid, mid, value);
      }
      else if (ch == 9) {
        value = 0x00180000 | (dac_val & 0xFFFF);
        CLOVERwrite_DAC2(sid, mid, value);
      }
      else if (ch == 10) {
        value = 0x00190000 | (dac_val & 0xFFFF);
        CLOVERwrite_DAC2(sid, mid, value);
      }
    }
  }

  usleep(1000000);
}






