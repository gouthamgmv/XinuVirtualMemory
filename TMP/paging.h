/* paging.h */

typedef unsigned int	 bsd_t;

/* Structure for a page directory entry */

typedef struct {

  unsigned int pd_pres	: 1;		/* page table present?		*/
  unsigned int pd_write : 1;		/* page is writable?		*/
  unsigned int pd_user	: 1;		/* is use level protection?	*/
  unsigned int pd_pwt	: 1;		/* write through cachine for pt?*/
  unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
  unsigned int pd_acc	: 1;		/* page table was accessed?	*/
  unsigned int pd_mbz	: 1;		/* must be zero			*/
  unsigned int pd_fmb	: 1;		/* four MB pages?		*/
  unsigned int pd_global: 1;		/* global (ignored)		*/
  unsigned int pd_avail : 3;		/* for programmer's use		*/
  unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {

  unsigned int pt_pres	: 1;		/* page is present?		*/
  unsigned int pt_write : 1;		/* page is writable?		*/
  unsigned int pt_user	: 1;		/* is use level protection?	*/
  unsigned int pt_pwt	: 1;		/* write through for this page? */
  unsigned int pt_pcd	: 1;		/* cache disable for this page? */
  unsigned int pt_acc	: 1;		/* page was accessed?		*/
  unsigned int pt_dirty : 1;		/* page was written?		*/
  unsigned int pt_mbz	: 1;		/* must be zero			*/
  unsigned int pt_global: 1;		/* should be zero in 586	*/
  unsigned int pt_avail : 3;		/* for programmer's use		*/
  unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

typedef struct{
  unsigned int pg_offset : 12;		/* page offset			*/
  unsigned int pt_offset : 10;		/* page table offset		*/
  unsigned int pd_offset : 10;		/* page directory offset	*/
} virt_addr_t;

typedef struct{
  int bs_status;			/* MAPPED or UNMAPPED		*/
  int bs_pid;				/* process id using this slot   */
  int bs_vpno;				/* starting virtual page number */
  int bs_npages;			/* number of pages in the store */
  int bs_sem;				/* semaphore mechanism ?	*/
  bsd_t bs_bsid;			
  struct bs_map_t *next;
  int bs_heap;
} bs_map_t;

typedef struct{
  int fr_status;			/* MAPPED or UNMAPPED		*/
  int fr_pid;				/* process id using this frame  */
  int fr_vpno;				/* corresponding virtual page no*/
  int fr_refcnt;			/* reference count		*/
  int fr_type;				/* FR_DIR, FR_TBL, FR_PAGE	*/
  int fr_dirty;
  int fr_timeCount;
  void *cookie;				/* private data structure	*/
  unsigned long int fr_loadtime;	/* when the page is loaded 	*/
}fr_map_t;

/*typedef struct fr_frlist
{
	int fr_frid;
	struct fr_frlist *prev;
	struct fr_frlist *next;
} fr_frlist;*/

typedef struct fr_list
{
	int fr_frid;
	struct fr_list *next;
} fr_list;

extern bs_map_t bsm_tab[];
extern fr_map_t frm_tab[];
/* Prototypes for required API calls */
SYSCALL xmmap(int, bsd_t, int);
SYSCALL xunmap(int);

/* given calls for dealing with backing store */

int get_bs(bsd_t, unsigned int);
SYSCALL release_bs(bsd_t);
SYSCALL read_bs(char *, bsd_t, int);
SYSCALL write_bs(char *, bsd_t, int);

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/

//default 3072 frames --> 1024+3072=4096=16M
//#define NFRAMES 	3072	/* number of frames		*/
#define NFRAMES 	20	/* number of frames		*/

#define BSM_UNMAPPED	0
#define BSM_MAPPED	1

#define FRM_UNMAPPED	0
#define FRM_MAPPED	1

#define FR_PAGE		0
#define FR_TBL		1
#define FR_DIR		2

#define FIFO 3
#define LRU 4

#define MAX_ID          15              /* You get 10 mappings, 0 - 9 */

#define BACKING_STORE_BASE	0x00800000
#define BACKING_STORE_UNIT_SIZE 0x00080000

extern int timeCount;
extern int init_pt();
extern pt_t *createPageTable();
extern pd_t *init_pd();
extern pd_t *createPageDirectory();
extern fr_list *addn(int frame);
extern void insn(int frame);
extern void deln(int frame);
extern void freept(pt_t *pt, int pid);
extern void updateTimeCounts();