#include "update_adc.h"

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
static void add_device(struct dev_open **list, libusb_device_handle *tobeadded,
                       uint16_t vendor_id, uint16_t product_id, int sid);
static int handle_interface_id(struct dev_open **list, uint16_t vendor_id, uint16_t product_id,
                               int sid, int interface, enum EManipInterface manip_type);
static void remove_device_id(struct dev_open **list, uint16_t vendor_id, uint16_t product_id, int sid);
libusb_device_handle* nkusb_get_device_handle(uint16_t vendor_id, uint16_t product_id, int sid);
int USB3WriteControl(int sid, uint8_t bRequest, uint16_t wLength, unsigned char *data);
int USB3ReadControl(int sid, uint8_t bRequest, uint16_t wLength, unsigned char *data);

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

static void add_device(struct dev_open **list, libusb_device_handle *tobeadded,
                       uint16_t vendor_id, uint16_t product_id, int sid)
{
  struct dev_open *curr;

  curr = (struct dev_open *)malloc(sizeof(struct dev_open));
  curr->devh = tobeadded;
  curr->vendor_id = vendor_id;
  curr->product_id = product_id;
  curr->serial_id = sid;
  curr->next  = *list;
  *list = curr;
}

static int handle_interface_id(struct dev_open **list, uint16_t vendor_id, uint16_t product_id,
                               int sid, int interface, enum EManipInterface manip_type)
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
    if (desc.idVendor == vendor_id && desc.idProduct == product_id
    && (sid == 0xFF || sid == get_serial_id(curr->devh))) { 
      if (manip_type == kInterfaceClaim) {
        if ((ret = libusb_claim_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not claim interface (%d) on device (%u, %u, %u)\n",
                  interface, vendor_id, product_id, sid);
        }
      }
      else if (manip_type == kInterfaceRelease) {
        if ((ret =libusb_release_interface(curr->devh, interface)) < 0) {
          fprintf(stdout, "Warning: handle_interface_id: Could not release interface (%d) on device (%u, %u, %u)\n",
                  interface, vendor_id, product_id, sid);
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

static void remove_device_id(struct dev_open **list, uint16_t vendor_id, uint16_t product_id, int sid)
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
    if (desc.idVendor == vendor_id && desc.idProduct == product_id
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

libusb_device_handle* nkusb_get_device_handle(uint16_t vendor_id, uint16_t product_id, int sid) 
{
  struct dev_open *curr = ldev_open;
  while (curr) {
    if (curr->vendor_id == vendor_id && curr->product_id == product_id) {
      if (sid == 0xFF)
        return curr->devh;
      else if (curr->serial_id == sid)
        return curr->devh;
    }

    curr = curr->next;
  }

  return 0;
}

int USB3WriteControl(int sid, uint8_t bRequest, uint16_t wLength, unsigned char *data)
{
  const unsigned int timeout = 5000;
  int stat = 0;
  
  libusb_device_handle *devh = nkusb_get_device_handle(CLOVER_VID, CLOVER_PID, sid);
  if (!devh) {
    fprintf(stderr, "USB3Write: Could not get device handle for the device.\n");
    return -1;
  }
  
  if ((stat = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_OUT, bRequest, 0, 0, data, wLength, timeout)) < 0) {
    fprintf(stderr, "USB3WriteControl:  Could not make write request; error = %d\n", stat);
    return stat;
  }
  
  return stat;
}

int USB3ReadControl(int sid, uint8_t bRequest, uint16_t wLength, unsigned char *data)
{
  const unsigned int timeout = 5000;
  int stat = 0;
  
  libusb_device_handle *devh = nkusb_get_device_handle(CLOVER_VID, CLOVER_PID, sid);
  if (!devh) {
    fprintf(stderr, "USB3ReadControl: Could not get device handle for the device.\n");
    return -1;
  }
  
  if ((stat = libusb_control_transfer(devh, LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_ENDPOINT_IN, bRequest, 0, 0, data, wLength, timeout)) < 0) {
    fprintf(stderr, "USB3ReadControl: Could not make read request; error = %d\n", stat);
    return stat;
  }

  return 0;
}

int CLOVERopen(int sid)
{
  struct libusb_device **devs;
  struct libusb_device *dev;
  struct libusb_device_handle *devh;
  size_t i = 0;
  int nopen_devices = 0; //number of open devices
  int r;
  int interface = 0;
  int sid_tmp;
  int speed;

  if (libusb_init(0) < 0) {
    fprintf(stderr, "failed to initialise libusb\n");
    exit(1);
  }

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

      // do not open twice
      if (is_device_open(devh)) {
        fprintf(stdout, "Info, open_device: device already open." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      // See if sid matches
      // Assume interface 0
      if (libusb_claim_interface(devh, interface) < 0) {
        fprintf(stdout, "Warning, open_device: could not claim interface 0 on the device." " Ignoring.\n");
        libusb_close(devh);
        continue;
      }

      sid_tmp = get_serial_id(devh);

      if (sid == 0xFF || sid == sid_tmp) {
        add_device(&ldev_open, devh, CLOVER_VID, CLOVER_PID, sid_tmp);
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

  // claim interface
  handle_interface_id(&ldev_open, CLOVER_VID, CLOVER_PID, sid, 0, kInterfaceClaim);

  if (!nopen_devices)
    return -1;

  devh = nkusb_get_device_handle(CLOVER_VID, CLOVER_PID, sid);
  if (!devh) {
    fprintf(stderr, "Could not get device handle for the device.\n");
    return -1;
  }

  return 0;
}

void CLOVERclose(int sid)
{
  handle_interface_id(&ldev_open, CLOVER_VID, CLOVER_PID, sid, 0, kInterfaceRelease);
  remove_device_id(&ldev_open, CLOVER_VID, CLOVER_PID, sid);
  libusb_exit(0); 
}

int CLOVERport(int sid, int port)
{
  unsigned char wdat[2];
  
  wdat[0] = port & 0xFF;
  return USB3WriteControl(sid, VENDOR_ADC_SELECT, 1, wdat);
}

int CLOVERerase(int sid, int sector)
{
  unsigned char wdat[2];
  
  wdat[0] = sector & 0xFF;
  return USB3WriteControl(sid, VENDOR_ADC_ERASE, 1, wdat);
}

int CLOVERfinish(int sid)
{
  unsigned char wdat[2];
  
  return USB3WriteControl(sid, VENDOR_ADC_FINISH, 0, wdat);
}

int CLOVERwrite(int sid, int sector, int page, char *data)
{
  unsigned char wdat[258];
  int i;

  wdat[0] = sector & 0xFF;
  wdat[1] = page & 0xFF;
  for (i = 0; i < 256; i++)
    wdat[i + 2] = i & 0xFF;
//    wdat[i + 2] = data[i] & 0xFF;
  return USB3WriteControl(sid, VENDOR_ADC_WRITE, 258, wdat);
}

int CLOVERread(int sid, unsigned char *data)
{
  return USB3ReadControl(sid, VENDOR_READ_ADC_FPGA_VERSION, 256, data);
}


int main(int argc, char *argv[])
{
  int sid;
  int slot;
  int is_dev;
  char filename[256];
  unsigned char version[256];
  int flag;
  FILE *fp;
  int file_size;
  int nsector;
  int sector;
  int npage;
  int page;
  int naddr;
  int addr;
  char data[256];
  int proc_full;
  int proc;
  int name_size;
  int i;

  if(argc < 3) {
    printf("./update_adc.exe tcb_sid adc_slot to see uploaded firmware version\n");
    printf("./update_adc.exe tcb_sid adc_slot FPGA_firmare_name for uploading firmware\n");
    return 0;
  }
  
  sid = atoi(argv[1]);
  slot = atoi(argv[2]);

  // open clover daq
  is_dev = CLOVERopen(sid);
  if (is_dev < 0) {
    printf("No Clover DAQ exist!\n");
  }

  // set slot
  CLOVERport(sid, slot);

  // show firmware version
  CLOVERread(sid, version);
proc_full = 0;
for (i = 0; i < 256; i++) {
proc = version[i] & 0xFF;
if (proc != i) {
printf("mismatch %d != %d\n", proc, i);
proc_full = proc_full + 1;
}
}
if (proc_full) 
printf("mismatch = %d\n", proc_full);
else
printf("okay!!!\n");
  
/*
  printf("Version: ");
  for (i = 0; i < 256; i++) {
    flag = version[i] & 0xFF;
    if (flag == 0xFF) {
      printf("\n");
      i = 256;
    }
    else
      printf("%c", version[i]);
  }
  */
 
  if (argc > 3) {
  /*
    sprintf(filename, "%s", argv[3]);
    if ((access(filename, 0)) == 0) {
      fp = fopen(filename, "rb");
      fseek(fp, 0L, SEEK_END);
      file_size = ftell(fp);
      fclose(fp);
      
      nsector = file_size / 65536;
      npage = (file_size % 65536) / 256;
      naddr = (file_size % 65536) % 256;
      proc_full = nsector * 256 + npage + 1;
      proc = 0;
      
      fp = fopen(filename, "rb");

      for (sector = 0; sector < nsector; sector++) {
        // erase sector
        CLOVERerase(sid, sector + 2);

        // write data
        for (page = 0; page < 256; page++) {
          fread(data, 1, 256, fp);
          CLOVERwrite(sid, sector + 2, page, data);
          proc = proc + 1;
          printf("%d / %d programmed\n", proc, proc_full);
        }
      }

      // erase sector
      CLOVERerase(sid, nsector + 2);

      // write data
      for (page = 0; page < npage; page++) {
        fread(data, 1, 256, fp);
        CLOVERwrite(sid, nsector + 2, page, data);
        proc = proc + 1;
        printf("%d / %d programmed\n", proc, proc_full);
      }

      if (naddr) {
        fread(data, 1, naddr, fp);
        for (addr = naddr; addr < 256; addr++)
          data[addr] = 0;
        CLOVERwrite(sid, nsector + 2, npage, data);
      }

      fclose(fp);

      // erase sector
      CLOVERerase(sid, 1);
      
      for (i = 0; i < 256; i++) {
        if (filename[i] == 0) {
          name_size = i;
          i = 256;
        }
        else
          data[i] = filename[i] & 0xFF;
      }

      for (i = name_size; i < 256; i++)
        data[i] = 0xFF;
        
      CLOVERwrite(sid, 1, 0, data);
    
      CLOVERfinish(sid);

      proc = proc + 1;
      printf("%d / %d programmed\n", proc, proc_full);
    }
    else
      printf("No %s\n", filename);
      */

      // erase sector
      CLOVERerase(sid, 1);
      
      CLOVERwrite(sid, 1, 0, data);
    
      CLOVERfinish(sid);
  }

  // close clover daq
  CLOVERclose(sid);

  return 0;
}  

