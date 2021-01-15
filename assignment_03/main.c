#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("Dual BSD/GPL");

int do_work(int amount)
{
	int x;
	int z;

	for (x = 0; x < amount; ++x)
		udelay(10);

	if (amount > 10) {
		/* That was a long sleep, tell userspace about it */
		pr_info("We slept a long time!");
	}

	z = x * amount;

	return z;
}

int my_init(void)
{
	int x = 10;

	x = do_work(x);

	return x;
}

void my_exit(void)
{
}

module_init(my_init);
module_exit(my_exit);
