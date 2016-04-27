#include"storage.h"
#include "fs.h"
#define UCON0 		((volatile unsigned int *)(0x50000020))  //因为这里的UCON0是在MMU激活前调用  所以是用物理地址
#ifndef NULL
#define NULL 		((void *)0)
#endif
/******************   以下是定时器4寄存器相关寄存器的地址 虚拟地址 在MMU激活之前后***********/
#define TIMER_BASE	(0xd1000000)
#define TCFG0		((volatile unsigned int *)(TIMER_BASE + 0X0))
#define TCFG1		((volatile unsigned int *)(TIMER_BASE + 0X4))
#define TCON 		((volatile unsigned int *)(TIMER_BASE + 0X8))
#define TCONB4		((volatile unsigned int *)(TIMER_BASE + 0X3c))

extern void init_sys_mmu(void);
extern void start_mmu(void);
extern void Enable_IRQ(void);
extern void umask_int(unsigned int offset);
extern void * get_free_pages(unsigned int flag, int order);
extern void put_free_pages(void * addr, int order);
extern char * INT_to_Str(unsigned int temp);
extern char num[25];
extern void init_page_map(void);
extern int kmalloc_init(void);
extern void * kmalloc( unsigned int size);
extern void kfree(void * addr);
extern int do_fork(int (*f)(void *),void *args);
extern int task_init();

typedef void(*init_func)(void);

void SendStr(const char * str)
{
	while(*str)
	{
		*(UCON0) = *str++;
	}
}
/**********MMC激活后的发送字符函数*************/
void SendStr_VIR(const char * str)
{
	while(*str)
	{
		*((volatile unsigned int *)0xd0000020) = *str++;
	}
}
void hello(void)
{
	const char *str = "helloworld before start MMU\n";
	while(*str)
	{
		*(UCON0) = *str++;
	}
}
void test_mmu(void)
{
	const char *p = "test_mmu\n";
	while(*p){
	*(volatile unsigned int *)0xd0000020 = *p++;
	};
} 

void arm920t_init_mmu(void)
{
	SendStr("\narm920t_init_mmu\n");
}
void s3c2410_init_clock(void)
{
	SendStr("s3c2410_init_clock\n");
}
void s3c2410_init_memory(void)
{
	SendStr("s3c2410_init_memory\n");
}
void s3c2410_init_irq(void)
{
	SendStr("s3c2410_init_irq\n");
}
void s3c2410_init_io(void)
{
	SendStr("s3c2410_init_io\n");
}

/**************定时器4中断初始化***********/
void Timer_Init(void)
{

	*TCFG0 |= 0X800; // FCLK内部时钟分频后得到HCLK 外部高速时钟 PCLK低速外围设备时钟和UCLK USB时钟  PCLK由FCLK分频后还可以载分频 这里TCFG0是一级分频  在其15-8位填写分频的值 这里是8为1/8分频. 二级分频TCFG1默认是1/2分频
	*TCON  &= (~(7 << 20));  //清除20 21 22位
	*TCON  |= (1 << 22);	//开启自动重装    当计数到0时将TCONB4中的值自动复制到内部寄存器TCNT4
	*TCON  |= (1 << 21);	//将TCONB的置装载到TCON  在计数开始前是需要的
	*TCONB4 = 0xffff;	//计数置 
	*TCON  |= (1 << 20);	//开启寄存器
	*TCON  &= ~(1 << 21);	//关闭装载

	umask_int(14);           //关闭对14号中断源的屏蔽  INTMSK 对应位置0
	Enable_IRQ();		//开启总中断
	
	SendStr_VIR("Timer_Init\n");
}
void test_timer()
{
	SendStr_VIR("this is timer\n");
	//Disable_IRQ();
}
static init_func init[]=
{
	arm920t_init_mmu,
	s3c2410_init_clock,
	s3c2410_init_memory,
	s3c2410_init_irq,
	s3c2410_init_io,
	hello,
	0
};

void printf_int(unsigned int temp)
{	
	INT_to_Str(temp);
	SendStr_VIR(num);
}

//int whilecounter = 25;
void Delay()
{
	volatile unsigned int time = 0xffff;
	while(time--);
}
int test_process(void *p)
{
	while(1)
	{
		Delay();
		SendStr_VIR("test process id is:");
		 printf_int((unsigned int)(p));
		SendStr_VIR("\n");
	}
}

void load_init_boot(init_func * init)
{
	char * p1 = NULL;
	int i = 0;
	char buf[20];
	char ram[20];
	struct inode *node = NULL;

	arm920t_init_mmu();
	s3c2410_init_clock();
	s3c2410_init_memory();
	s3c2410_init_irq();
	s3c2410_init_io();
	hello();
	init_sys_mmu();  //初始化mmu
	start_mmu();  //激活mmu
	test_mmu(); 
	
	ramdisk_driver_init(); //RAMDISK初始化
	romfs_init();	//文件系统初始化
	//while(--whilecounter)
	//SendStr_VIR("while xzq\n");
	
	init_page_map();  //buddy算法 分配
	kmalloc_init();  //动态内存slab管理初始化
	SendStr_VIR("init_page_map\n");
	task_init();  //多进程初始化
	Timer_Init();  //定时器初始化
	//Disable_IRQ();
	/*for(i = 0; i < 9; i++ )
	{
	p1 = (char *)get_free_pages(0,0);
	INT_to_Str((unsigned int)p1);
	SendStr_VIR(num);  //十进制显示
	}
	put_free_pages(p1,6);
	p1 = (char *)get_free_pages(0,6);
	INT_to_Str((unsigned int)p1);
	SendStr_VIR(num);  //十进制显示
	put_free_pages(p1,6);
	
	p1 = (char *)kmalloc(64);
	INT_to_Str((unsigned int)p1);
	SendStr_VIR(num);  //十进制显示
	kfree(p1);
	
	p1 = (char *)kmalloc(64);
	INT_to_Str((unsigned int)p1);
	SendStr_VIR(num);  //十进制显示
	kfree(p1);
	
	storage[RAMDISK]->dout(storage[RAMDISK],(void *)buf, 0, 10);
	SendStr_VIR("\n");
	for(i = 0; i < 5; i++)
	{
		 printf_int((int)buf[i]);
	}	
		
	node  = fs_type[ROMFS]->namei(fs_type[ROMFS],"number.txt");
	if(node == NULL)
		SendStr_VIR("namei is error\n");
	SendStr_VIR(num);  //十进制显示
	fs_type[ROMFS]->device->dout(fs_type[ROMFS]->device,ram, fs_type[ROMFS]->get_daddr(node),node->dsize);
	for(i = 0; i < 10; i++)
	{
		 printf_int((int)ram[i]);
	}	
*/		
	do_fork(test_process, (void *)0x1);
	do_fork(test_process, (void *)0x2);
	do_fork(test_process, (void *)0x3);
	while(1)
	{
		Delay();
		 SendStr_VIR("this is the original process\n");
	}
	while(1);
	//boot_start();
}
void plat_boot(void)
{
	load_init_boot(init);
}
	
		
