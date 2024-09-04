#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>

extern void* _binary_build_core_bin_start;

int init_module(void) {
  pr_info("Init switch_os kernel module %p\n", _binary_build_core_bin_start);

  return 0; 
} 
 
void cleanup_module(void) {
  pr_info("Unloading switch_os kernel module\n");
} 
 
MODULE_LICENSE("GPL");
