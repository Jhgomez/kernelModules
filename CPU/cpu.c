// provides file management functions
#include <linux/fs.h>
// Used in initialization and cleaning macros
#include <linux/init.h>
// For using kernel functions
#include <linux/kernel.h>
// For using modules functions
#include <linux/module.h>
// For using seq_file functions
#include <linux/seq_file.h>
// For using file management functions
#include <linux/stat.h>
// For using string functions
#include <linux/string.h>
// For using copy data functions between user space and kernel
#include <linux/uaccess.h>
// For using memory functions
#include <linux/mm.h>
// For using sysinfo structure
#include <linux/sysinfo.h>
// For using task management functions
#include <linux/sched/task.h>
// For using task management functions
#include <linux/sched.h>
// For using procfs functions
#include <linux/proc_fs.h>
// For using copy data functions between user space and kernel
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Linux module creation");
MODULE_AUTHOR("Juan Gomez");
MODULE_VERSION("1.0");

// This module reads cpu file which contains CPU information

static int CPUUsagePercentage(void){
    struct file *file;
    char read[256];

    int user, nice, idle, iowait, irq, softirq, steal, guest, guest_nice;
    // Variables to store CPU information
    // user is the time CPU has spent in user mode
    // nice is time CPU has spent in user mode in low priority
    // idle is time CPU has spent doing nothing
    // io wait is time CPU has spent waiting for I/O operations to complete
    // irq is time CPU has spent proccessing hardware interruptions
    // softirq is time CPU has spent proccessing software interruptions
    // steal is time CPU has spent in oc mode
    // guest is time CPU has spent executing a VM
    // guest_nice is time CPU has spent executing a VM in low priority
    int total, percentage;

    // Open /proc/stat file
    file = filp_open("/proc/stat", O_RDONLY, 0);
    if (file == NULL){
        printk(KERN_INFO "Error opening file\n");
        return -1;
    }

    // Clean read var
    memset(read, 0, 256);

    // Read file and store it in read variable
    kernel_read(file, read, 256, &file->f_pos);
    

    sscanf(read, "cpu %d %d %d %d %d %d %d %d %d", &user, &nice, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

    total = user + nice + idle + iowait + irq + softirq + steal + guest + guest_nice;

    percentage = (total - idle) * 100 / total;

    filp_close(file, NULL);

    return percentage;
}

static int write_file(struct seq_file *file, void *v) {
    int percentage = CPUUsagePercentage();
    if (percentage == -1) {
        seq_printf(file, "Error reading file\n");
    } else {
        seq_printf(file, "{\n");
        seq_printf(file, "\"percentage_used\": %d,\n", percentage);
        seq_printf(file, "\"tasks\": [\n");

        struct task_struct *task;
        int ram;
        bool first_task = true;

        for_each_process(task) {
            if (!first_task) {
                seq_printf(file, ",\n");
            }
            seq_printf(file, "    {\n");
            seq_printf(file, "      \"pid\": %d,\n", task->pid);
            seq_printf(file, "      \"name\": \"%s\",\n", task->comm);
            seq_printf(file, "      \"state\": %d,\n", task->__state);
            seq_printf(file, "      \"user\": %d,\n", task->cred->uid.val);
            if (task->mm) {
                ram = (get_mm_rss(task->mm) << PAGE_SHIFT) / (1024 * 1024);
                seq_printf(file, "      \"ram\": %d,\n", ram);
            } else {
                seq_printf(file, "      \"ram\": null,\n");
            }
            seq_printf(file, "      \"father\": %d\n", task->parent->pid);
            seq_printf(file, "    }");
            first_task = false;
        }

        seq_printf(file, "\n  ]\n");
        seq_printf(file, "}\n");
    }
    return 0;
}

static int on_invoke(struct inode *inode, struct file *file) {
    return single_open(file, write_file, NULL);
}

static struct proc_ops operations = {
    .proc_open = on_invoke,
    .proc_read = seq_read
};

static int _insert(void) {
    proc_create("cpu", 0, NULL, &operations);
    printk(KERN_INFO "Creating file /proc/cpu\n - Class 4\n");
    return 0;
}

static void _delete(void) {
    remove_proc_entry("cpu", NULL);
    printk(KERN_INFO "Deleting file /proc/cpu\n - Class 4\n");
}

module_init(_insert);
module_exit(_delete);