#ifndef __KERNEL_PROC_H_
#define __KERNEL_PROC_H_

#include <mm/mmu.h>
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

	struct pd *pgdir;
	phys_addr_t pgdir_phys;

	phys_addr_t kstack_phys;

	UT_hash_handle hh;
};


void proc_userinit();


#endif
