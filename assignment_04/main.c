#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Malone Apryl <mapryl@student.21-school.ru>");
MODULE_DESCRIPTION("Hello world");

static int hello_init(void)
{
	printk(KERN_ALERT "Hello, world !\n");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "Cleaning up module.\n");
}

module_init(hello_init);
module_exit(hello_exit);
