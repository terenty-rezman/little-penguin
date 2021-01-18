/**
 * this module is complete sh*t but i'm not gonna rewrite it from scratch
 * just fix a little
 **/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>

/* Dont have a license, LOL */
MODULE_LICENSE("LICENSE");
MODULE_AUTHOR("Louis Solofrizzo <louis@ne02ptzero.me>");
MODULE_DESCRIPTION("Useless module");

static ssize_t myfd_read(struct file *fp, char __user *user, size_t size,
			 loff_t *offs);

static ssize_t myfd_write(struct file *fp, const char __user *user, size_t size,
			  loff_t *offs);

static struct file_operations myfd_fops = { 
    .owner = THIS_MODULE,
    .read = &myfd_read,
    .write = &myfd_write
};

static struct miscdevice myfd_device = { 
    .minor = MISC_DYNAMIC_MINOR,
    .name = "reverse",
    .fops = &myfd_fops 
};

static char str[PAGE_SIZE];
static DEFINE_MUTEX(myfd_mutex);

static int __init myfd_init(void)
{
	int error;

	error = misc_register(&myfd_device);
	if (error) {
		pr_err("can't misc_register reverse\n");
		return error;
	}

	return 0;
}

static void __exit myfd_cleanup(void)
{
	misc_deregister(&myfd_device);
}

/**
 * this function is complete sh*t in particular
**/
ssize_t myfd_read(struct file *fp, char __user *user, size_t size, loff_t *offs)
{
	int t; 
	int i;
	size_t ret;
	char *tmp;

	/**
     * Malloc like a boss
     **/
	tmp = kmalloc(sizeof(char) * PAGE_SIZE, GFP_KERNEL);
	if(tmp == NULL)
		return -ENOMEM;

	mutex_lock(&myfd_mutex);
	for (t = strlen(str) - 1, i = 0; t >= 0; t--, i++) {
		tmp[i] = str[t];
	}
	mutex_unlock(&myfd_mutex);

	tmp[i] = '\0';
	ret = simple_read_from_buffer(user, size, offs, tmp, i);
	kfree(tmp);
	return ret;
}

ssize_t myfd_write(struct file *fp, const char __user *user, size_t size,
		   loff_t *offs)
{
	ssize_t res;

	mutex_lock(&myfd_mutex);
	res = simple_write_to_buffer(str, ARRAY_SIZE(str) - 1, offs, user, size);
	if(res >= 0) {
		str[*offs] = '\0';
	}
	mutex_unlock(&myfd_mutex);
	return res;
}

module_init(myfd_init);
module_exit(myfd_cleanup);