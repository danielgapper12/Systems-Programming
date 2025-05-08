#define LINUX

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include "kmlab_given.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gapper");
MODULE_DESCRIPTION("CPTS360 KM PA");

#define DEBUG 1

// store per process data
struct proc_list {
    struct list_head list; // kernel's linked list 
    unsigned int pid;      // Process ID
    unsigned long cpu_time; // CPU usage time
};

// global variables
struct proc_dir_entry *proc_entry;

LIST_HEAD(proc_list);
static struct timer_list my_timer;
static DEFINE_SPINLOCK(proc_lock);

static void setup_timer(void);
static void my_timer_callback(struct timer_list *timer);
static ssize_t procfile_read(struct file *file, char __user *buffer, size_t length, loff_t *offset);
static ssize_t procfile_write(struct file *file, const char __user *buff, size_t len, loff_t *off);

// file operations structure
static const struct proc_ops proc_file_fops = {
    .proc_read = procfile_read,
    .proc_write = procfile_write,
};

// called when the module is loaded
int __init kmlab_init(void)
{
    #ifdef DEBUG
    pr_info("KMLAB MODULE LOADING\n");
    #endif

    struct proc_dir_entry *proc_dir;
    proc_dir = proc_mkdir("kmlab", NULL);

    if (!proc_dir) {
        pr_err("Failed to create /proc/kmlab directory\n");
        return -ENOMEM;
    }
    proc_entry = proc_create("status", 0666, proc_dir, &proc_file_fops);
    if (!proc_entry) {
        pr_err("Failed to create /proc/kmlab/status\n");
        return -ENOMEM;
    }

    setup_timer();

    pr_info("KMLAB MODULE LOADED\n");
    return 0;   
}

// called when module is unloaded
void __exit kmlab_exit(void)
{
    #ifdef DEBUG
    pr_info("KMLAB MODULE UNLOADING\n");
    #endif

    remove_proc_entry("status", NULL);
    remove_proc_entry("kmlab", NULL);

    struct proc_list *entry, *n;

    list_for_each_entry_safe(entry, n, &proc_list, list) {
        list_del(&entry->list);
        kfree(entry);
    }

    del_timer_sync(&my_timer);

    pr_info("KMLAB MODULE UNLOADED\n");
}

static void setup_timer(void)
{
    timer_setup(&my_timer, my_timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000)); // Schedule timer for 5 seconds
}

void my_timer_callback(struct timer_list *timer)
{
    pr_info("Timer triggered, updating CPU times.\n");

    unsigned long flags;
    spin_lock_irqsave(&proc_lock, flags);
    struct proc_list *entry;
    list_for_each_entry(entry, &proc_list, list) {
        get_cpu_use(entry->pid, &entry->cpu_time);
    }
    spin_unlock_irqrestore(&proc_lock, flags);

    mod_timer(&my_timer, jiffies + msecs_to_jiffies(5000));
}

// Read callback for /proc/kmlab/status
static ssize_t procfile_read(struct file *file, char __user *buffer, size_t length, loff_t *offset)
{
    char *output;
    size_t buf_len = 0;
    struct proc_list *entry;

    output = kmalloc(1024, GFP_KERNEL);
    if (!output)
        return -ENOMEM;

    buf_len += snprintf(output, 1024, "PID\tCPU_TIME\n");
    list_for_each_entry(entry, &proc_list, list) 
    {
        buf_len += snprintf(output + buf_len, 1024 - buf_len, "%u\t%lu\n", entry->pid, entry->cpu_time);
    }

    if (*offset >= buf_len) 
    {
        kfree(output);
        return 0;
    }
    if (copy_to_user(buffer, output, buf_len)) 
    {
        kfree(output);
        return -EFAULT;
    }

    *offset += buf_len;
    kfree(output);
    return buf_len;
}

// Write callback for /proc/kmlab/status
static ssize_t procfile_write(struct file *file, const char __user *buff, size_t len, loff_t *off)
{
    unsigned int pid;
    struct proc_list *new_entry;
    char input[32];

    if (len > 31)
        return -EINVAL;
    if (copy_from_user(input, buff, len))
        return -EFAULT;
    input[len] = '\0';

    if (kstrtouint(input, 10, &pid))
        return -EINVAL;

    new_entry = kmalloc(sizeof(struct proc_list), GFP_KERNEL);
    if (!new_entry)
        return -ENOMEM;

    new_entry->pid = pid;
    new_entry->cpu_time = 0;

    unsigned long flags;
    spin_lock_irqsave(&proc_lock, flags);
    list_add(&new_entry->list, &proc_list);
    spin_unlock_irqrestore(&proc_lock, flags);

    return len;
}

// Register init and exit functions
module_init(kmlab_init);
module_exit(kmlab_exit);
