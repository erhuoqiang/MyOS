
#define TASK_SIZE 4096
#define TASK_MASK (~(TASK_SIZE - 1))
#define current current_task_info()

#ifndef NULL
#define NULL ((void *)0)
#endif

extern void Disable_IRQ(void);
extern void Enable_IRQ(void);
extern void * kmalloc( unsigned int size);
extern void kfree(void * addr);
//由进程空间当前sp的指向 和TASK_MASK与后得到进程空间的起始地址
struct task_info * current_task_info(void)
{
	register unsigned long sp asm("sp");
	return (struct task_info *)(sp&TASK_MASK);
}
//描诉进程空间的结构体
struct task_info
{
	unsigned int sp;
	struct task_info * next;
};


//返回下一个进程的空间的结构体  汇编里就是将值给R0
void *__common_schedule(void)
{
	return (void *)current->next;
}


int task_init()
{
	current->next = current;
}

#define DO_INIT_SP(sp,fn,args,lr,cpsr,pt_base) \
	do{\
	(sp) = (sp) - 4;\
	*(volatile unsigned int *)(sp) = (unsigned int)(fn); /*r15  fn是该进程空间对应的进程函数 也就是下一次需要返回的地址*/\
	(sp) = (sp) - 4;\
	*(volatile unsigned int *)(sp) = (unsigned int)(lr); /*r14   r14用于保存fn运行过程中调用函数时的返回地址*/\
	(sp) = (sp) - 4*13;/*为r0-r12腾出空间*/\
	*(volatile unsigned int *)(sp) = (unsigned int)(args); /*r0 = arg(sp) = (sp) - 4;  //进程函数传参用的参数，如果参数过多用结构体*/\
	(sp) = (sp) - 4;\
	*(volatile unsigned int *)(sp) = (unsigned int)(cpsr); /*cpsr*/\
	}while(0)

unsigned int get_cpsr(void)
{
	unsigned int p;
	asm volatile
	(
		"mrs %0,cpsr\n"
		:"=r"(p)
		:
	);
	return p;
}
/*
int task_stack_base = 0x30300000;
struct  task_info * copy_task_info()
{
	struct task_info * temp = (  struct task_info * )task_stack_base;
	 task_stack_base = (struct task_info *)((unsigned int )task_stack_base+ TASK_SIZE);
	return temp;
}*/
//添加一个进程分四个步骤 第一个分配空间这里是4K 然后初始化堆栈空间的SP指针，然后接着初始化进程函数，将寄存器r0-r12 ，cpsr，PC，R14等初始化
//然后就是将进程添加进系统的进程链表实现 微观串行，宏观并行的多线程效果。
int do_fork(int (*f)(void *),void *args)
{
	struct task_info *task = NULL,*temp = NULL;
	if((task = (struct task_info *)kmalloc(TASK_SIZE-1)) == NULL)
		return -1;
	
	//task = copy_task_info();
	task->sp = (unsigned int)((unsigned int )task + TASK_SIZE);
	DO_INIT_SP(task->sp,f,args,0,0x1f&get_cpsr(),0);  //传递cpsr当前模式所以需要取后5位
	Disable_IRQ();
	task->next = current->next;
	current->next = task;
	Enable_IRQ();
	return 0;
}
	




