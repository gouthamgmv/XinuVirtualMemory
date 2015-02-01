/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <stdio.h>

fr_map_t frm_tab[NFRAMES];
//fr_frlist *fr, *endfr;
fr_list *curr;

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
	//kprintf("\n Initializing frames!");
	int i;
	for(i=0; i<NFRAMES; i++)
	{
		frm_tab[i].fr_status = FRM_UNMAPPED;
		frm_tab[i].fr_pid = 0;
		frm_tab[i].fr_vpno = 0;
		frm_tab[i].fr_refcnt = 0;
		frm_tab[i].fr_type = -1;
		frm_tab[i].fr_dirty = -1;
		frm_tab[i].fr_loadtime = 0;
		frm_tab[i].fr_timeCount = 0;
	}
	//fr = NULL;
	//endfr = NULL;
	curr = NULL;
	return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
	//kprintf("\n Searching for a free frame!");
	int i, frame = -1, j;
	for(i=0; i<NFRAMES; i++)
	{
		if((*avail == FR_TBL) || (*avail == FR_DIR))
		{
			if(i < (1535 - NFRAMES))
			{
				if(frm_tab[i].fr_status == FRM_UNMAPPED)
				{
					frm_tab[i].fr_status = FRM_MAPPED;
					frm_tab[i].fr_refcnt = 1;
					frame = i;
					//kprintf("\n Frame %d allocated to process with type %d", frame+FRAME0, *avail);
					*avail = frame;
					return OK;
				}
			}
		}
		else if(*avail == FR_PAGE)
		{
			if(frm_tab[i].fr_status == FRM_UNMAPPED)
			{
				frm_tab[i].fr_status = FRM_MAPPED;
				frm_tab[i].fr_refcnt = 1;
				timeCount++;
				frm_tab[i].fr_timeCount = timeCount;
				//kprintf("\n Time counts of %d is %d", i+FRAME0, timeCount);
				updateTimeCounts();
				frame = i;
				*avail = frame;
				insn(i);
				//kprintf("\n Frame %d allocated to page", frame+FRAME0);
				return OK;
			}
		}
	}
	if(*avail == FR_PAGE)
	{
		if(grpolicy() == FIFO)
			frame = FIFOReplace();
		else if(grpolicy() == LRU)
			frame = LRUReplace();
		frm_tab[frame].fr_status = FRM_MAPPED;
		frm_tab[frame].fr_refcnt = 1;
		timeCount++;
		updateTimeCounts();
		frm_tab[frame].fr_timeCount = timeCount;
		*avail = frame;
		insn(frame);
		return OK;
	}
	return SYSERR;
}


void updateTimeCounts()
{
	//kprintf("\n LOLtmp at least??");
	fr_map_t *frame;
	fr_list *temp = curr;
	STATWORD ps;
	int store, pageth, pid, i, j, k, count = 0;
	bs_map_t *bs_map, *tmp;
	struct pentry *p;
	pd_t *pd;
	pt_t *pt;
	disable(ps);
	while(temp)
	{
		//kprintf("\n LOLtmp??");
		//count++;
		//if(count == 1000)
			//return;
	//for(k=0; k<NFRAMES; k++)
	//{
		frame = &frm_tab[temp->fr_frid];
		if(bsm_lookup(frame->fr_pid, ((frame->fr_vpno)*NBPG), &store, &pageth) == SYSERR)
		{
			restore(ps);
			return SYSERR;
		}
		bs_map = &bsm_tab[store];
		tmp = bs_map->next;
		while(tmp != NULL)
		{
			pid = tmp->bs_pid;
			p = &proctab[pid];
			pd = p->pdbr;
			//kprintf("\n LOLtmp!");
			for(i=4; i<(NBPG/4); i++)
			{
				if(pd[i].pd_pres != 0)
				{
					pt = pd[i].pd_base * NBPG;
					for(j=0; j<(NBPG/4); j++)
					{
						if(pt[j].pt_pres != 0)
						{
							if(pt[j].pt_acc == 1)
							{
								//kprintf("\n %d was accessed!", pt[j].pt_base);
								pt[j].pt_acc = 0;
								frame->fr_timeCount = timeCount;
								/*for(k=0; k<NFRAMES; k++)
								{
									if(frame == &frm_tab[k])
										kprintf("\n We are in frame %d", k+FRAME0);
								}*/
								//kprintf("\n tc of %d is %d", pt[j].pt_base, timeCount);
								//kprintf("\n LOL!");
							}
						}
					}
				}
			}
			tmp = tmp->next;
		}
		temp = temp->next;
	}
	restore(ps);
}
void testframe()
{
	int i;
	fr_map_t *frame;
	for(i=0; i<NFRAMES; i++)
	{
		frame = &frm_tab[i];
		if(frame->fr_type == FR_PAGE);
			kprintf("\n Frame %d TC %d", i+FRAME0, frame->fr_timeCount);
	}
}
int LRUReplace()
{
	//kprintf("\n Here?");
	fr_map_t *frame;
	//fr_frlist *test = fr;
	STATWORD ps;
	int store, pageth, repfr, tc = 100, vp = 0, i;
	disable(ps);
	//while(test)
	for(i=0; i<NFRAMES; i++)
	{
		frame = &frm_tab[i];
		if(frame->fr_type == FR_PAGE)
		{
			//kprintf("\n %d frt %d tc %d", i+FRAME0, frame->fr_timeCount, tc);
			if(frame->fr_timeCount < tc)
			{
				tc = frame->fr_timeCount;
				repfr = i;
				if(bsm_lookup(frame->fr_pid, ((frame->fr_vpno)*NBPG), &store, &pageth) == SYSERR)
				{
					//kprintf("\n Here?");
					restore(ps);
					return SYSERR;
				}	
			}
			else if(frame->fr_timeCount == tc && frame->fr_vpno > vp)
			{
				vp = frame->fr_vpno;
				repfr = i;
				if(bsm_lookup(frame->fr_pid, ((frame->fr_vpno)*NBPG), &store, &pageth) == SYSERR)
				{
					//kprintf("\n Here?");
					restore(ps);
					return SYSERR;
				}
			}	
		}
	}
	write_bs((repfr+FRAME0)*NBPG, store, pageth);
	free_frm(repfr);
	kprintf("\n Frame %d is being replaced!", repfr+FRAME0);
	restore(ps);
	return repfr;
}

