#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

int get_bs(bsd_t bs_id, unsigned int npages) {
	if( bs_id < 0 || bs_id > MAX_ID || npages < 1 || npages > BACKING_STORE_UNIT_SIZE/NBPG)
	{
		kprintf("\n Error in parameters!");
		return SYSERR;
	}
	STATWORD ps;
	bs_map_t *bs_map;
	bs_map = &bsm_tab[bs_id];
	if(bs_map->bs_heap == 1)
	{
		restore(ps);
		return SYSERR;
	}
	disable(ps);
	if(bs_map->bs_status == BSM_MAPPED)
	{
		restore(ps);
		return bs_map->bs_npages;
	}
	if(bs_map->bs_status == BSM_UNMAPPED)
	{
		bs_map->bs_status = BSM_MAPPED;
		bs_map->bs_npages = npages;
		bs_map->bs_pid = currpid;
		restore(ps);
		return npages;
	}
	restore(ps);
   return SYSERR;
}


