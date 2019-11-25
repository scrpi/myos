#define LACKS_SYS_TYPES_H
#define LACKS_ERRNO_H
#define LACKS_TIME_H
#define LACKS_SYS_MMAN_H
#define LACKS_FCNTL_H
#define LACKS_UNISTD_H
#define LACKS_SYS_PARAM_H

#define NO_MALLOC_STATS 1
#define HAVE_MORECORE 1
#define HAVE_MMAP 0
#define USE_LOCKS 0

#define MALLOC_FAILURE_ACTION do {} while (0);

#define DEBUG 1

#define ENOMEM 12
#define EINVAL 22

#include <stddef.h>
#include <alloc.h>

#include "dlmalloc.c"
