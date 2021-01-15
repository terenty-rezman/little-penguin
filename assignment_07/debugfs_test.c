#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/debugfs.h>

static char foo_data[PAGE_SIZE] = {0};
static size_t foo_data_len = 0;
static DECLARE_RWSEM(foo_sem);

int foo_open(struct inode *inode, struct file *file)
{
    /* trim data to 0 if opened in wtire only replicating default behavior for ordinary files */
    if ((file->f_flags & O_ACCMODE) == O_WRONLY)
    {
        down_write(&foo_sem);
        foo_data_len = 0;
        up_write(&foo_sem);
    }

    return 0; /* success */
}

static ssize_t foo_read(struct file *file, char *buf, size_t len, loff_t *offset)
{
    int read_count = 0;
    int can_read_count = foo_data_len - *offset;

    if(can_read_count < 0)
        can_read_count = 0;
    
    if(len > can_read_count)
        read_count = can_read_count;
    else
        read_count = len;

    down_read(&foo_sem);

    if(read_count) {
        if (copy_to_user(buf, foo_data + *offset, read_count)) {
            up_read(&foo_sem);
            return -EFAULT; /* error */
        }
    }

    *offset += read_count;

    up_read(&foo_sem);
    return read_count; /* success */
}

static ssize_t foo_write(struct file *file, const char __user *buf, size_t len, loff_t *offset)
{
    int write_count = 0;
    int can_write_count = ARRAY_SIZE(foo_data) - *offset;

    if(can_write_count < 0)
        can_write_count = 0;

    if(len > can_write_count)
        write_count = can_write_count;
    else
        write_count = len;

    down_write(&foo_sem);

    if(write_count) {
        if(copy_from_user(foo_data + *offset, buf, write_count)) {
            up_write(&foo_sem);
            return -EFAULT; /* error */
        }
    }

    *offset += write_count;

    if(foo_data_len < *offset)
        foo_data_len = *offset;

    up_write(&foo_sem);
    return len;
}

static const struct file_operations foo_fops = {
    .owner = THIS_MODULE,
    .open = foo_open,
    .read = foo_read,
    .write = foo_write,
    .llseek = no_llseek,
};
static struct dentry *debug_dir;

int __init init_module(void)
{
    struct dentry *foo_file;

    debug_dir = debugfs_create_dir("fortytwo", NULL);
    if (IS_ERR(debug_dir))
		return PTR_ERR(debug_dir);

    foo_file = debugfs_create_file("foo", S_IWUSR | S_IROTH, debug_dir, NULL, &foo_fops);
    if (IS_ERR(foo_file))
		return PTR_ERR(foo_file);

    pr_info("test module loaded\n");

    return 0;
}

void __exit cleanup_module(void)
{
    debugfs_remove_recursive(debug_dir);
    pr_info("test module unloaded\n");
}

MODULE_DESCRIPTION("debugfs test");
MODULE_AUTHOR("Malone Apryl <terenty.rezman@mail.ru>");
MODULE_LICENSE("GPL");