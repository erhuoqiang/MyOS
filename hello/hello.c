typedef void(*init_func)(void);

#define UCON0 ((volatile unsigned int *)(0x50000020))
#define NULL ((void *)0)
void SendStr(const char * str)
{
	while(*str)
	{
		*(UCON0) = *str++;
	}
}

void arm920t_init_mmu(void)
{
	SendStr("arm920t_init_mmu\n");
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

static init_func init[]=
{
	arm920t_init_mmu,
	s3c2410_init_clock,
	s3c2410_init_memory,
	s3c2410_init_irq,
	s3c2410_init_io,
	0
};

void hello(init_func * init)
{
	int i;
	for(i = 0; init[i]; i++)
		init[i]();
	
	//boot_start();
}
void plat_boot(void)
{
	hello(init);
}
	
		
