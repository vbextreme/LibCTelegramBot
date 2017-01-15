#ifndef _STDEXTRA_H_INCLUDE_
#define _STDEXTRA_H_INCLUDE_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio_ext.h>
#include <ctype.h>
#include <memory.h>
#include <time.h>
#include <utime.h>
#include <errno.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <sched.h>
#include <pthread.h>
#include <pwd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <linux/input.h>

#define HUMAN_VISION 300

typedef enum {FALSE, TRUE} bool_t;
typedef char char_t;
typedef int int_t;
typedef unsigned int uint_t;
typedef int_t err_t;
typedef long long_t;
typedef unsigned long ulong_t;
typedef uint_t utf8_t;
typedef FILE file_h;




#define __private static

#define __packed __attribute__((packed))

#define NEW(T) (T*)malloc(sizeof(T))
#define NEWS(T,S) (T*)malloc(sizeof(T) * (S))
#define FREE(T) do{ if (T) { free(T); T = NULL; }} while(0)

#ifdef DEBUG_ENABLE
	#define dbg(FORMAT, arg...) do{fprintf(stderr, "%s[%u]:{%d} %s(): " FORMAT "\n", __FILE__, __LINE__, (int_t)pthread_self(), __FUNCTION__, ## arg); fflush(stderr);}while(0)
#else
	#define dbg(FORMAT, arg...) do{}while(0)
#endif

#if ASSERT_ENABLE
	#define iassert(C) do{ if ( !(C) ){fprintf(stderr,"assertion fail %s[%u]: %s\n", __FILE__, __LINE__, #C); exit(0);}}while(0)
#else
	#define iassert(C) do{}while(0)
#endif

#endif
