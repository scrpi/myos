#include <proc.h>
#include <stdint.h>

static struct proc *proc_table;

static uint16_t next_pid = 1;

static struct proc *proc_alloc()
{
	struct proc *p;

	p = malloc(sizeof(*p));
	if (!p)
		return NULL;

	memset(p, 0, sizeof(*p));

	p->state = P_EMBRYO;
	p->pid = next_pid++;

	HASH_ADD_INT(proc_table, p->pid, p);

	return p;
}
