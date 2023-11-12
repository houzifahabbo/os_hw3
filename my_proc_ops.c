#include <linux/module.h>
#include <linux/slab.h>    /*for kmalloc()*/
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#include <asm/uaccess.h>

#define INITIAL_BUFFER_SIZE 512

#define PROCFS_MAX_SIZE 2048UL

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

static char *procfs_buffer;

static unsigned long procfs_buffer_size = 0;

ssize_t my_read(struct file *file, char __user *usr_buf, size_t size, loff_t *offset)

{

    if (*offset || procfs_buffer_size == 0)
    {

        pr_debug("procfs_read: END\n");

        *offset = 0;

        return 0;
    }

    procfs_buffer_size = min(procfs_buffer_size, size);

    if (copy_to_user(usr_buf, procfs_buffer, procfs_buffer_size))

        return -EFAULT;

    *offset += procfs_buffer_size;

    pr_debug("procfs_read: read %lu bytes\n", procfs_buffer_size);

    return procfs_buffer_size;
}

// ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)
// {
//     struct task_struct *task;
//     int state = -1;
//     char input_buffer[32]; // Assuming a reasonable size for the input

//     if (size >= sizeof(input_buffer))
//         return -EINVAL; // Input too large

//     if (copy_from_user(input_buffer, usr_buf, size))
//         return -EFAULT;

//     input_buffer[size] = '\0'; // Null-terminate the input

//     if (strcmp(input_buffer, "R") == 0) {
//         state = 0;
//     } else if (strcmp(input_buffer, "S") == 0) {
//         state = 1;
//     } else if (strcmp(input_buffer, "D") == 0) {
//         state = 2;
//     } else if (strcmp(input_buffer, "T") == 0) {
//         state = 3;
//     } else if (strcmp(input_buffer, "t") == 0) {
//         state = 4;
//     } else if (strcmp(input_buffer, "X") == 0) {
//         state = 5;
//     } else if (strcmp(input_buffer, "Z") == 0) {
//         state = 6;
//     } else if (strcmp(input_buffer, "P") == 0) {
//         state = 7;
//     } else if (strcmp(input_buffer, "I") == 0) {
//         state = 8;
//     } else {
//         return -EINVAL; // Invalid input
//     }

//     if (!procfs_buffer) {
//         procfs_buffer = kmalloc(INITIAL_BUFFER_SIZE, GFP_KERNEL);
//         if (!procfs_buffer)
//             return -ENOMEM; // Allocation failed
//         procfs_buffer[0] = '\0'; // Initialize procfs_buffer
//     }

//     for_each_process(task) {
//         int task_state = task_state_index(task);
//         if (task_state == state) {
//             // Append task information to procfs_buffer
//             sprintf(procfs_buffer + strlen(procfs_buffer),
//                     "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
//                     task->pid, task_state_array[task_state], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);
//         }
//     }

//     return size; // Return the number of bytes processed
// }

// ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)
// {
//     struct task_struct *task;
//     int state = -1;

//     // Allocate dynamic memory for the buffer if not allocated
//     if (!procfs_buffer)
//     {
//         procfs_buffer = kmalloc(INITIAL_BUFFER_SIZE, GFP_KERNEL);
//         if (!procfs_buffer)
//             return -ENOMEM;
//     }

//     // Initialize the buffer
//     procfs_buffer[0] = '\0';

//     if (copy_from_user(procfs_buffer, usr_buf, size))
//         return -EFAULT;

//     *offset += size;

//     if(strcmp(procfs_buffer , "R")==0){
//         state = 0;
//     } else if(strcmp(procfs_buffer , "S")==0){
//         state = 1;
//     } else if(strcmp(procfs_buffer , "D")==0){
//         state = 2;
//     } else if(strcmp(procfs_buffer , "T")==0){
//         state = 3;
//     } else if(strcmp(procfs_buffer , "t")==0){
//         state = 4;
//     } else if(strcmp(procfs_buffer , "X")==0){
//         state = 5;
//     } else if(strcmp(procfs_buffer , "Z")==0){
//         state = 6;
//     } else if(strcmp(procfs_buffer , "P")==0){
//         state = 7;
//     } else if(strcmp(procfs_buffer , "I")==0){
//         state = 8;
//     } else {
//         return -EINVAL;
//     }

//     // for_each_process(task)
//     // {
//     //     int state_index = task_state_index(task);
//     //     if (state_index == state)
//     //     {
//     //         // int task_size = snprintf(procfs_buffer + strlen(procfs_buffer), INITIAL_BUFFER_SIZE - strlen(procfs_buffer), "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n", task->pid, task_state_array[task_state_index(task)], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);

//     //         // if (task_size < 0)
//     //         //     return task_size;
//     //          procfs_buffer_size += snprintf(procfs_buffer + procfs_buffer_size,
//     //                 INITIAL_BUFFER_SIZE - procfs_buffer_size,
//     //                 "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
//     //                 task->pid, task_state_array[state_index], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);
//     //         // Check if the buffer is full and resize if necessary
//     //         if (strlen(procfs_buffer) + procfs_buffer_size >= INITIAL_BUFFER_SIZE)
//     //         {
//     //             char *new_buffer = kmalloc(INITIAL_BUFFER_SIZE * 2, GFP_KERNEL);
//     //             if (!new_buffer)
//     //                 return -ENOMEM;

