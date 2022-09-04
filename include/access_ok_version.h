/*
 * @file access_ok_version.h
 * @date 2022-05-13
 * just copy from github
 */
#ifndef _ACCESS_OK_VERSION
#define _ACCESS_OK_VERSION

#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)
#define access_ok_wrapper(type,arg,cmd) \
    access_ok(type, arg, cmd)
#else
#define access_ok_wrapper(type,arg,cmd) \
    access_ok(arg, cmd)
#endif

#endif /*_ACCESS_OK_VERSION */