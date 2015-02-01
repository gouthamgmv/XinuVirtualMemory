/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>
#include <stdio.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint()
{
	 STATWORD ps;
	 unsigned long cr2;
	 virt_addr_t *virt_vaddr;
	 unsigned int pd_diff, pt_diff, pg_diff;
	 struct pentry *p;
	 pd_t *pageDir;
	 pt_t *pageTab;
	 int store, pageth, frame, diff;
	 bs_map_t *bs_map;
	 disable(ps);
	 cr2 = read_cr2();
	 virt_vaddr = (virt_addr_t*) cr2;
	 pd_diff = cr2 >> 22;
	 pt_diff = cr2 >> 12 & 1023;
	 pg_diff = cr2 & 1023;
	 p = &proctab[currpid];
	 pageDir = p->pdbr;
	 if(pageDir[pd_diff].pd_pres != 1)
	 {
		pageTab = createPageTable();
		pageDir[pd_diff].pd_pres = 1;
		pageDir[pd_diff].pd_write = 1;
		pageDir[pd_diff].pd_base = ((unsigned int) pageTab)/NBPG;
	 }
	 else
		pageTab = (pageDir[pd_diff].pd_base)*NBPG;
	if(bsm_lookup(currpid, virt_vaddr, &store, &pageth) == SYSERR)
	{
		kill(currpid);
		restore(ps);
		return SYSERR;
	}
	diff = (cr2/NBPG) - pageth;
	frame = frame_lookup(store, diff);
	if(frame == -1)
	{
		frame = FR_PAGE;
		get_frm(&frame);
		if(frame == -1)
		{
			kprintf("\n Frame allocation unsuccessful!");
			restore(ps);
			return OK;
		}
		frm_tab[frame].fr_pid = currpid;
		frm_tab[frame].fr_refcnt = 1;
		frm_tab[frame].fr_vpno = ((unsigned int) virt_vaddr)/NBPG;
		frm_tab[frame].fr_type = FR_PAGE;
		read_bs((frame+FRAME0)*NBPG, store, pageth);
	}
	else
	{
		frm_tab[frame].fr_refcnt++;
		/*timeCount++;
		frm_tab[frame].fr_timeCount = timeCount;
		kprintf("\n Time count of %d is %d", frame+FRAME0, timeCount);*/
	}
	pageTab[pt_diff].pt_pres = 1;
	pageTab[pt_diff].pt_write = 1;
	pageTab[pt_diff].pt_base = frame + FRAME0;
	pageTab[pt_diff].pt_acc = 1;
	restore(ps);
	return OK;
}


