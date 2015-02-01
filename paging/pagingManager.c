#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>
#include <stdio.h>
pt_t *pt_tab[4];
int init_pt()
{
	//kprintf("\n Initializing the first four page tables!");
	int i, j;
	pt_t *pageTable;
	for(i=0; i<4; i++)
	{
		pt_tab[i] = createPageTable();
		if(pt_tab[i] == NULL)
		{
			kprintf("\n Could not create page table!");
			return SYSERR;
		}
		pageTable = pt_tab[i];
		for(j=0; j<(NBPG/4); j++)
		{
			pageTable[j].pt_pres = 1;
			pageTable[j].pt_write = 1;
			pageTable[j].pt_base = (i*(NBPG/4))+j;
		}
	}
	//kprintf("\n First four page tables initialized!");
	return OK;
}
pt_t *createPageTable()
{
	int frame, i;
	frame = FR_TBL;
	get_frm(&frame);
	if(frame == -1)
		return NULL;
	pt_t *pageTab;
	frm_tab[frame].fr_type = FR_TBL;
	pageTab = (pt_t*)((frame+FRAME0)*NBPG);
	for(i=0; i<(NBPG/4); i++)
	{
		pageTab[i].pt_pres = 0;
		pageTab[i].pt_write = 0;
		pageTab[i].pt_user = 0;
		pageTab[i].pt_pwt = 0;
		pageTab[i].pt_pcd = 0;
		pageTab[i].pt_acc = 0;
		pageTab[i].pt_dirty = 0;
		pageTab[i].pt_mbz = 0;
		pageTab[i].pt_global = 0;
		pageTab[i].pt_avail = 0;
		pageTab[i].pt_base = 0;
	}
	return pageTab;
}
pd_t *init_pd(int pid)
{
	int i;
	pd_t *pageDir;
	pageDir = createPageDirectory(pid);
	if(pageDir == NULL)
	{
		kprintf("\n Could not create page directory!");
		return NULL;
	}
	for(i=0; i<4; i++)
	{
		pageDir[i].pd_pres = 1;
		pageDir[i].pd_write = 1;
		pageDir[i].pd_base = ((unsigned int) pt_tab[i])/NBPG;
		//kprintf("\n Page directory %d has page table %d", i, pageDir[i].pd_base);
	}
	return pageDir;
}
pd_t *createPageDirectory(int pid)
{
	int frame = FR_DIR, i;
	get_frm(&frame);
	if(frame == -1)
		return NULL;
	pd_t *pageDirectory;
	frm_tab[frame].fr_type = FR_DIR;
	frm_tab[frame].fr_pid = pid;
	pageDirectory = (frame+FRAME0)*NBPG;
	for(i=0; i<(NBPG/4); i++)
	{
		pageDirectory[i].pd_pres = 0;
		pageDirectory[i].pd_write = 0;
		pageDirectory[i].pd_user = 0;
		pageDirectory[i].pd_pwt = 0;
		pageDirectory[i].pd_pcd = 0;
		pageDirectory[i].pd_acc = 0;
		pageDirectory[i].pd_mbz = 0;
		pageDirectory[i].pd_fmb = 0;
		pageDirectory[i].pd_global = 0;
		pageDirectory[i].pd_avail = 0;
		pageDirectory[i].pd_base = 0;
	}
	return pageDirectory;
}

int resetPT(int frame)
{
	int i, j, k, flag=0;
	struct pentry *p;
	pd_t *pd;
	pt_t *pt;
	for(i=0; i<NPROC; i++)
	{
		p = &proctab[i];
		if(p->pstate == PRFREE)
			continue;
		pd = p->pdbr;
		for(j=4; j<(NBPG/4); j++)
		{
			if(pd[j].pd_pres != 0)
			{
				flag = 0;
				pt = pd[j].pd_base * NBPG;
				for(k=0; k<(NBPG/4); k++)
				{
					if(pt[k].pt_pres != 0)
					{
						if(pt[k].pt_base == frame+FRAME0)
						{
							pt[k].pt_pres = 0;
							pt[k].pt_base = 0;
						}
					}
				}
				for(k=0; k<(NBPG/4); k++)
				{
					if(pt[k].pt_pres != 0)
						flag = 1;
				}
				if(flag != 1)
					freept(pt, i);
			}
		}
	}
	return OK;
}

void freept(pt_t *pt, int pid)
{
	unsigned int frame = ((unsigned int) pt / NBPG);
	struct pentry *p;
	pd_t *pd;
	p = &proctab[pid];
	pd = p->pdbr;
	int i;
	for(i=4; i<(NBPG/4); i++)
	{
		if(pd[i].pd_pres != 0)
		{
			if(pd[i].pd_base == frame)
			{
				pd[i].pd_base = 0;
				pd[i].pd_pres = 0;
				free_frm(frame - FRAME0);
			}
		}
	}
}