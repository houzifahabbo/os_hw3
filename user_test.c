#include <linux/module.h> 
#include <linux/slab.h>    /*for kmalloc()*/
#include <linux/init.h>    /* Needed for the macros */
#include <linux/kernel.h>  /* Needed for pr_info() */
#include <linux/proc_fs.h> /*proc_ops, proc)create, proc_remove, remove_proc_entry...*/
#include <asm/uaccess.h>


#define INITIAL_BUFFER_SIZE 512

#define PROCFS_MAX_SIZE 2048UL 
 
static const char * const task_state_array[] = {
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

    if (*offset || procfs_buffer_size == 0) { 

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

ssize_t my_write(struct file *file, const char __user *usr_buf, size_t size, loff_t *offset) 

{
    struct task_struct *task;
    int state = -1;
    procfs_buffer_size = min(PROCFS_MAX_SIZE, size);

    if (copy_from_user(procfs_buffer, usr_buf, procfs_buffer_size)) 

        return -EFAULT; 

    *offset += procfs_buffer_size; 
    
    if(strcmp(procfs_buffer , "R")==0){
        state = 0;
    } else if(strcmp(procfs_buffer , "S")==0){
        state = 1;
    } else if(strcmp(procfs_buffer , "D")==0){
        state = 2;
    } else if(strcmp(procfs_buffer , "T")==0){
        state = 3;
    } else if(strcmp(procfs_buffer , "t")==0){
        state = 4;
    } else if(strcmp(procfs_buffer , "X")==0){
        state = 5;
    } else if(strcmp(procfs_buffer , "Z")==0){
        state = 6;
    } else if(strcmp(procfs_buffer , "P")==0){
        state = 7;
    } else if(strcmp(procfs_buffer , "I")==0){
        state = 8;
    } else {
        state = -1;
    }
 
    for_each_process(task) {
        int state_index = task_state_index(task);
        if (state_index == state) {
            written_size += sprintf(procfs_buffer + written_size, "pid = %d state = %s utime = %llu, stime = %llu, utime+stime = %llu, vruntime = %llu\n", task->pid, task_state_array[task_state_index(task)], task->utime, task->stime, task->utime + task->stime, task->se.vruntime);
        }
    }
    procfs_buffer_size = written_size;
    
    return procfs_buffer_size; 

} 

 int my_open(struct inode *inode, struct file *file) 

{ 

    try_module_get(THIS_MODULE); 

    return 0; 

} 

 int my_release(struct inode *inode, struct file *file) 

{ 

    module_put(THIS_MODULE); 

    return 0; 

} 