#ifndef _LISH_H
#define _LISH_H
#include "list.h"
#endif
extern  void SendStr_VIR(const char * str);
extern char * INT_to_Str(unsigned int temp);
extern char num[25];

//slab管理机制用的结构体
struct kmem_cache
{
	unsigned int obj_size;   //子内存大小
	unsigned int obj_nr;	 //当前子内存数目
	unsigned int page_order;	//SLAB 申请的内存ORDER大小
	unsigned int flags;	
	struct page * head_page;	
	struct page * end_page;
	void *nf_block;		//下一个可用内存块的地址
};
//伙伴算法管理PAGE用的结构体
struct page
{
	unsigned int vaddr;  
	unsigned int flags;
	int order;
	struct kmem_cache *cachep;
	struct list_head list;
};

#define _MEM_END 0x30700000
#define _MEM_START 0x300f0000

#define PAGE_SHIFT (12)    //4KB 每PAGE
#define PAGE_SIZE  (1<<PAGE_SHIFT)

#define KERNEL_MEM_END (_MEM_END)

#define KERNEL_PAGING_START  ( (_MEM_START + PAGE_SIZE -1 ) & (~(PAGE_SIZE -1)) )
#define KERNEL_PAGING_END	(((KERNEL_MEM_END - KERNEL_PAGING_START) / (PAGE_SIZE + sizeof(struct page))) *PAGE_SIZE + KERNEL_PAGING_START)

#define KERNEL_PAGE_NUM 	 ((KERNEL_PAGING_END - KERNEL_PAGING_START) / PAGE_SIZE)
#define KERNEL_PAGE_END 	_MEM_END
#define KERNEL_PAGE_START 	(KERNEL_PAGE_END - sizeof(struct page) * KERNEL_PAGE_NUM)

/****************BUDDY的初始化**********************/
#define PAGE_AVAILABLE 0x00
#define PAGE_DIRTY     0x01
#define PAGE_PROTECT   0x02
#define PAGE_BUDDY_BUSY 0x04
#define PAGE_IN_CACHE  0x08

#define MAX_BUDDY_PAGE_NUM  (9)

#define PAGE_NUM_FOR_MAX_BUDDY ((1<<(MAX_BUDDY_PAGE_NUM-1)) - 1)             ///

struct list_head page_buddy[MAX_BUDDY_PAGE_NUM];

void init_page_buddy(void)
{
	int i = 0;
	for(; i < MAX_BUDDY_PAGE_NUM; i++)
		INIT_LIST_HEAD(page_buddy + i);
}

void init_page_map(void)
{
	int i;
	struct page *pg = (struct page *)KERNEL_PAGE_START;
	init_page_buddy();
	
	for( i = 0 ; i < KERNEL_PAGE_NUM; i++,pg++ )
	{
		pg->vaddr = (KERNEL_PAGING_START + i * PAGE_SIZE);
		pg->flags = PAGE_AVAILABLE;
		INIT_LIST_HEAD(&(pg->list));
		
		if( i < ( KERNEL_PAGE_NUM &(~PAGE_NUM_FOR_MAX_BUDDY)) )
		{
			if((i & PAGE_NUM_FOR_MAX_BUDDY) == 0)
			{
				//SendStr_VIR(" order 8 ");
				pg->order = MAX_BUDDY_PAGE_NUM - 1;
			}
			else
			{	
				pg->order = -1;
			}
			list_add_tail(&pg->list , &page_buddy[MAX_BUDDY_PAGE_NUM - 1]);		
		}
		else
		{
			SendStr_VIR(" order 0 ");
			pg->order = 0;
			list_add_tail(&pg->list , page_buddy);	
		}
	}	
}



/*****************************BUDDY的申请************************/
#define BUDDY_END(x, order)  ( (x) + (1<<(order)) - 1 )
#define NEXT_BUDDY_START(x, order)  ( (x) + (1<<(order)) )
#define PREV_BUDDY_START(x, order)  ( (x) - (1<<(order)) )
#define NULL 0

struct page * get_pages_from_list(int order)
{
	int neworder = order;
	struct page * pg = NULL, *ret = NULL;
	struct list_head * end = NULL, * tempend = NULL;
	
	for( ; neworder < MAX_BUDDY_PAGE_NUM; neworder++ )
	{	
		if(list_empty(page_buddy+neworder))
		{
			continue;
		}
		else
		{
			pg = list_entry( (page_buddy + neworder)->next, struct page, list );
			end = &(BUDDY_END(pg, neworder)->list);
			end->next->prev = page_buddy + neworder;
			(page_buddy + neworder)->next = end->next;
			goto OUT;
		}
	}
	return NULL;
	
OUT:
	for( neworder--; neworder >= order; neworder-- )
	{
		tempend = &(BUDDY_END(pg, neworder)->list);
		end = &(pg->list);
		pg->order = neworder;
		list_add_chain_tail(end, tempend, page_buddy + neworder);
		pg = NEXT_BUDDY_START(pg, neworder);
	}
	pg->flags |= PAGE_BUDDY_BUSY;  //busy
	pg->order = order;
	BUDDY_END(pg, order)->list.next = NULL;
	return pg;
}

