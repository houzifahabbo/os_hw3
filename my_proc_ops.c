#include <linux/module.h>
#include <linux/slab.h>    /*for kmalloc()*/
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#include <asm/uaccess.h>

#define INITIAL_BUFFER_SIZE 512

#define PROCFS_MAX_SIZE 2048UL

size_t buffer_capacity = INITIAL_BUFFER_SIZE;

static int state = -2;

static char *procfs_buffer;

static unsigned long procfs_buffer_size = -2;

static const char *const task_state_array[] = {
    "R (running)",
    "S (sleeping)",
    "D (disk sleep)",
    "T (stopped)",
    "t (tracing stop)",
    "X (dead)",
    "Z (zombie)",
    "P (parked)",
    "I (idle)",
};

ssize_t my_read(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)
{
    struct task_struct *task;
    size_t bytes_written = 0;
    printk(KERN_INFO "Message from read: %d\n", state);

    if (*offset || procfs_buffer_size == 0)
    {
        *offset = 0;
        return 0;
    }

    for_each_process(task)
    {
        int task_state = task_state_index(task);
        if (bytes_written >= buffer_capacity)
        {
            // If not, reallocate the buffer
            char *temp_buffer = krealloc(procfs_buffer, buffer_capacity * 2, GFP_KERNEL);
            if (!temp_buffer)
            {
                // Handle reallocation failure
                kfree(procfs_buffer);
                return -ENOMEM;
            }

            procfs_buffer = temp_buffer;
            buffer_capacity *= 2;
            procfs_buffer_size = buffer_capacity;
        }
        if (task_state == state || state == -1)
        {
            int ret = snprintf(procfs_buffer + bytes_written, buffer_capacity - bytes_written,
                               "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
                               task->pid, task_state_array[task_state], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);

            if (ret < 0)
            {
                // Handle error
                return ret;
            }

            bytes_written += ret;
        }
    }

    procfs_buffer_size = bytes_written;

    if (copy_to_user(usr_buf, procfs_buffer, procfs_buffer_size))
    {
        return -EFAULT;
    }

    *offset += procfs_buffer_size;

    return procfs_buffer_size;
}

ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)

{
    printk(KERN_INFO "Message from write before: %d\n", state);

    procfs_buffer_size = min(PROCFS_MAX_SIZE, size);

    if (copy_from_user(procfs_buffer, usr_buf, procfs_buffer_size))

        return -EFAULT;

    *offset += procfs_buffer_size;
    if (strncasecmp(procfs_buffer, "R", 1) == 0)
    {
        state = 0;
    }
    else if (strncasecmp(procfs_buffer, "S", 1) == 0)
    {
        state = 1;
    }
    else if (strncasecmp(procfs_buffer, "D", 1) == 0)
    {
        state = 2;
    }
    else if (strncasecmp(procfs_buffer, "T", 1) == 0)
    {
        state = 3;
    }
    else if (strncasecmp(procfs_buffer, "t", 1) == 0)
    {
        state = 4;
    }
    else if (strncasecmp(procfs_buffer, "X", 1) == 0)
    {
        state = 5;
    }
    else if (strncasecmp(procfs_buffer, "Z", 1) == 0)
    {
        state = 6;
    }
    else if (strncasecmp(procfs_buffer, "P", 1) == 0)
    {
        state = 7;
    }
    else if (strncasecmp(procfs_buffer, "I", 1) == 0)
    {
        state = 8;
    }
    else
    {
        state = -1;
    }
    printk(KERN_INFO "Message from write after: %d\n", state);

    return procfs_buffer_size;
}

int my_open(struct inode *inode, struct file *file)
{
    try_module_get(THIS_MODULE);
    if (!procfs_buffer)
    {
        printk(KERN_INFO "Message from open: %d\n", state);
        procfs_buffer = kmalloc(INITIAL_BUFFER_SIZE, GFP_KERNEL);
        if (!procfs_buffer)
            return -ENOMEM;

        // Initialize the buffer
        procfs_buffer[0] = '\0';
        state = -1;
    }

    return 0;
}

int my_release(struct inode *inode, struct file *file)

{

    module_put(THIS_MODULE);
    kfree(procfs_buffer);

    return 0;
}