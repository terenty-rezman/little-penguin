#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/nsproxy.h>
#include <../fs/mount.h>

static int mymounts_show(struct seq_file *m, void *v)
{
	struct mount *mnt;

    mnt = current->nsproxy->mnt_ns->root;
	seq_printf(m, "%s\n", mnt->mnt_devname);
	return 0;
}

static int mymounts_open(struct inode *inode, struct file *file)
{
	/* using seq_file interface */
	return single_open(file, mymounts_show, NULL);
}

static const struct file_operations mymounts_fops = {
	.owner = THIS_MODULE,
	.open = mymounts_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

struct miscdevice ft_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mymounts",
	.fops = &mymounts_fops,
};

static int __init mymounts_init(void)
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

static void __exit mymounts_exit(void)
{
	misc_deregister(&ft_device);
	pr_info("ft unloaded\n");
}

module_init(mymounts_init);
module_exit(mymounts_exit);

MODULE_DESCRIPTION("prints all mount points");
MODULE_AUTHOR("Malone Apryl <mapryl@student.21-school.ru>");
MODULE_LICENSE("GPL");
