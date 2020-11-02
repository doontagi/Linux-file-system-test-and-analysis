/**
 *  author : hanul lee 
 *  date: 2019.11.01
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

#define MODULE_NAME "block_log"
#define BLOCK_FILE "block_log_file"
#define BLOCK_DIR "block_log"
#define CIRCULAR_QUEUE_MAX_SIZE 1024

static struct proc_dir_entry *lkm_proc_dir;
static struct proc_dir_entry *lkm_proc_file;
char block_log[CIRCULAR_QUEUE_MAX_SIZE][100];  // buffer for block log

struct block_information {  // struct for containing block information
    unsigned long long block_number;
    long sec; // seconds
    long usec; //micro seconds
    char *fs_name; // file system name
};

extern int circular_queue_front;
extern int circular_queue_rear;
extern int circular_queue_size;
extern struct block_information circular_queue[CIRCULAR_QUEUE_MAX_SIZE];


/**
 * write_block_log_buffer - write block log received from circular queue on kernel.
 * it iterates and write each log from first element of circular queue to last element.
 */
static ssize_t write_block_log_buffer(struct file * file, const char __user * _user_bufffer, size_t count, loff_t *ppos)
{
	int i;
	int j;
	j = 0;
    printk(KERN_INFO "write block log\n");
    for (i=circular_queue_front; i<CIRCULAR_QUEUE_MAX_SIZE; i++) {
        sprintf(block_log[j], "fs: %s, block number: %lld, time: %d.%d\n", circular_queue[i].fs_name, circular_queue[i].block_number, circular_queue[i].sec, circular_queue[i].usec);
        j++;
    }
    for (i=0; i<circular_queue_front; ++i) {
        sprintf(block_log[j], "fs: %s, block number: %lld, time: %d.%d\n", circular_queue[i].fs_name, circular_queue[i].block_number, circular_queue[i].sec, circular_queue[i].usec);
        j++;
    }
    return count;
}

/**
 * read_block_log_buffer - read the buffer where logs are stored. And copy it into user buffer.
 */
static ssize_t read_block_log_buffer(struct file * file, const char __user * _user_bufffer, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "read block log\n");

	// To prevent the read from being repeated
	if(*ppos > 0) {
    	return 0;
    }
	if(copy_to_user(_user_bufffer, block_log, sizeof(block_log))) {
		return -EFAULT;
	}
    *ppos = sizeof(block_log);
    return sizeof(block_log);
}

static int open_block_log(struct inode * inode, struct file * file)
{
    printk(KERN_INFO "open block log\n");
    return 0;
}

static const struct file_operations myproc_fops = {
        .owner = THIS_MODULE,
        .open = open_block_log,
        .write = write_block_log_buffer,
        .read = read_block_log_buffer,
};

static int __init lkm_init(void)
{
    printk(KERN_INFO "initialize lkm\n");
    lkm_proc_dir = proc_mkdir(BLOCK_DIR, NULL);
    lkm_proc_file = proc_create(BLOCK_FILE, 0600, lkm_proc_dir, &myproc_fops);

    return 0;
}

static int __exit lkm_exit(void)
{
    printk(KERN_INFO "exit lkm\n");
    remove_proc_entry(BLOCK_FILE, lkm_proc_dir);
    remove_proc_entry(BLOCK_DIR, NULL);
    return 0;
}

module_init(lkm_init);
module_exit(lkm_exit);
MODULE_AUTHOR("Hanul Lee");
MODULE_DESCRIPTION("PRINT BLOCK NUMBER");
MODULE_LICENSE("GPL");
MODULE_VERSION("NEW");