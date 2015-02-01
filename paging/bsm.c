/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bsm_tab[MAX_ID];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	//kprintf("\n Initializing backing store!");
	int i;
	for(i=0; i<= MAX_ID; i++)
	{
		bsm_tab[i].bs_bsid = i;
		bsm_tab[i].bs_status = BSM_UNMAPPED;
		bsm_tab[i].bs_pid = 0;
		bsm_tab[i].bs_vpno = 0;
		bsm_tab[i].bs_npages = BACKING_STORE_UNIT_SIZE/NBPG;
		bsm_tab[i].bs_sem = 0;
		bsm_tab[i].next = NULL;
		bsm_tab[i].bs_heap = 0;
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail)
{
	int x = *avail, i;
	if(x >= BACKING_STORE_UNIT_SIZE/NBPG)
		return SYSERR;
	for(i=0; i<=MAX_ID; i++)
	{
		if(bsm_tab[i].bs_status == BSM_UNMAPPED)
		{
			*avail = i;
			return OK;
		}
	}
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	bs_map_t *bs_map;
	bs_map = &bsm_tab[i];
	bs_map->bs_bsid = i;
	bs_map->bs_status = BSM_UNMAPPED;
	bs_map->bs_pid = 0;
	bs_map->bs_vpno = 0;
	bs_map->bs_npages = BACKING_STORE_UNIT_SIZE/NBPG;
	bs_map->bs_sem = 0;
	bs_map->next = NULL;
	bs_map->bs_heap = 0;
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	STATWORD ps;
	int i;
	bs_map_t *bs_map, *tmp;
	disable(ps);
	for(i=0; i<MAX_ID; i++)
	{
		bs_map = &bsm_tab[i];
		if(bs_map->bs_status == BSM_UNMAPPED)
			continue;
		tmp = bs_map->next;
		while(tmp != NULL)
		{
			if(tmp->bs_pid == pid && (vaddr/NBPG) >= tmp->bs_vpno && (vaddr/NBPG) < (tmp->bs_vpno + tmp->bs_npages))
			{
				*store = i;
				*pageth = tmp->bs_vpno;
				restore(ps);
				return OK;
			}
			tmp = tmp->next;
		}
	}
	restore(ps);
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages)
{
	bs_map_t *optr, *nptr;
	optr = &bsm_tab[source];
	nptr = (bs_map_t*) getmem(sizeof(bs_map_t));
	if(!nptr)
		return SYSERR;
	nptr->bs_bsid = source;
	nptr->bs_status = BSM_MAPPED;
	nptr->bs_pid = pid;
	nptr->bs_vpno = vpno;
	nptr->bs_npages = npages;
	nptr->next = optr->next;
	optr->next = nptr;
	//kprintf("\n BSM Mapping added with vpno %d", nptr->bs_vpno);
	return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	STATWORD ps;
	int i;
	bs_map_t *bs_map, *tmp, *prev;
	disable(ps);
	for(i=0; i<MAX_ID; i++)
	{
		bs_map = &bsm_tab[i];
		if(bs_map->bs_status == BSM_UNMAPPED)
			continue;
		prev = bs_map;
		tmp = bs_map->next;
		while(tmp != NULL)
		{
			if(tmp->bs_pid = pid && vpno >= tmp->bs_vpno && vpno < (tmp->bs_vpno + tmp->bs_npages))
			{
				if(vpno == tmp->bs_vpno)
				{
					prev->next = tmp->next;
					freemem((struct mblock*) tmp, sizeof(bs_map));
				}
				else
					tmp->bs_npages = tmp->bs_npages - vpno;
				restore(ps);
				return OK;
			}
			prev = tmp;
			tmp = tmp->next;
		}
	}
	restore(ps);
	return SYSERR;
}

int killProc(int pid)
{
	int i, pdbr, frame;
	bs_map_t *bs_map, *prev, *tmp;
	STATWORD ps;
	disable(ps);
	for(i=0; i<MAX_ID; i++)
	{
		bs_map = &bsm_tab[i];
		if(bs_map->bs_status == BSM_UNMAPPED)
			continue;
		prev = bs_map;
		tmp = bs_map->next;
		while(tmp != NULL)
		{
			if(tmp->bs_pid == pid)
				xmunmap(tmp->bs_vpno);
			prev = tmp;
			tmp = tmp->next;
		}
	}
	pdbr = &proctab[pid].pdbr;
	frame = (pdbr/NBPG) - FRAME0;
	free_frm(frame);
	restore(ps);
	return OK;
}
