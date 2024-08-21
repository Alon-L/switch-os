#include <linux/module.h>
#include <linux/printk.h>
#include <linux/efi.h>
#include <linux/kernel.h>

efi_guid_t switch_os_protocol_guid = EFI_GUID(0x402dfce7, 0x61fc, 0x4b83, 0x96, 0x97, 0xf4, 0x46, 0x35, 0x21, 0x5e, 0xee);

typedef struct {
    efi_status_t (*switch_os) (uint arg);
} switch_os_protocol;

int init_module(void) {
  switch_os_protocol* protocol;
  efi_status_t result;
  efi_time_t time;

  pr_info("Init switch_os kernel module\n");

  if (!efi_enabled(EFI_BOOT)) {
    pr_err("System is not booted in UEFI mode\n");
    return -ENODEV;
  }

  efi.get_time(&time, NULL);

  pr_info("efi time: %hhx:%hhx:%hhx\n", time.second, time.minute, time.hour);

  /*
  pr_info("get_time: %lx\n", (uintptr_t)efi.systab->runtime->get_time);
  pr_info("boottime: %lx\n", (uintptr_t)efi.systab->boottime);
  pr_info("locate_protocol: %lx\n", (uintptr_t)efi.systab->boottime->locate_protocol);

  efi.systab->boottime->locate_protocol(&switch_os_protocol_guid, NULL, (void **) &protocol);

  pr_info("protocol address: %lx\n", (uintptr_t)protocol);*/

 /* result = protocol->switch_os(0);

  pr_info("switch_os status: %lu\n", result);*/

  /* A non 0 return means init_module failed; module can't be loaded. */
  return 0; 
} 
 
void cleanup_module(void) {
  pr_info("Unloading switch_os kernel module\n");
} 
 
MODULE_LICENSE("GPL");
