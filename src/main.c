#include <linux/init.h> 
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux Test Driver");
MODULE_AUTHOR("Christian Sargusingh")

static int __init test_init(void) {
  printk(KERN_NOTICE "Hello World.\n");
  return 0;
}

static void __exit test_exit(void) {
  printk(KERN_NOTICE "Goodbye World.\n");
  return;
}

module_init(test_init);
module_exit(test_exit);