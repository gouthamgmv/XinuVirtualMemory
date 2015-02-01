/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{
	STATWORD ps;
	int pid, *size;
	bs_map_t *bs_map;
	disable(ps);
	*size = hsize;
	if(get_bsm(size) == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	pid = create(procaddr, ssize, hsize, priority, name, nargs, args);
	bs_map = &bsm_tab[*size];
	bs_map->bs_status = BSM_MAPPED;
	bs_map->bs_npages = hsize;
	bs_map->bs_heap = 1;
	if(bsm_map(pid, 4096, bs_map->bs_bsid, hsize) == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	proctab[pid].store = bs_map->bs_bsid;
	proctab[pid].vhpno = 4096;
	proctab[pid].vhpnpages = hsize;
	proctab[pid].vlist = (struct vblock*) getmem(sizeof(struct vblock));
	struct vblock *nextvb = getmem(sizeof(struct vblock));
	nextvb->add = 4096*4096;
	nextvb->len = hsize*4096;
	nextvb->next = (struct vblock*) NULL;
	proctab[pid].vlist->next = (struct vblock*) nextvb;
	restore(ps);
	return pid;
}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