/***********************BUDDY释放*********************************/
void put_pages_to_list(struct page * pg, int order)
{
	struct page * tprev, * tnext, *END;
	if( (pg->flags & PAGE_BUDDY_BUSY) == 0)
		return;
	pg->flags &= ~(PAGE_BUDDY_BUSY);  //free busy
	for( ; order <  MAX_BUDDY_PAGE_NUM - 1 ; order++)
	{
		tprev = PREV_BUDDY_START(pg, order);
		tnext = NEXT_BUDDY_START(pg, order);
		
	        if((!(tnext->flags & PAGE_BUDDY_BUSY)) && (tnext->order == order) )
		{
			pg->order++;
			tnext->order = -1;
			list_remove_chain( &(tnext->list), &(BUDDY_END(tnext,order)->list) );
			BUDDY_END( pg, order )->list.next =  &(tnext->list);
			tnext->list.prev = &(BUDDY_END(pg, order)->list);	
			END = BUDDY_END(tnext, order);
		}
		else if((!(tprev->flags & PAGE_BUDDY_BUSY)) &&  (tprev->order == order) )
		{
			END = BUDDY_END(pg, order);
			pg->order = -1;
			list_remove_chain( &(tprev->list), &(BUDDY_END(tprev,order)->list) );
			BUDDY_END(tprev, order)->list.next = &(pg->list);
			pg->list.prev = &(BUDDY_END(tprev, order)->list);
			pg = tprev;
			pg->order++;
			END = BUDDY_END(pg, order);
		}
		else
		{
			break;
		}
	}
	list_add_chain( &(pg->list), &(END->list), &page_buddy[order]);
}



/**********************BUDDY 申请和释放函数的封装*************/
struct page * paging_to_page(unsigned int addr)
{
	unsigned int i;
	i = (addr - KERNEL_PAGING_START)>>PAGE_SHIFT;
	if(i > KERNEL_PAGE_NUM)
		return NULL;
	return ((struct page *)KERNEL_PAGE_START)+i;
}

void * page_address(struct page * pg)
{
	return (unsigned int*)(pg->vaddr);
}

struct page *alloc_pages(unsigned int flag, int order)
{
	struct page *pg;
	int i;
	pg = get_pages_from_list(order);
	if(pg == NULL)
	{	
		return NULL;
	}
	for(i = 0; i < (1<<order); i++)
	{
		(pg+i)->flags |= PAGE_DIRTY;
	}	
	return pg;
}

void free_pages(struct page *pg, int order)
{
	int i;
	for (i = 0; i < (1<<order); i++)
	{
		(pg+i)->flags &= ~PAGE_DIRTY;
	}
	put_pages_to_list(pg, order);
}

void * get_free_pages(unsigned int flag, int order)
{
	struct page * page;
	page = alloc_pages(flag, order);
	if(page == NULL)
	{
		return NULL;
	}
	return page_address(page);
}

void put_free_pages(void * addr, int order)
{
	free_pages(paging_to_page( (unsigned int )addr ), order);	
}




/*************************************************SLAB管理机制*****************************************/
#define KMEM_CACHE_DEFAULT_ORDER (0)
#define KMEM_CACHE_MAX_ORDER (5)
#define KMEM_CACHE_SAVE_RATE (0x5a)   //90
#define KMEM_CACHE_PERSENT   (0X64) 	//100
#define KMEM_CACHE_MAX_WAST	((PAGE_SIZE) - ((PAGE_SIZE)*KMEM_CACHE_SAVE_RATE)/KMEM_CACHE_PERSENT)   //0.1*PAGE_SIZE


/*跟据子内存快的大小找到合适的order值（order分配的BUDDY大小 能够容纳10块以上为合适) 在ORDER为5以内若不存在合适的
则判断子内存快有大于  （1<< KMEM_CACHE_MAX_ORDER ) *PAGE_SIZE若没有大于 则返回ORDER =  KMEM_CACHE_MAX_ORDER ； */
int find_right_order(unsigned int size)
{
	int order = 0;
	for( ; order <= KMEM_CACHE_MAX_ORDER; order++)
	{
		if( size < KMEM_CACHE_MAX_WAST * (1<<order) )
		{
			return order;
		}
	}
	
	if( size > PAGE_SIZE * (1<<order) )   //
	{	
		return -1;	
	}
	return order;

}

