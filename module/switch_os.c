#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>

int init_module(void) {
  pr_info("Init switch_os kernel module\n");

  return 0; 
} 
 
void cleanup_module(void) {
  pr_info("Unloading switch_os kernel module\n");
} 
 
MODULE_LICENSE("GPL");
