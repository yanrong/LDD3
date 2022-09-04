/*
 * scull.h -- definitions for the char module
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
 *
 * $Id: scull.h,v 1.15 2004/11/04 17:51:18 rubini Exp $
 *
 */

#ifndef _SCULLC_H_
#define _SCULLC_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/cdev.h>

/* Marcos to help debugging */

#undef PDEBUG /* undef it, just in case */
#ifdef SCULLC_DEBUG
#   ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#       define PDEBUG(fmt, args...) printk(KERN_DEBUG "scullc :" fmt, ##args)
#   else
/* This one for use space */
#       define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#   endif
#else
#   define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)  /* nothing: it's a placeholder */

#ifndef SCULLC_MAJOR
#define SCULLC_MAJOR 0 /* dynamic major by default */
#endif

#ifndef SCULLC_DEVS
#define SCULLC_DEVS 4 /* scullc0 through scull3 */
#endif

/*
 * The bare device is a variable-length region of memory
 * Use a linked list of indirect blocks
 *
 * "scullc_dev->data" points to an array of pointers, each
 * pointer refers to a memory page
 * The array (quantum-set) is SCULL_QSET long.
 */

#ifndef SCULLC_QUANTUM
#define SCULLC_QUANTUM 4000
#endif

#ifndef SCULLC_QSET
#define SCULLC_QSET 500
#endif

struct scullc_dev {
    void **data;
    struct scullc_dev *next;    /* next listitem */
    int vmas;                   /* active mappings */
    int quantum;                /* the current allocation size */
    int qset;                   /* the current array size */
    size_t size;                /* amount of data stored here */
    struct semaphore sem;       /* mutual exclusion semaphore */
    struct cdev cdev;           /* Char device structure */
};

extern struct scullc_dev *scullc_devices;
extern struct file_operations scullc_fops;

/* The different configurable parameters */
extern int scullc_major; /* main.c */
extern int scullc_devs;
extern int scullc_order;
extern int scullc_qset;

/* Prototypes for shared functions */
int scullc_trim(struct scullc_dev *dev);
struct scullc_dev *scullc_follow(struct scullc_dev *dev, int n);

#ifdef SCULLC_DEBUG
#define SCULLC_USE_PROC
#endif

/* Ioctl definitions */

/* Use 'k' as magic number */
#define SCULLC_IOC_MAGIC 'K'
/* Please use a different 8-bit number in your code */

#define SCULLC_IOCRESET _IO(SCULLC_IOC_MAGIC, 0)

/*
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 */
#define SCULLC_IOCSQUANTUM   _IOW(SCULLC_IOC_MAGIC,     1, int)
#define SCULLC_IOCTQUANTUM   _IO(SCULLC_IOC_MAGIC,      2)
#define SCULLC_IOCGQUANTUM   _IOR(SCULLC_IOC_MAGIC,     3, int)
#define SCULLC_IOCQQUANTUM   _IO(SCULLC_IOC_MAGIC,      4)
#define SCULLC_IOCXQUANTUM   _IOWR(SCULLC_IOC_MAGIC,    5, int)
#define SCULLC_IOCHQUANTUM   _IO(SCULLC_IOC_MAGIC,      6)
#define SCULLC_IOCSQSET      _IOW(SCULLC_IOC_MAGIC,     7, int)
#define SCULLC_IOCTQSET      _IO(SCULLC_IOC_MAGIC,      8)
#define SCULLC_IOCGQSET      _IOR(SCULLC_IOC_MAGIC,     9, int)
#define SCULLC_IOCQQSET      _IO(SCULLC_IOC_MAGIC,      10)
#define SCULLC_IOCXQSET      _IOWR(SCULLC_IOC_MAGIC,    11, int)
#define SCULLC_IOCHQSET      _IO(SCULLC_IOC_MAGIC,      12)

#define SCULLC_IOC_MAXNR 12

#endif /* _SCULLC_H_ */
