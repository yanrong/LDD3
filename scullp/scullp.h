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

#ifndef _SCULLP_H_
#define _SCULLP_H_

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */
#include <linux/cdev.h>
#include <linux/semaphore.h>

/* Marcos to help debugging */

#undef PDEBUG /* undef it, just in case */
#ifdef SCULLP_DEBUG
#   ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#       define PDEBUG(fmt, args...) printk(KERN_DEBUG "scullp :" fmt, ##args)
#   else
/* This one for use space */
#       define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#   endif
#else
#   define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...)  /* nothing: it's a placeholder */

#define SCULLP_MAJOR 0 /* dynamic major by default */

#define SCULLP_DEVS 4 /* scullp0 through scull3 */

/**
 * The bare device is a variable-length region of memory
 * Use a linked list of indirect blocks
 *
 * "scullp_dev->data" points to an array of pointers, each
 * pointer refers to a memory page
 * The array (quantum-set) is SCULL_QSET long.
 */
#define SCULLP_ORDER    0 /* one page at a time */
#define SCULLP_QSET     500

struct scullp_dev {
    void **data;
    struct scullp_dev *next;    /* next listitem */
    int vmas;                   /* active mappings */
    int order;                  /* the current allocation oreer */
    int qset;                   /* the current array size */
    size_t size;                /* amount of data stored here */
    //struct semaphore sem;       /* mutual exclusion semaphore */
    struct mutex mutex;       /* mutual exclusion semaphore */
    struct cdev cdev;           /* Char device structure */
};

extern struct scullp_dev *scullp_devices;
extern struct file_operations scullp_fops;

/* The different configurable parameters */
extern int scullp_major; /* main.c */
extern int scullp_devs;
extern int scullp_order;
extern int scullp_qset;

/* Prototypes for shared functions */
int scullp_trim(struct scullp_dev *dev);
struct scullp_dev *scullp_follow(struct scullp_dev *dev, int n);

#ifdef SCULLP_DEBUG
#define SCULLP_USE_PROC
#endif

/* Ioctl definitions */

/* Use 'k' as magic number */
#define SCULLP_IOC_MAGIC 'K'
/* Please use a different 8-bit number in your code */

#define SCULLP_IOCRESET _IO(SCULLP_IOC_MAGIC, 0)

/**
 * S means "Set" through a ptr,
 * T means "Tell" directly with the argument value
 * G means "Get": reply by setting through a pointer
 * Q means "Query": response is on the return value
 * X means "eXchange": switch G and S atomically
 * H means "sHift": switch T and Q atomically
 */
#define SCULLP_IOCSORDER   _IOW(SCULLP_IOC_MAGIC,     1, int)
#define SCULLP_IOCTORDER   _IO(SCULLP_IOC_MAGIC,      2)
#define SCULLP_IOCGORDER   _IOR(SCULLP_IOC_MAGIC,     3, int)
#define SCULLP_IOCQORDER   _IO(SCULLP_IOC_MAGIC,      4)
#define SCULLP_IOCXORDER   _IOWR(SCULLP_IOC_MAGIC,    5, int)
#define SCULLP_IOCHORDER   _IO(SCULLP_IOC_MAGIC,      6)
#define SCULLP_IOCSQSET    _IOW(SCULLP_IOC_MAGIC,     7, int)
#define SCULLP_IOCTQSET    _IO(SCULLP_IOC_MAGIC,      8)
#define SCULLP_IOCGQSET    _IOR(SCULLP_IOC_MAGIC,     9, int)
#define SCULLP_IOCQQSET    _IO(SCULLP_IOC_MAGIC,      10)
#define SCULLP_IOCXQSET    _IOWR(SCULLP_IOC_MAGIC,    11, int)
#define SCULLP_IOCHQSET    _IO(SCULLP_IOC_MAGIC,      12)

#define SCULLP_IOC_MAXNR 12

#endif /* _SCULLP_H_ */
