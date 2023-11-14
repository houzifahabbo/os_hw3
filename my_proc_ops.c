#include <linux/slab.h>    /* for kmalloc() */
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/proc_fs.h> /* proc_ops, proc_create, proc_remove, remove_proc_entry... */

// UL is to make it unsigned int
#define INITIAL_BUFFER_SIZE 2048UL

// The buffer used to store character data for this proc entry.
static int state_filter = -2;
static char *global_buffer;

static unsigned long global_buffer_size = 0;

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
    size_t bytes_written = 0, buffer_capacity = INITIAL_BUFFER_SIZE;

    if (*offset || global_buffer_size == 0)
    {
        // Reset offset and return 0 for subsequent reads
        *offset = 0;
        return 0;
    }

    for_each_process(task)
    {
        int task_state = task_state_index(task);
        if (bytes_written >= buffer_capacity)
        {
            // If the buffer is full, reallocate it
            char *temp_buffer = krealloc(global_buffer, buffer_capacity * 2, GFP_KERNEL);
            if (!temp_buffer)
            {
                // Return error if reallocation fails
                kfree(global_buffer);
                return -ENOMEM;
            }

            global_buffer = temp_buffer;
            buffer_capacity *= 2;
            global_buffer_size = buffer_capacity;
        }
        if (task_state == state_filter || state_filter == -1)
        {
            int ret = snprintf(global_buffer + bytes_written, buffer_capacity - bytes_written,
                               "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
                               task->pid, task_state_array[task_state], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);

            if (ret < 0)
            {
                // Return error if snprintf fails
                kfree(global_buffer);
                return ret;
            }

            bytes_written += ret;
        }
    }

    // Update the size of the buffer
    global_buffer_size = bytes_written;

    // Copy data to user space
    if (copy_to_user(usr_buf, global_buffer, global_buffer_size))
    {
        return -EFAULT;
    }

    // Update the offset and reset the state filter
    *offset += global_buffer_size;
    state_filter = -2;
    return global_buffer_size;
}

ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)
{
    // Set the buffer size to the minimum of INITIAL_BUFFER_SIZE and incoming size
    global_buffer_size = min(INITIAL_BUFFER_SIZE, size);

    // Copy data from user space to kernel space
    if (copy_from_user(global_buffer, usr_buf, global_buffer_size))
        return -EFAULT;

    // Update the offset and set the state filter based on the first character of the buffer
    *offset += global_buffer_size;
    if (strncasecmp(global_buffer, "R", 1) == 0)
    {
        state_filter = 0;
    }
    else if (strncasecmp(global_buffer, "S", 1) == 0)
    {
        state_filter = 1;
    }
    else if (strncasecmp(global_buffer, "D", 1) == 0)
    {
        state_filter = 2;
    }
    else if (strncasecmp(global_buffer, "T", 1) == 0)
    {
        state_filter = 3;
    }
    else if (strncasecmp(global_buffer, "t", 1) == 0)
    {
        state_filter = 4;
    }
    else if (strncasecmp(global_buffer, "X", 1) == 0)
    {
        state_filter = 5;
    }
    else if (strncasecmp(global_buffer, "Z", 1) == 0)
    {
        state_filter = 6;
    }
    else if (strncasecmp(global_buffer, "P", 1) == 0)
    {
        state_filter = 7;
    }
    else if (strncasecmp(global_buffer, "I", 1) == 0)
    {
        state_filter = 8;
    }
    else
    {
        state_filter = -1;
    }

    return global_buffer_size;
}

int my_open(struct inode *inode, struct file *file)
{
    // Allocate memory for the global buffer
    global_buffer = kmalloc(INITIAL_BUFFER_SIZE, GFP_KERNEL);
    if (!global_buffer)
    {
        // Return error if memory allocation fails
        return -ENOMEM;
    }

    // Initialize the buffer
    global_buffer[INITIAL_BUFFER_SIZE - 1] = '\0'; // Ensure proper null-termination
    global_buffer_size = INITIAL_BUFFER_SIZE;

    // To set the filter if the module has been called without writing to the module
    if (state_filter == -2)
    {
        state_filter = -1;
    }

    return 0;
}

int my_release(struct inode *inode, struct file *file)
{
    // Free allocated memory for the global buffer
    if (global_buffer)
    {
        kfree(global_buffer);
        global_buffer_size = 0;
    }

    return 0;
}
