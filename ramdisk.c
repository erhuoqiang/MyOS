#include"storage.h"

#define RAMDISK_SECTOR_SIZE 512
#define RAMDISK_SECTOR_MASK (~(RAMDISK_SECTOR_SIZE - 1))
#define RAMDISK_SECTOR_OFFSET (RAMDISK_SECTOR_SIZE -1)

extern remap_ll(unsigned int paddr, unsigned int vaddr ,int size); //在mmu.c里面
extern register_storage_device(struct storage_device * sd, unsigned int num);
void * memcpy(void *dest, const void *src, unsigned int num)
{
	char * tempdest = (char *)dest;	
	char * tempsrc = (char *)src;
	
	if(dest == NULL || src == NULL || num <0)
	{
		return NULL;
	}
	
	while(num--)
	{
		*tempdest++ = *tempsrc++;
	}
	return dest;
}
extern void SendStr_VIR(const char * str);
//RAMDISK的读函数
int ramdisk_dout(struct storage_device *sd, void *dest, unsigned int addr, size_t size)
{SendStr_VIR("init_page_map\n");
	if(sd == NULL)
		return -1;	
	
       if( memcpy(dest, (void *)(addr+sd->start_pos), size) == NULL)
		return -1;	
	return 0;
}

//填充存储设备结构体
struct storage_device ramdisk_storage_device = 
{
	.start_pos = 0x40800000,
	.sector_size = RAMDISK_SECTOR_SIZE,
	.storage_size = 2 * 1024* 1024, 
	.dout = ramdisk_dout,
	 .din = NULL
};

int ramdisk_driver_init(void)
{
	int ret;
	remap_ll(0x30800000, 0x40800000, 2 * 1024 * 1024);   //将设备映射到MMU
	ret = register_storage_device(&ramdisk_storage_device, RAMDISK);//注册到系统
	return ret;
}

	

