/* xm.c = xmmap xmunmap */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>


/*-------------------------------------------------------------------------
 * xmmap - xmmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmmap(int virtpage, bsd_t source, int npages)
{
  /* sanity check ! */

  if ( (virtpage < 4096) || ( source < 0 ) || ( source > MAX_ID) ||(npages < 1) || ( npages >128)){
	kprintf("xmmap call error: parameter error! \n");
	return SYSERR;
  }
	STATWORD ps;
	bs_map_t *bs_map;
	disable(ps);
	bs_map = &bsm_tab[source];
	if(bs_map->bs_status == BSM_UNMAPPED)
	{
		restore(ps);
		return SYSERR;
	}
	if(bs_map->bs_heap == 1)
	{
		restore(ps);
		return SYSERR;
	}
	if(bsm_map(currpid, virtpage, bs_map->bs_bsid, npages) == SYSERR)
	{
		restore(ps);
		return SYSERR;
	}
	restore(ps);
	return OK;
}



/*-------------------------------------------------------------------------
 * xmunmap - xmunmap
 *-------------------------------------------------------------------------
 */
SYSCALL xmunmap(int virtpage )
{
  /* sanity check ! */
  if ( (virtpage < 4096) ){ 
	kprintf("xmummap call error: virtpage (%d) invalid! \n", virtpage);
	return SYSERR;
  }
	STATWORD ps;
	bs_map_t *bs_map;
	int store, pageth, diff, flag;
	disable(ps);
	char *vaddr = (char*) (virtpage << 12);
	if(bsm_lookup(currpid, vaddr, &store, &pageth) == SYSERR)
	{
		kprintf("\n Backing store mapping not present!");
		restore(ps);
		return SYSERR;
	}
	//kprintf("\n BSM ID %d found!", store);
	diff = virtpage - pageth;
	free_frm2(store, diff);
	bsm_unmap(currpid, virtpage, flag);
	restore(ps);
  return OK;
}