/*将slab 内存分配成若干子块，每一个子块的大小由obj_size决定。子块分配完成后，再让每一个子块最开始的内存中存储下一个子块的起始位置，最后一个存储NULL，返回能容纳子块的个数*/
int kmem_cache_line_object(void * head, unsigned int size, int order)
{
	void ** p1 = (void **)head;
	char * p2 = (char *)head + size;
	
	int i = 0, s = PAGE_SIZE * (1 << order);
	
	for( ; s > PAGE_SIZE; i++, s-= size)
	{
		*(p1) = (void *)p2;
		p1 = (void **)p2;
		p2 += size;
	}
	
	if(s == size)  //如果剩下那部分刚好整除size
	{
		i++;
		*p1 = NULL;
	}
	else
	{
		*(void **)((char *)p1 - size) = NULL;
	}
	return i;
}


struct kmem_cache * kmem_cache_create(struct kmem_cache * cache, unsigned int size, unsigned int flags)
{
	int order = find_right_order(size);
	
	if( order == -1 )
	{
		return  NULL;
	}
	
	if( (cache->head_page = alloc_pages(0, order)) == NULL )
	{
		return NULL;
	}
	cache->nf_block = page_address(cache->head_page);
	
	cache->obj_nr = kmem_cache_line_object( cache->nf_block, size, order );

	cache->obj_size = size;
	cache->page_order = order;
	cache->flags = flags;
	cache->end_page = BUDDY_END(cache->head_page, order);
	
	return cache;
}

void * kmem_cache_alloc(struct kmem_cache *cache, unsigned int flag)
{
	void *p;
	struct page * pg;
	
	if(cache == NULL)
		return NULL;
	
	void ** nf_block = &(cache->nf_block);
	unsigned int *nr = &(cache->obj_nr);
	int order = cache->page_order;
	
	if(*nr == 0)
	{	
		if( (pg = alloc_pages(0, order))  == NULL )
		{
			return NULL;
		}
		*nf_block = page_address(pg);
		cache->end_page->list.next = &pg->list;
		cache->end_page = BUDDY_END(pg, order);	
		*nr += kmem_cache_line_object(*nf_block, cache->obj_size, order);	
	}
	
	*(nr)--;
	p = *nf_block;
	*nf_block = *((void **)*nf_block);
	pg = paging_to_page((unsigned int)p);
	pg->cachep = cache;    //这里为slab子内存块的释放打下基础（可以由被分配的地址转为对应的page结构体然后找到对应的cache，确保每个被分配使用的
				//page都有一个对应的cache。   为kmalloc 后的kfree打下基础
	return p;
}

//slab释放分配的空间
void kmem_cache_free(struct kmem_cache * cache, void *objp)
{
	*(void **)objp = cache->nf_block;
	cache->nf_block = objp;	
	cache->obj_nr++;
}

//SLAB的销毁		
void kmem_cache_destroy(struct kmem_cache * cache)
{
	int order = cache->page_order;
	struct page *pg = cache->head_page;
	struct list_head * list;
	if( cache == NULL )
		return;
	while(1)
	{
		free_pages(pg, order);
		list = BUDDY_END(pg, order)->list.next;
		if( list == NULL)
			return;
		else
			pg = list_entry(list, struct page, list);
	}
}
	
/**************************kmalloc******************************/
#define KMALLOC_BIAS_SHIFT	(5)	
#define KMALLOC_MAX_SIZE	(4096)
#define KMALLOC_MINIMAL_SIZE_BIAS	(1<<(KMALLOC_BIAS_SHIFT))

#define KMALLOC_CACHE_SIZE	(KMALLOC_MAX_SIZE / KMALLOC_MINIMAL_SIZE_BIAS)

struct kmem_cache kmalloc_cache[KMALLOC_CACHE_SIZE] = {{0,0,0,0,NULL,NULL,NULL}};

#define kmalloc_cache_size_to_index(size)  ((size) >> (KMALLOC_BIAS_SHIFT))


int kmalloc_init(void)
{
	int i = 0;
	for( ; i < KMALLOC_CACHE_SIZE; i++)
	{
		if(kmem_cache_create(kmalloc_cache + i, (i + 1) * KMALLOC_MINIMAL_SIZE_BIAS, 0) == 0)
			return -1;
	}
	return 0;
}


void * kmalloc( unsigned int size)
{
	int index =  kmalloc_cache_size_to_index(size);
	if(index >= KMALLOC_CACHE_SIZE)
		return NULL;
	return kmem_cache_alloc(kmalloc_cache, 0);
}



void kfree(void * addr)
{
	struct page * pg;
	pg = paging_to_page((unsigned int)addr);
	kmem_cache_free((pg->cachep),addr);
}







