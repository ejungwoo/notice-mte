#include "NoticeLTE.h"

enum EManipInterface {kInterfaceClaim, kInterfaceRelease};

struct dev_open {
   libusb_device_handle *devh;
   uint16_t vendor_id;
   uint16_t product_id;
   int serial_id;
   struct dev_open *next;
} *ldev_open = 0;

// internal functions *********************************************************************************
static int is_device_open_LTE(libusb_device_handle *devh);
static unsigned char get_serial_id_LTE(libusb_device_handle *devh);
static void add_device_LTE(struct dev_open **list, libusb_device_handle *tobeadded, int sid);
static int handle_interface_id_LTE(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type);
static void remove_device_id_LTE(struct dev_open **list, int sid);
libusb_device_handle* nkusb_get_device_handle_LTE(int sid);
int USB3Reset_LTE(int sid);
int USB3Read_i_LTE(int sid, unsigned long count, unsigned long addr, unsigned char *data);
int USB3Write_LTE(int sid, unsigned long addr, unsigned long data);
unsigned long USB3Read_LTE(int sid, unsigned long addr);

static int is_device_open_LTE(libusb_device_handle *devh)
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

static unsigned char get_serial_id_LTE(libusb_device_handle *devh)
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

static void add_device_LTE(struct dev_open **list, libusb_device_handle *tobeadded, int sid)
{
  struct dev_open *curr;

  curr = (struct dev_open *)malloc(sizeof(struct dev_open));
  curr->devh = tobeadded;
  curr->vendor_id = LTE_VID;
  curr->product_id = LTE_PID;
  curr->serial_id = sid;
  curr->next  = *list;
  *list = curr;
}

