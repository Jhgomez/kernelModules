// all modules needs this
#include <linux/module.h>
// To be able to use kern_info
#include <linux/kernel.h>
// header for intitialization and clean macros
#include <linux/init.h>
// Needed to use procfs functions
#include <linux/proc_fs.h>
// Needed to use functions to copy data between user's space and kernel
#include <asm/uaccess.h>
// To be able to use seq_file functions
#include <linux/seq_file.h>
// To be able to use memory functions
#include <linux/mm.h>
// To be able to use sysinfo structure
#include <linux/sysinfo.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Module creation example in Linux");
MODULE_AUTHOR("Juan Gomez");
MODULE_VERSION("1.0");

/*
Build a json with RAM memory information
{
    "total_ram": 0,
    "free_ram": 0,
    "used_ram": 0,
    "percentage_used": 0,
}
*/

static int write_file(struct seq_file * file, void *v) {
    struct sysinfo info;
    // variables for storing memory info
    long total_ram, free_ram, used_ram, percentage_used;
    si_meminfo(&info);
    // Obtain memory info
    total_ram = (info.totalram * info.mem_unit) / (1024 * 1024);
    // Parse memory info to MB
    free_ram = (info.freeram * info.mem_unit) / (1024 * 1024);
    used_ram = total_ram - free_ram;
    percentage_used = (used_ram * 100) / total_ram;
    // Write info
    seq_printf(file, "{\n");
    seq_printf(file, "\"total_ram\": %lu,\n", total_ram);
    seq_printf(file, "\"free_ram\": %lu,\n", free_ram);
    seq_printf(file, "\"used_ram\": %lu,\n", used_ram);
    seq_printf(file, "\"percentage_used\": %lu\n", percentage_used);
    seq_printf(file, "}\n");
    return 0;
}

// This function is executed when a CAT is made to the module
// CAT is like printing something and is a GNU core utility

static int on_invoke(struct inode *inode, struct file *file) {
    return single_open(file, write_file, NULL);
}

// If kernel is 5.6.0 or above use the following structure
static struct proc_ops operations = {
    .proc_open = on_invoke,
    .proc_read = seq_read
};

static int _insert(void) {
    proc_create("ram", 0, NULL, &operations);
    printk(KERN_INFO "Creating file /proc/ram\n - Class 4\n");
    return 0;
}

static void _delete(void) {
    remove_proc_entry("ram", NULL);
    printk(KERN_INFO "Deleting file /proc/ram\n - Class 4\n");
}

module_init(_insert);
module_exit(_delete);
