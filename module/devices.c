#include "devices.h"

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>

#include "core/header.h"
#include "core_loader.h"
#include "linux/suspend.h"
#include "suspend/hook_sleep_prepare.h"

#define SWITCH_OS_DEV_DIR "switch_os"

extern struct core_header* g_core_header;

struct switch_os_device {
  char* name;
  struct file_operations fops;
  struct cdev cdev;
};

/**
 * Calls core with the `CORE_ACTION_SWITCH` action.
 * This switches the running system with the dump on the disk.
 */
static int switch_open(struct inode* inode, struct file* file) {
  err_t err = SUCCESS;

  g_core_header->action = CORE_ACTION_SWITCH;

  CHECK_RETHROW(hook_sleep_prepare());

  CHECK(pm_suspend(PM_SUSPEND_MEM) == 0);

cleanup:
  unhook_sleep_prepare();

  return IS_SUCCESS(err) ? 0 : -EINVAL;
}

/**
 * Calls core with the `CORE_ACTION_STORE` action.
 * This stores a dump of the running system on the disk.
 */
static int store_open(struct inode* inode, struct file* file) {
  err_t err = SUCCESS;

  g_core_header->action = CORE_ACTION_STORE;

  CHECK_RETHROW(hook_sleep_prepare());

  CHECK(pm_suspend(PM_SUSPEND_MEM) == 0);

cleanup:
  unhook_sleep_prepare();

  return IS_SUCCESS(err) ? 0 : -EINVAL;
}

static struct switch_os_device g_devices[] = {
  {
    .name = "switch",
    .fops = {.open = switch_open},
  },
  {
    .name = "store",
    .fops = {.open = store_open},
  },
};

err_t register_devices(void) {
  err_t err = SUCCESS;

  // Receive a free dynamic major number for the character devices.
  dev_t region_id = 0;
  CHECK(alloc_chrdev_region(&region_id, 0, ARRAY_SIZE(g_devices), "switch_os") == 0);
  int major = MAJOR(region_id);

  for (size_t i = 0; i < ARRAY_SIZE(g_devices); i++) {
    struct switch_os_device* device = &g_devices[i];
    dev_t dev_id = MKDEV(major, i);

    // Create the device so it shows in `/dev`.
    struct class* class = class_create(device->name);
    CHECK(!IS_ERR(device_create(class, NULL, dev_id, NULL, SWITCH_OS_DEV_DIR "/%s", device->name)));

    // Add the file operations.
    cdev_init(&device->cdev, &device->fops);
    CHECK(cdev_add(&device->cdev, dev_id, 1) == 0);
  }

cleanup:
  return err;
}
