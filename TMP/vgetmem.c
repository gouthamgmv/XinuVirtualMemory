/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;
	struct vblock *i, *j, *k;
	struct pentry *p;
	disable(ps);
	p = &proctab[currpid];
	if(nbytes == 0 || (p->vlist->next) == (struct vblock*) NULL)
	{
		restore(ps);
		return((WORD*) SYSERR);
	}
	nbytes = (unsigned int) roundmb (nbytes);
	for(j = &(p->vlist), i = p->vlist->next; i != (struct vblock*) NULL; j = i, j = j->next)
	{
		if(i->len == nbytes)
		{
			j->next = i->next;
			restore(ps);
			return((WORD *)i);
		}
		else if(i->len > nbytes)
		{
			k = (struct vblock*) ((unsigned) i + nbytes);
			j->next = k;
			k->next = i->next;
			k->len = i->len - nbytes;
			restore(ps);
			return((WORD *)i);
		}
		restore(ps);
		return((WORD*)SYSERR);
	}
	return( SYSERR );
}


