#ifndef __STORAGE_H__
#define __STORAGE_H__

#define MAX_STORAGE_DEVICE (2)   //系统允许注册的存储器数目
#define RAMDISK  0 //RAMDISK 的序号
#ifndef NULL
#define NULL 		((void *)0)
#endif
typedef unsigned int size_t;
//存储设备结构体
struct storage_device
{
	unsigned int start_pos;  //起始地址
	size_t sector_size;  //节大小
	size_t storage_size; //存储设备的总大小
	int (*dout)(struct storage_device *sd, void * dest, unsigned int bias, size_t size);  //读函数指针  sd是读的设备， dest读出数据存储地址，bias表示偏移量 ,size表示大小
	int (*din)(struct storage_device *sd, void *dest, unsigned int bias, size_t size);  //写函数指针
};

extern struct storage_device * storage[MAX_STORAGE_DEVICE];
extern int register_storage_device(struct storage_device * sd, unsigned int num); 
extern int ramdisk_driver_init(void);
#endif


