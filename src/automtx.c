#include <linux/init.h> 
#include <linux/module.h>



static int __init automtx_driver_init(void) {
  printk(KERN_NOTICE "Initializing Automatrix Driver\n");
  return 0;
}

static void __exit automtx_driver_exit(void) {
  printk(KERN_NOTICE "Automatrix Driver teardown success.\n");
  return;
}

module_init(automtx_driver_init);
module_exit(automtx_driver_exit);

MODULE_LICENSE("MIT");
MODULE_DESCRIPTION("Automatrix Driver");
MODULE_AUTHOR("Christian Sargusingh <christian@leapsystems.online>");
MODULE_VERSION("1.0");