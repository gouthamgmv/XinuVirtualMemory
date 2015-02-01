/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	vblock	*block;
	unsigned size;
{
	STATWORD ps;
	struct vblock *i, *j;
	unsigned k;
	struct pentry *p;
	if(size == 0 || (unsigned) block > (unsigned) maxaddr || (unsigned) block < ((unsigned) &end))
		return SYSERR;
	size = (unsigned) roundmb(size);
	disable(ps);
	p = &proctab[currpid];
	for(i=p->vlist->next, j=&(p->vlist); i!=(struct vblock*) NULL && i<block; j=i, i=i->next);
	if(((k = j->len + (unsigned) j) > (unsigned) block && j != &(p->vlist)) || (i != NULL && (size + (unsigned) block) > (unsigned) i))
	{
		restore(ps);
		return SYSERR;
	}
	if(j != &(p->vlist) && k == (unsigned) block)
		j->len = j->len + size;
	else
	{
		block->len = size;
		block->next = i;
		j->next = block;
		j = block;
	}
	if((unsigned) (j->len + (unsigned) j) == (unsigned) i)
	{
		j->len += i->len;
		j->next = i->next;
	}
	restore(ps);
	return(OK);
}
