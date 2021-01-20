#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/seq_file.h>
#include <linux/nsproxy.h>
#include <linux/mount.h>
#include <linux/namei.h>
#include <linux/path.h>
#include <linux/kallsyms.h>
#include <linux/proc_fs.h>

static struct vfsmount *(*kallsym_collect_mounts)(struct path *);
static void (*kallsym_drop_collected_mounts)(struct vfsmount *);
static int (*kallsym_iterate_mounts)(int (*)(struct vfsmount *, void *), void *, struct vfsmount *);

static int print_mount(struct vfsmount *mnt, void *arg)
{
	struct seq_file *m = arg;
	struct dentry *dentry;

	dentry = mnt->mnt_root;

	seq_printf(m, "%s\n", dentry->d_name.name);
	return 0;
}

static int mymounts_show(struct seq_file *m, void *v)
{
	int err;
	struct path root_path;
	struct vfsmount *root_mnt;

	err = kern_path("/", LOOKUP_FOLLOW, &root_path);
	if (err) {
		pr_err("mymounts: kern_path failed\n");
		return 0;
	}

	root_mnt = kallsym_collect_mounts(&root_path);

	kallsym_iterate_mounts(print_mount, m, root_mnt);

	kallsym_drop_collected_mounts(root_mnt);
	path_put(&root_path);

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

static int __init mymounts_init(void)
{
	struct proc_dir_entry *entry;

	entry = proc_create("mymounts", S_IRUSR, NULL, &mymounts_fops);
	if (!entry) {
		pr_err("Failed to create error_log proc entry\n");
		return -ENOMEM;
	}

	kallsym_collect_mounts = (void *)kallsyms_lookup_name("collect_mounts");
	kallsym_drop_collected_mounts = (void *)kallsyms_lookup_name("drop_collected_mounts");
	kallsym_iterate_mounts = (void *)kallsyms_lookup_name("iterate_mounts");

	if (!kallsym_collect_mounts || !kallsym_drop_collected_mounts ||
	    !kallsym_iterate_mounts) {
		pr_err("mymounts: kallsyms_lookup_name failed\n");
		remove_proc_entry("mymounts", NULL);
		return 1;
	}

	pr_info("ft loaded\n");
	return 0;
}

static void __exit mymounts_exit(void)
{
	remove_proc_entry("mymounts", NULL);
	pr_info("ft unloaded\n");
}

module_init(mymounts_init);
module_exit(mymounts_exit);

MODULE_DESCRIPTION("prints all mount points");
MODULE_AUTHOR("Malone Apryl <mapryl@student.21-school.ru>");
MODULE_LICENSE("GPL");