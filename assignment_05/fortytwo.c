#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

static const char login[] = "mapryl\n";
static const size_t login_size = ARRAY_SIZE(login);

static ssize_t ft_read(struct file *file, char __user *buf, size_t len, loff_t *ppos)
{
	int read_len = *ppos >= login_size ? 0 : login_size - *ppos;
	unsigned long not_copied_len = 0;

	if(read_len < 0)
		read_len = 0;

	if(read_len) {
		not_copied_len = copy_to_user(buf, login + *ppos, read_len);
		*ppos += read_len;
	}
		
	return read_len - not_copied_len;
}

static ssize_t ft_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos)
{
	const size_t only_login_size = login_size - 1; // minus '\n' symbol
	char local_buf[only_login_size];
	
	if(len != only_login_size)
		return -EINVAL;

	if(copy_from_user(local_buf, buf, len))
		return -EINVAL;

	return len;
}

static const struct file_operations ft_fops = {
	.owner		= THIS_MODULE,
	.read		= ft_read,
	.write		= ft_write,
	.llseek 	= no_llseek,
};

struct miscdevice ft_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "fortytwo",
	.fops = &ft_fops,
};

static int __init ft_init(void)
{
	int error;

	error = misc_register(&ft_device);
	if (error) {
		pr_err("can't misc_register\n");
		return error;
	}

	pr_info("ft loaded\n");
	return 0;
}

static void __exit ft_exit(void)
{
	misc_deregister(&ft_device);
	pr_info("ft unloaded\n");
}

module_init(ft_init);
module_exit(ft_exit);

MODULE_DESCRIPTION("fortytwo device");
MODULE_AUTHOR("Malone Apryl <mapryl@student.21-school.ru>");
MODULE_LICENSE("GPL");
