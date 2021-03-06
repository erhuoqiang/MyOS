#define SRCPND		(0xca000000)  //SRCPND 物理地址0X4A000000对应的虚拟地址
#define INTMSK		(SRCPND+0x8)	//中断屏蔽寄存器	
#define INTOFFSET 	(SRCPND+0x14)		
#define INTPND 		(SRCPND+0x10)

#define TIMER_BASE	(0xd1000000)
#define TCON 		((volatile unsigned int *)(TIMER_BASE + 0X8))
extern  void SendStr_VIR(const char * str);

//激活全局中断
void Enable_IRQ(void)
{
	//SendStr_VIR("Enable_IRQ\n");
	asm volatile(
	"mrs r4,cpsr\n\t"
	"bic r4,r4,#0x80\n\t"    //bit指令是将对应位置0  将后面常数 为1的位置0
	"msr cpsr,r4\n\t"
	:::"r4");
	
}

void Disable_IRQ(void)
{
	
	asm volatile(
	"mrs r4,cpsr\n\t"
	"orr r4,r4,#0x80 \n\t"  
	"msr cpsr,r4\n\t"
	:::"r4");
	//SendStr_VIR("Disable_IRQ\n");
}
void umask_int(unsigned int offset)
{	
	SendStr_VIR("umask_int\n");
	*(volatile unsigned int *)INTMSK&=~(1<<offset);  //屏蔽某位 对应位则置1  通过则为0
}

char num[25] = "111";
/*int strlen(const char * src)
{
	const char * temp = src;
	if(src == 0)	
		return 0;
	while(*src++);
	return src - temp - 1;
}*/

	
char * INT_to_Str(unsigned int temp)
{
	unsigned int i = 0;
	char num1[25]="";
	char * src = num1;
	char * dest = num;
	if(temp == 0) 
	{
		num[0] = '0';	
		num[1] = '\n';	
		num[2] = 0;
		return num;
	}
	while(temp != 0)
	{
		num1[i++] = temp % 10 + 48;
		temp /= 10;
	}
	//num1[i++] = '\n';
	num1[i] = 0;
	dest = dest + i;
	*dest-- = 0;
	while(*src)
	{
		*dest-- = *src++;
	}
	
	return num;
}

int counter = 0;
void commom_irq_handler(void)
{
	unsigned int tmp = (1<<(*(volatile unsigned int *)INTOFFSET));
	INT_to_Str(*(volatile unsigned int *)INTOFFSET);
	SendStr_VIR(num);
	*(volatile unsigned int *)SRCPND |= tmp;
	*(volatile unsigned int *)INTPND |= tmp;
	Enable_IRQ();
	
	SendStr_VIR("timer interrupt occured\n");

	if( ++counter == 10) 
	{
		*TCON &= ~(1<<20);
		SendStr_VIR("Close Timer4\n");
	}
}