static int handle_interface_id_LTE(struct dev_open **list, int sid, int interface, enum EManipInterface manip_type)
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
    if (desc.idVendor == LTE_VID && desc.idProduct == LTE_PID
    && (sid == 0xFF || sid == get_serial_id_LTE(curr->devh))) { 
      if (manip_type == kInterfaceClaim) {
        if ((ret = libusb_claim_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not claim interface (%d) on device (%u, %u, %u)\n",
                  interface, LTE_VID, LTE_PID, sid);
        }
      }
      else if (manip_type == kInterfaceRelease) {
        if ((ret =libusb_release_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not release interface (%d) on device (%u, %u, %u)\n",
                  interface, LTE_VID, LTE_PID, sid);
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

static void remove_device_id_LTE(struct dev_open **list, int sid)
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
    if (desc.idVendor == LTE_VID && desc.idProduct == LTE_PID
    && (sid == 0xFF || sid == get_serial_id_LTE(curr->devh))) { 
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

libusb_device_handle* nkusb_get_device_handle_LTE(int sid) 
{
  struct dev_open *curr = ldev_open;
  while (curr) {
    if (curr->vendor_id == LTE_VID && curr->product_id == LTE_PID) {
      if (sid == 0xFF)
        return curr->devh;
      else if (curr->serial_id == sid)
        return curr->devh;
    }

    curr = curr->next;
  }

  return 0;
}

int USB3Reset_LTE(int sid)
{
  const unsigned int timeout = 1000;
  unsigned char data;
  int stat = 0;
  
  libusb_device_handle *devh = nkusb_get_device_handle_LTE(sid);
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

int USB3Read_i_LTE(int sid, unsigned long count, unsigned long addr, unsigned char *data)
{
  const unsigned int timeout = 1000; // Wait forever
  int length = 8;
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

  libusb_device_handle *devh = nkusb_get_device_handle_LTE(sid);
  if (!devh) {
    fprintf(stderr, "USB3Read: Could not get device handle for the device.\n");
    return -1;
  }

  if ((stat = libusb_bulk_transfer(devh, 0x06, buffer, length, &transferred, timeout)) < 0) {
    fprintf(stderr, "USB3Read: Could not make write request; error = %d\n", stat);
    USB3Reset_LTE(sid);
    free(buffer);
    return stat;
  }

  for (loop = 0; loop < nbulk; loop++) {
    if ((stat = libusb_bulk_transfer(devh, 0x82, buffer, size, &transferred, timeout)) < 0) {
      fprintf(stderr, "USB3Read: Could not make read request; error = %d\n", stat);
      USB3Reset_LTE(sid);
      return 1;
    }
    memcpy(data + loop * size, buffer, size);
  }

  if (remains) {
    if ((stat = libusb_bulk_transfer(devh, 0x82, buffer, remains * 4, &transferred, timeout)) < 0) {
      fprintf(stderr, "USB3Read: Could not make read request; error = %d\n", stat);
      USB3Reset_LTE(sid);
      return 1;
    }

    memcpy(data + nbulk * size, buffer, remains * 4);
  }

  free(buffer);
  
  return 0;
}

int USB3Write_LTE(int sid, unsigned long addr, unsigned long data)
{
  int transferred = 0;  
  const unsigned int timeout = 1000;
  int length = 8;
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

  libusb_device_handle *devh = nkusb_get_device_handle_LTE(sid);
  if (!devh) {
    fprintf(stderr, "USB3Write: Could not get device handle for the device.\n");
    return -1;
  }
  
  if ((stat = libusb_bulk_transfer(devh, 0x06, buffer, length, &transferred, timeout)) < 0) {
    fprintf(stderr, "USB3Write: Could not make write request; error = %d\n", stat);
    USB3Reset_LTE(sid);
    free(buffer);
    return stat;
  }
  
  free(buffer);

  usleep(1000);

  return stat;
}

unsigned long USB3Read_LTE(int sid, unsigned long addr)
{
  unsigned char data[4];
  unsigned long value;
  unsigned long tmp;

  USB3Read_i_LTE(sid, 1, addr, data);

  value = data[0] & 0xFF;
  tmp = data[1] & 0xFF;
  value = value + (unsigned long)(tmp << 8);
  tmp = data[2] & 0xFF;
  value = value + (unsigned long)(tmp << 16);
  tmp = data[3] & 0xFF;
  value = value + (unsigned long)(tmp << 24);

  return value;
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

// open LTE
int LTEopen(int sid)
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

  fprintf(stdout, "Info: open_device: opening device Vendor ID = 0x%X, Product ID = 0x%X, Serial ID = %u\n",
                   LTE_VID, LTE_PID, sid);

  while ((dev = devs[i++])) {
    struct libusb_device_descriptor desc;
    r = libusb_get_device_descriptor(dev, &desc);
    if (r < 0) {
      fprintf(stdout, "Warning, open_device: could not get device device descriptior." " Ignoring.\n");
      continue;
    }

    if (desc.idVendor == LTE_VID && desc.idProduct == LTE_PID)  {
      r = libusb_open(dev, &devh);
      if (r < 0) {
        fprintf(stdout, "Warning, open_device: could not open device." " Ignoring.\n");
        continue;
      }

      if (is_device_open_LTE(devh)) {
        fprintf(stdout, "Info, open_device: device already open." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      if (libusb_claim_interface(devh, interface) < 0) {
        fprintf(stdout, "Warning, open_device: could not claim interface 0 on the device." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      sid_tmp = get_serial_id_LTE(devh);

      if (sid == 0xFF || sid == sid_tmp) {
        add_device_LTE(&ldev_open, devh, sid_tmp);
        nopen_devices++;
  
        // Print out the speed of just open device 
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

  handle_interface_id_LTE(&ldev_open, sid, 0, kInterfaceClaim);

  if (!nopen_devices)
    return -1;

  devh = nkusb_get_device_handle_LTE(sid);
  if (!devh) {
    fprintf(stderr, "Could not get device handle for the device.\n");
    return -1;
  }

  return status;
}

// close LTE
void LTEclose(int sid)
{
  handle_interface_id_LTE(&ldev_open, sid, 0, kInterfaceRelease);
  remove_device_id_LTE(&ldev_open, sid);
}

// reset timer
void LTEresetTIMER(int sid)
{
  USB3Write_LTE(sid, 0x20000000, 1);
}

// reset data acquisition
void LTEreset(int sid)
{
  USB3Write_LTE(sid, 0x20000000, 4);
}

// start data acquisition
void LTEstart(int sid)
{
  USB3Write_LTE(sid, 0x20000000, 8);
}

// stop data acquisition
void LTEstop(int sid)
{
  USB3Write_LTE(sid, 0x20000000, 0);
}

// read RUN status
unsigned long LTEread_RUN(int sid)
{
  return USB3Read_LTE(sid, 0x20000000);
}

void LTEwrite_CW(int sid, unsigned long data)
{
  USB3Write_LTE(sid, 0x20000001, data);
}

unsigned long LTEread_CW(int sid) 
{
  return USB3Read_LTE(sid, 0x20000001);
}

void LTEwrite_RUN_NUMBER(int sid, unsigned long data)
{
  USB3Write_LTE(sid, 0x20000002, data);
}

unsigned long LTEread_RUN_NUMBER(int sid) 
{
  return USB3Read_LTE(sid, 0x20000002);
}

void LTEwrite_MTHR(int sid, unsigned long data)
{
  USB3Write_LTE(sid, 0x20000003, data);
}

unsigned long LTEread_MTHR(int sid) 
{
  return USB3Read_LTE(sid, 0x20000003);
}

void LTEsend_TRIG(int sid)
{
  USB3Write_LTE(sid, 0x20000004, 0);
}

unsigned long LTEread_LINK_STATUS(int sid) 
{
  return USB3Read_LTE(sid, 0x20000005);
}

void LTEwrite_TRIG_ENABLE(int sid, unsigned long data)
{
  USB3Write_LTE(sid, 0x20000006, data);
}

unsigned long LTEread_TRIG_ENABLE(int sid) 
{
  return USB3Read_LTE(sid, 0x20000006);
}

void LTEwrite_PTRIG_INTERVAL(int sid, unsigned long data)
{
  USB3Write_LTE(sid, 0x20000007, data);
}

unsigned long LTEread_PTRIG_INTERVAL(int sid) 
{
  return USB3Read_LTE(sid, 0x20000007);
}

void LTEwrite_ACQUISITION_TIME(int sid, unsigned long data)
{
  USB3Write_LTE(sid, 0x20000008, data);
}

unsigned long LTEread_ACQUISITION_TIME(int sid)
{
  return USB3Read_LTE(sid, 0x20000008);
}

int LTEread_MID(int sid, int ch)
{
  unsigned long addr;
  
  addr = 0x20000010 + ch;
  return USB3Read_LTE(sid, addr);
}

void LTEwrite_ECHO(int sid, unsigned long data)
{
  USB3Write_LTE(sid, 0x2000000F, data);
}

unsigned long LTEread_ECHO(int sid)
{
  return USB3Read_LTE(sid, 0x2000000F);
}

