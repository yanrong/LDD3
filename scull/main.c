/**
 * main.c -- the bare scull char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/system.h>     /*cli(), *_flags */
#include <asm/uaccess.h>    /* copy_*_use */

#include "scull.h"

/* Our paramters which can be set at load time */

int scull_major     = SCULL_MAJOR;
int scull_minor     = 0;
int scull_nr_devs   = SCULL_NR_DEVS; /* number of bare scull devices */
int scull_quantum   = SCULL_QUANTUM;
int scull_qset      = SCULL_QSET;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int , S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

struct scull_dev *scull_devices; /* allocate in scull_init_module */

/* The ioctl() implementation */
int scull_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int err = 0, tmp;
    int retval;

    /**
     * extract the type and number bitfields, and don't decode
     * wrong cmds: return ENOTTY(in anappropriate inctl) before access_ok()
     */
    if (_IOC_TYPE(cmd) != SCULL_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > SCULL_IOC_MAXNR) return -ENOTTY;

    /**
     * the direction is bitmask, and VERIFY_WRITE catches R/W
     * transfers, 'Type' is user-oriented, while access_ok if
     * kernel-oriented, so the concept of "read" and "write"
     * is reversed
     */

    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok_wrapper(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok_wrapper(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd))

    if (err) return -EFAULT;

    switch (cmd) {

    case SCULL_IOCRESET:
        scull_quantum = SCULL_QUANTUM;
        scull_qset = SCULL_QSET;
        break;

    case SCULL_IOCSQUANTUM: /* Set: arg point to the value */
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        retval = __get_user(scull_quantum, (int __user *)arg);
        break;

    case SCULL_IOCTQUANTUM: /* Tell: arg is the value */
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        scull_quantum = arg;
        break;

    case SCULL_IOCGQUANTUM: /* Get: arg is point to the value */
        retval = __put_user(scull_quantum, (int __user *)arg)

    case SCULL_IOCQQUANTUM: /* Query: return it (it is positive) */
        return scull_quantum;

    case SCULL_IOCXQUANTUM: /* eXchange: use arg as a pointer */
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        tmp = scull_quantum;
        retval = __get_user(scull_quantum, (int __user *)arg);
        if (retval == 0)
            retval = __put_user(tmp, (int __user *)arg);
        break;
    case SCULL_IOCHQUANTUM: /*sHift: like tell and query*/
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        tmp = scull_quantum;
        scull_quantum = arg;
        return tmp;

    case SCULL_IOCSQSET:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        retval =  __get_user(scull_qset, (int __user *)arg);
        break;

    case SCULL_IOCTQSET:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        scull_qset = arg;
        break;

    case SCULL_IOCGQSET:
        retval = __put_user(scull_qset, (int __user *)arg);
        break;

    case SCULL_IOCQQSET:
        return scull_qset;

    case SCULL_IOCXQSET:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        tmp = scull_qset;
        retval = __get_user(scull_qset, (int __user *)arg);
        if (retval == 0)
            retval = put_user(tmp, (int __user *)arg);
        break;

    case SCULL_IOCHQSET:
        if (!capable(CAP_SYS_ADMIN))
            return -EPERM;
        tmp = scull_qset;
        scull_qset = arg;
        return tmp;

    /**
     * The following two change the buffer size for scullpipe.
     * The scullpipe devie uses this same ioctl method, just to
     * wirte less code. Actually, it's the same driver, isn't it?
     */
    case SCULL_P_IOCTSIZE:
        scull_p_buffer = arg;
        break;

    case SCULL_P_IOCQSIZE:
        return scull_p_buffer;

    default:
        return -ENOTTY;
    }
    return retval;
}

loff_t scull_llseek(struct *filp, loff_t off, int whence)
{
    struct scull_dev *dev = filp->private_data;
    loff_t newpos;

    switch (whence)
    {
    case 0: /* SEEK_SET */
        newpos = off;
        break;
    case 1: /* SEEK_CUR */
        newpos = filp->f_pos + off;
        break;
    case 2: /* SEEK_END */
        newpos = dev->size + off;
    default: /* can't happen */
        return -EINVAL;
    }

    if (newpos < 0) return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}

struct file_operations scull_fops = {
    .owner =    THIS_MODULE,
    .llseek =   scull_llseek,
    .read =     scull_read,
    .write =    scull_write,
    .ioctl =    scull_ioctl,
    .open =     scull_open,
    .release =  scull_release,
}
/* Finally, the module stuff */

/**
 * The cleanup function is used to handle intialization failures as well
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
static void __exit scull_cleanup_module(void)
{
    int i;
    dev_t devno = MKDEV(scull_major, scull_minor);

    /* Get rid of our char dev entries */
    if (scull_devices) {
        for (i = 0; i < scull_nr_devs; i++) {
            scull_trim(scull_devices + i);
            cdev_del(&scull_devices[i].cdev);
        }
        kfree(scull_devices);
    }

#ifdef SCULL_DEBUG /* use proc only if debugging */
    scull_remove_proc();
#endif

    /* cleaup_module is never called if registering failed */
    unregister_chrdev_region(devno, scull_nr_devs);

    /* and call the cleanup functions for friend devices */
    scull_p_cleanup();
    scull_access_cleanup();
}

/* Set up the char_dev structure for this devices */
static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err, devno;
    devno = MKDEV(scull_major, scull_minor + index);

    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add(&dev->cdev, devno, 1);

    /* Fail gracefully if need be */
    if (err)
        printk(KERN_NOTICE "Error %d adding scull %d", err, index);
}

static int __init scull_init_module(void)
{
    int result, i;
    dev_t dev = 0;

    /**
     * Get a range of minor numbers to work with, asking for dynamic
     * major unless directed otherwise at load time
     */
    if (scull_major) { /* specified the major number */
        dev = MKDEV(scull_major, scull_minor);
        result = register_chardev_region(dev, scull_nr_devs, "scull");
    } else {
        result = alloc_chrdev_region(&dev, scull_minor, scull_nr_devs, "scnull");
        scull_major = MAJOR(dev);
    }

    if(result < 0) { /* a negative return represent an error */
        printk(KERN_WARNNING "scull: can't get major %d\n", scull_major);
        return result;
    }

    /**
     * allocate the devices -- we can't have them static, as the number
     * can be specified at load time
     */
    scull_devices = kmalloc(scull_nr_devs * sizeof(struct scull_dev), GFP_KERNEL);
    if (!scull_devices) {
        result = -ENOMEM;
        goto fail; /*Make this more graceful */
    }
    memset(scull_devices, 0 , scull_nr_devs * sizeof(struct scull_dev));

    /* Initialize each device */
    for (i = 0; i < scull_nr_devs; i++) {
        scull_devices[i].quantum = scull_quantum;
        scull_devices[i].qset = scull_qset;
        init_MUTEX(&scull_devices[i].sem);
        scull_setup_cdev(&scull_devices[i], i);
    }

    /* At this point call the init function for any friend device */
    dev = MKDEV(scull_major, scull_minor + scull_nr_devs);
    dev += scull_p_init(dev);
    dev += scull_access_init(dev);

#ifdef SCULL_DEBUG /* only when debugging */
    scull_create_proc();
#endif

    return 0; /* succeed */

fail:
    scull_cleaup_module();
    return result;
}

module_init(scull_init_module);
module_exit(scull_cleanup_module);