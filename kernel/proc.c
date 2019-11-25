#include <proc.h>
#include <kernel.h>
#include <stdint.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

static struct proc *proc_table;

static uint16_t next_pid = 1;

static inline void find_next_pid()
{
	struct proc *tmp;

	do {
		if (next_pid++ <= 1)
			next_pid = 2;

		HASH_FIND_INT(proc_table, &next_pid, tmp);
	} while (tmp);
}

static struct proc *proc_alloc()
{
	struct proc *p;

	p = malloc(sizeof(*p));
	if (!p)
		return NULL;

	memset(p, 0, sizeof(*p));

	p->state = P_EMBRYO;
	p->pid = next_pid;
	p->kstack_phys = pmm_alloc();

	HASH_ADD_INT(proc_table, pid, p);

	find_next_pid();

	return p;
}

void proc_userinit()
{
	struct proc *init;

	init = proc_alloc();

	init->pgdir_phys = pmm_alloc();
	if (!init->pgdir_phys)
		panic("%s: OOM\n", __func__);

	init->pgdir = vmm_create_pd(init->pgdir_phys);
}
