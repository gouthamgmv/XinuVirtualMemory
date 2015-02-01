#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

SYSCALL release_bs(bsd_t bs_id) {
	STATWORD ps;
	disable(ps);
	bs_map_t *bs_map, *tmp;
	bs_map = &bsm_tab[bs_id];
	tmp = bs_map->next;
	if(tmp != NULL)
		return OK;
	free_bsm(bs_id);
	restore(ps);
   return OK;
}