//     //             strcpy(new_buffer, procfs_buffer);
//     //             kfree(procfs_buffer);
//     //             procfs_buffer = new_buffer;
//     //         }
//     //     }
//     // }

//     // procfs_buffer_size = min((size_t)strlen(procfs_buffer), (size_t)INITIAL_BUFFER_SIZE);

//     return procfs_buffer_size;
// }

ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)

{
    struct task_struct *task;
    int state = -1;
    procfs_buffer_size = min(PROCFS_MAX_SIZE, size);

    if (copy_from_user(procfs_buffer, usr_buf, procfs_buffer_size))

        return -EFAULT;

    *offset += procfs_buffer_size;

    if (strcmp(procfs_buffer, "R") == 0)
    {
        state = 0;
    }
    else if (strcmp(procfs_buffer, "S") == 0)
    {
        state = 1;
    }
    else if (strcmp(procfs_buffer, "D") == 0)
    {
        state = 2;
    }
    else if (strcmp(procfs_buffer, "T") == 0)
    {
        state = 3;
    }
    else if (strcmp(procfs_buffer, "t") == 0)
    {
        state = 4;
    }
    else if (strcmp(procfs_buffer, "X") == 0)
    {
        state = 5;
    }
    else if (strcmp(procfs_buffer, "Z") == 0)
    {
        state = 6;
    }
    else if (strcmp(procfs_buffer, "P") == 0)
    {
        state = 7;
    }
    else if (strcmp(procfs_buffer, "I") == 0)
    {
        state = 8;
    }
    else
    {
        state = -1;
    }

    for_each_process(task)
    {
        int state_index = task_state_index(task);
        if (state_index == state)
        {
            if (procfs_buffer_size + INITIAL_BUFFER_SIZE >= buffer_capacity)
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
            }
            printk(KERN_INFO "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
                   task->pid, task_state_array[task_state_index(task)], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);

            procfs_buffer_size += sprintf(procfs_buffer + procfs_buffer_size, // Update the buffer pointer
                                          "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
                                          task->pid, task_state_array[task_state_index(task)], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);
        }
    }

    return procfs_buffer_size;
}

// ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset)
// {
//     int state = -1;
//     procfs_buffer_size = min(PROCFS_MAX_SIZE, size);

//     if (copy_from_user(procfs_buffer, usr_buf, procfs_buffer_size))
//         return -EFAULT;

//     *offset += procfs_buffer_size;

//     if (strcmp(procfs_buffer, "R") == 0)
//     {
//         state = 0;
//     }
//     else if (strcmp(procfs_buffer, "S") == 0)
//     {
//         state = 1;
//     }
//     else if (strcmp(procfs_buffer, "D") == 0)
//     {
//         state = 2;
//     }
//     else if (strcmp(procfs_buffer, "T") == 0)
//     {
//         state = 3;
//     }
//     else if (strcmp(procfs_buffer, "t") == 0)
//     {
//         state = 4;
//     }
//     else if (strcmp(procfs_buffer, "X") == 0)
//     {
//         state = 5;
//     }
//     else if (strcmp(procfs_buffer, "Z") == 0)
//     {
//         state = 6;
//     }
//     else if (strcmp(procfs_buffer, "P") == 0)
//     {
//         state = 7;
//     }
//     else if (strcmp(procfs_buffer, "I") == 0)
//     {
//         state = 8;
//     }
//     else
//     {
//         state = -1;
//     }

//     // User specified a valid status, so include only tasks with that status
//     struct task_struct *task;
//     for_each_process(task)
//     {
//         int task_state = task_state_index(task);
//         if (task_state == state)
//         {
//             // Append task information to procfs_buffer
//             procfs_buffer_size += snprintf(procfs_buffer + procfs_buffer_size,
//                                            PROCFS_MAX_SIZE - procfs_buffer_size,
//                                            "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
//                                            task->pid, task_state_array[task_state], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);
//         }
//     }

//     return procfs_buffer_size;
// }

int my_open(struct inode *inode, struct file *file)

{
    size_t buffer_capacity = INITIAL_BUFFER_SIZE;

    struct task_struct *task;
    try_module_get(THIS_MODULE);
    if (!procfs_buffer)
    {
        procfs_buffer = kmalloc(INITIAL_BUFFER_SIZE, GFP_KERNEL);
        if (!procfs_buffer)
            return -ENOMEM;

        // Initialize the buffer
        procfs_buffer[0] = '\0';
    }

    for_each_process(task)
    {
        if (procfs_buffer_size + INITIAL_BUFFER_SIZE >= buffer_capacity)
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
        }
        printk(KERN_INFO "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
               task->pid, task_state_array[task_state_index(task)], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);

        procfs_buffer_size += sprintf(procfs_buffer + procfs_buffer_size, // Update the buffer pointer
                                      "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n",
                                      task->pid, task_state_array[task_state_index(task)], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);
    }

    return 0;
}

int my_release(struct inode *inode, struct file *file)

{

    module_put(THIS_MODULE);

    kfree(procfs_buffer);

    return 0;
}

// sudo service ssh start

// make clean && make && sudo rmmod -f mytaskinfo.ko && sudo insmod mytaskinfo.ko

// make clean && make && sudo insmod mytaskinfo.ko