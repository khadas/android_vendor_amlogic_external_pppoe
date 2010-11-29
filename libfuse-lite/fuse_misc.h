/*
    FUSE: Filesystem in Userspace
    Copyright (C) 2001-2007  Miklos Szeredi <miklos@szeredi.hu>

    This program can be distributed under the terms of the GNU LGPLv2.
    See the file COPYING.LIB
*/

#include "config.h"
#include <pthread.h>

#ifndef USE_UCLIBC
#define fuse_mutex_init(mut) pthread_mutex_init(mut, NULL)
#else
static inline void fuse_mutex_init(pthread_mutex_t *mut)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
    pthread_mutex_init(mut, &attr);
    pthread_mutexattr_destroy(&attr);
}
#endif

typedef struct _pthread_rwlock_t
{
//  struct _pthread_fastlock __rw_lock; /* Lock to guarantee mutual exclusion */
  int __rw_readers;                   /* Number of readers */
//  _pthread_descr __rw_writer;         /* Identity of writer, or NULL if none */
//  _pthread_descr __rw_read_waiting;   /* Threads waiting for reading */
//  _pthread_descr __rw_write_waiting;  /* Threads waiting for writing */
  int __rw_kind;                      /* Reader/Writer preference selection */
  int __rw_pshared;                   /* Shared between processes or not */
} pthread_rwlock_t;

typedef struct
{
  int __lockkind;
  int __pshared;
} pthread_rwlockattr_t;


static inline int pthread_rwlock_init (pthread_rwlock_t * __rwlock,
				 pthread_rwlockattr_t *__attr)
{
		return 1;
}

static inline  int pthread_rwlock_destroy (pthread_rwlock_t *__rwlock)
{
		return 1;
}



/* Acquire read lock for RWLOCK.  */
static inline  int pthread_rwlock_rdlock (pthread_rwlock_t *__rwlock) 
{
	return 1;
}


/* Acquire write lock for RWLOCK.  */
static inline  int pthread_rwlock_wrlock (pthread_rwlock_t *__rwlock)
{
	return 1;
}




/* Unlock RWLOCK.  */
static inline  int pthread_rwlock_unlock (pthread_rwlock_t *__rwlock)
{
	return 1;
}



#ifdef HAVE_STRUCT_STAT_ST_ATIM
/* Linux */
#define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atim.tv_nsec)
#define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctim.tv_nsec)
#define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtim.tv_nsec)
#define ST_ATIM_NSEC_SET(stbuf, val) (stbuf)->st_atim.tv_nsec = (val)
#define ST_MTIM_NSEC_SET(stbuf, val) (stbuf)->st_mtim.tv_nsec = (val)
#elif defined(HAVE_STRUCT_STAT_ST_ATIMESPEC)
/* FreeBSD */
#define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atimespec.tv_nsec)
#define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctimespec.tv_nsec)
#define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtimespec.tv_nsec)
#define ST_ATIM_NSEC_SET(stbuf, val) (stbuf)->st_atimespec.tv_nsec = (val)
#define ST_MTIM_NSEC_SET(stbuf, val) (stbuf)->st_mtimespec.tv_nsec = (val)
#elif defined(HAVE_STRUCT_STAT_ST_ATIMENSEC)
#define ST_ATIM_NSEC(stbuf) ((stbuf)->st_atimensec)
#define ST_CTIM_NSEC(stbuf) ((stbuf)->st_ctimensec)
#define ST_MTIM_NSEC(stbuf) ((stbuf)->st_mtimensec)
#define ST_ATIM_NSEC_SET(stbuf, val) (stbuf)->st_atimensec = (val)
#define ST_MTIM_NSEC_SET(stbuf, val) (stbuf)->st_mtimensec = (val)
#else
#define ST_ATIM_NSEC(stbuf) 0
#define ST_CTIM_NSEC(stbuf) 0
#define ST_MTIM_NSEC(stbuf) 0
#define ST_ATIM_NSEC_SET(stbuf, val) do { } while (0)
#define ST_MTIM_NSEC_SET(stbuf, val) do { } while (0)
#endif