int FIFOReplace()
{
	fr_map_t *frame;
	STATWORD ps;
	int store, pageth, repfr;
	disable(ps);
	if(curr)
	{
		//kprintf("\n Here!");
		frame = &frm_tab[curr->fr_frid];
		repfr = curr->fr_frid;
		if(bsm_lookup(frame->fr_pid, ((frame->fr_vpno)*NBPG), &store, &pageth) == SYSERR)
		{
			//kprintf("\n Here?");
			restore(ps);
			return SYSERR;
		}
		//curr = curr->next;
		write_bs((repfr+FRAME0)*NBPG, store, pageth);
		free_frm(repfr);
		kprintf("\n Frame %d is being replaced!", repfr+FRAME0);
		restore(ps);
		return repfr;
	}
	restore(ps);
	return SYSERR;
}

void insn(int frame)
{
	//fr_frlist *head = addn(frame);
	fr_list *frhead = addn(frame);
	if(!curr)
	{
		curr = frhead;
		curr->next = NULL;
		//fr->prev = fr;
		//endfr = fr;
		return;
	}
	else
	{
		fr_list *tmp = curr;
		while(tmp->next != NULL)
			tmp = tmp->next;
		tmp->next = frhead;
	}
}

fr_list *addn(int frame)
{
	fr_list *head = getmem(sizeof(fr_list));
	head->fr_frid = frame;
	head->next = NULL;
	return head;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame 
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(int i)
{
	int store, pageth;
	if(i <= 0 || i > NFRAMES)
		return SYSERR;
	if(frm_tab[i].fr_type == FR_PAGE)
	{
		resetPT(i);
		if(grpolicy() == FIFO)
			curr = curr->next;
		else
			deln(i);
	}
	frm_tab[i].fr_status = FRM_UNMAPPED;
	frm_tab[i].fr_pid = 0;
	frm_tab[i].fr_vpno = 0;
	frm_tab[i].fr_refcnt = 0;
	frm_tab[i].fr_type = -1;
	frm_tab[i].fr_dirty = -1;
	frm_tab[i].fr_loadtime = 0;
	frm_tab[i].fr_timeCount = 0;
  return OK;
}

int free_frm2(int bs_bsid, int diff)
{
	bs_map_t *bs_map, *tmp;
	fr_map_t *fr_map;
	int i, j;
	bs_map = &bsm_tab[bs_bsid];
	tmp = bs_map->next;
	while(tmp != NULL)
	{
		for(i=0; i<NFRAMES; i++)
		{
			fr_map = &frm_tab[i];
			if(fr_map->fr_status == FRM_UNMAPPED || fr_map->fr_type != FR_PAGE)
				continue;
			if(tmp->bs_pid == fr_map->fr_pid && (fr_map->fr_vpno - tmp->bs_vpno) == diff)
			{
				for(j=0; j<tmp->bs_npages-diff; j++)
				{
					if((i+j+FRAME0) < 2048)
						reduceRef(i+j);
				}
			}
		}
		tmp = tmp->next;
	}
	return -1;
}

int frame_lookup(int store, int diff)
{
	bs_map_t *bs_map, *tmp;
	fr_map_t *fr;
	int i;
	bs_map = &bsm_tab[store];
	tmp = bs_map->next;
	while(tmp != NULL)
	{
		for(i=0; i<NFRAMES; i++)
		{
			fr = &frm_tab[i];
			if(fr->fr_status == FRM_UNMAPPED || fr->fr_type != FR_PAGE)
				continue;
			if(tmp->bs_pid == fr->fr_pid && ((fr->fr_vpno - tmp->bs_vpno) == diff))
				return i;
		}
		tmp = tmp->next;
	}
	return -1;
}

int reduceRef(int frame)
{
	fr_map_t *fr_map;
	fr_map = &frm_tab[frame];
	if(fr_map->fr_status != FRM_UNMAPPED)
	{
		if(fr_map->fr_refcnt > 0)
			fr_map->fr_refcnt--;
		if(fr_map->fr_refcnt == 0)
			free_frm(frame);
	}
	return OK;
}

void deln(int frame)
{
	fr_list *head, *tmp = curr;
	if(curr == NULL)
		return;
	else
	{
		while(tmp->next->fr_frid != frame)
		{
			tmp = tmp->next;
		}
		tmp->next = tmp->next->next;
		//freemem((struct mblock*) tmp, sizeof(fr_frlist));
	}
}	