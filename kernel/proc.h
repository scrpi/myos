#ifndef __KERNEL_PROC_H_
#define __KERNEL_PROC_H_

#include <uthash.h>

enum procstate {
	P_EMBRYO,
	P_SLEEPING,
	P_RUNNABLE,
	P_RUNNING,
	P_ZOMBIE
};

struct proc {
	char name[16];
	enum procstate state;
	int pid;

	UT_hash_handle hh;
};

