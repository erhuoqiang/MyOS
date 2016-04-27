#define PAGE_TABLE_L1_BASE_ADDR_MASK (0xffffc000) //与页表段基地址相与保证了其后12位是0
#define VIRT_TO_PTE_L1_INDEX(addr) (((addr)&0xfff00000)>>18)
#define PTE_L1_SECTION_NO_CACHE_AND_WB (0x0<<2)
#define PTE_L1_SECTION_DOMAIN_DEFAULT (0x0<<5)
#define PTE_ALL_AP_L1_SECTION_DEFAULT (0x1<<10)
#define PTE_L1_SECTION_PADDR_BASE_MASK (0xfff00000)
#define PTE_BITS_L1_SECTION (0x2)  //段页表的开始两位是10
#define L1_PTR_BASE_ADDR 0x30700000  //自己定义的页表基地址
#define PHYSICAL_MEM_ADDR 0x30000000  // 运行内存物理地址地址
#define VIRTUAL_MEM_ADDR 0x30000000  //内存映射的地址和物理地址一样 没有变化 这要特别注意
#define MEM_MAP_SIZE 0x800000		//运行内存大小为8M
#define PHYSICAL_IO_ADDR 0x48000000  //外设物理地址 
#define VIRTUAL_IO_ADDR 0xc8000000  //外设起始物理地址映射到的虚拟地址
#define IO_MAP_SIZE 0x18000000 		//外设地址空间大小336M
#define VIRTUAL_VECTOR_ADDR 	0x0	//异常向量表从内存起始位置映射0X3000000到虚拟地址的0X0
#define PHYSICAL_VECTOR_ADDR 	0x30000000  

/*
虚拟地址的组成 ：高12位由页表项对应的偏移量加上20位的物理地址组成（低20位）。  
 虚拟地址到物理地址的转换：取虚拟地址的高12位，也就是页表项对应偏移量 左移两位（因为一个页表项占用4个字节） 加上页表段基地址（由用户自己定义，如上面的宏） 找到页表项的物理地址 读取后  取高12位（页表项的高12位代表物理地址的高12位） 与虚拟地址的后20位相加 得到32完整的物理地址
*/

/*
获取页表项 这里是一级查表 一页对应1M的空间  页表项高12位对应物理地址的高12位，11-10位是AP位是用来控制用户模式和特权模式的访问权限8-5位是域 ARM中有16个域 每个域有自己的权限  这4位控制该页属于哪个域 域的权限由协处理器CP15中的C3寄存器控制（两位控制1一个域 寄存器是32位） ，从而实现存储器的初级保护。所以页的权限主要跟域和访问权限（ACCESS PERMISION）简称AP 有关，域是修改一页也就是1M的内存的权限，当对应域的C3寄存器中的2位 为11时 该域不受AP控制，为10结果不可预料，为01 权限受AP控制， 为00时 产生错误。  当由AP控制时 当两位都为0的时候 又受协处理器的CP15的C1寄存器的 S和R位控制，AP 有一个不为0时  则由AP位控制 具体权限查表可得3-3。页表项的第3位第2位 分表代表CACHE （保存着最近一段时间被CPU使用过俄内存数据）和write buffer(写缓存器，是用来对应内存的写操作的，将原本要写向内存的数据写到write buffer等CPU空闲下来的时候，再来搬进内存）是否使用的允许位
*/
unsigned int Get_Page_Term(unsigned int paddr)
{
		/*
		物理地址paddr 与上0xfff00000 取前12位 然后加上0x2 用来区分页表的类型 
		这里的MMU是采用一级查表  为段页表 所以最后两位总是二进制10）
		*/
	return((paddr &  PTE_L1_SECTION_PADDR_BASE_MASK)|PTE_BITS_L1_SECTION);
}



/*
通过页表的基地址与虚拟地址产生该段页表项的地址 ,取虚拟地址的高12位，也就是页表项对应偏移量 左移两位（因为一个页表项占用4个字节） 加上页表段基地址（由用户自己定义，如上面的宏）
*/
unsigned int Get_Page_Term_Addr(unsigned int baddr, unsigned int vaddr)
{
	return(baddr&PAGE_TABLE_L1_BASE_ADDR_MASK)|VIRT_TO_PTE_L1_INDEX(vaddr);
}



/*
	初始化MMU：建立页表项，  将地址空间0x30000000～0x30800000映射到虚拟地址0x30000000～0x30800000
	将地址空间0x48000000～0x60000000映射到虚拟空间0xc8000000～0e0000000	
	将内存地址的起始的1M内存，存放异常向量表的地方，映射到虚拟地址0X0~0x00100000
	页表段基地址为 L1_PTR_BASE_ADDR 0x30700000  
*/
void init_sys_mmu(void)
{
	unsigned int pte;
	unsigned int pte_addr;
	int j;
	//将8M内存进行虚拟地址映射
	for(j = 0; j < MEM_MAP_SIZE >> 20; j ++)
	{
		pte =  Get_Page_Term(PHYSICAL_MEM_ADDR + (j << 20));
		pte |= PTE_ALL_AP_L1_SECTION_DEFAULT;  //设置AP
		pte |= PTE_L1_SECTION_NO_CACHE_AND_WB;	//设置CACHE WRITEBUFFER
		pte |= PTE_L1_SECTION_DOMAIN_DEFAULT;	//设置域
		pte_addr = Get_Page_Term_Addr(L1_PTR_BASE_ADDR, VIRTUAL_MEM_ADDR+(j << 20));
	*(volatile unsigned int *)pte_addr=pte;
	}
	//将336M外设地址寻址空间进行虚拟地址映射 
	for(j = 0; j < IO_MAP_SIZE>>20; j ++)
	{
		pte =  Get_Page_Term(PHYSICAL_IO_ADDR + (j << 20));
		pte |= PTE_ALL_AP_L1_SECTION_DEFAULT;
		pte |= PTE_L1_SECTION_NO_CACHE_AND_WB;
		pte |= PTE_L1_SECTION_DOMAIN_DEFAULT;
		pte_addr = Get_Page_Term_Addr(L1_PTR_BASE_ADDR, VIRTUAL_IO_ADDR + (j << 20));
		*(volatile unsigned int *)pte_addr = pte;
	}

	//将内存地址的起始的1M内存，存放异常向量表的地方，映射到虚拟地址0X0~0x00100000
	for(j = 0; j < 1; j++)
	{
		pte = Get_Page_Term(PHYSICAL_VECTOR_ADDR + (j << 20));
		pte |= PTE_ALL_AP_L1_SECTION_DEFAULT;
		pte |= PTE_L1_SECTION_NO_CACHE_AND_WB;
		pte |= PTE_L1_SECTION_DOMAIN_DEFAULT;
		pte_addr = Get_Page_Term_Addr(L1_PTR_BASE_ADDR,VIRTUAL_VECTOR_ADDR + (j << 20));
		*(volatile unsigned int *)pte_addr = pte;
	}
	
}

//RAMDISK MMU映射初始化
void remap_ll(unsigned int paddr, unsigned int vaddr, int size)
{
	unsigned int pte;
	unsigned int pte_addr;
	int j;
	//将8M内存进行虚拟地址映射
	for(j = 0; j < size >> 20; j++)
	{
		pte =  Get_Page_Term(paddr + (j << 20));
		pte |= PTE_ALL_AP_L1_SECTION_DEFAULT;  //设置AP
		pte |= PTE_L1_SECTION_NO_CACHE_AND_WB;	//设置CACHE WRITEBUFFER
		pte |= PTE_L1_SECTION_DOMAIN_DEFAULT;	//设置域
		pte_addr = Get_Page_Term_Addr(L1_PTR_BASE_ADDR, vaddr + (j << 20));
		*(volatile unsigned int *)pte_addr=pte;
	}
}

/*激活MMU MMU的配置和实现都是通过操作CP15协处理器实现的 (主要是配置CP15的C2（保存页表起始地址) C3（内存域权限） C1 （0bit为MMU使能端）寄存器）ARM 支持16个协处理器 但是ARM920T系列的处理器协处理器只有2个 ，一个负责系统调试CP14，另外一个负责系统控制CP15，系统中的cache ,write buffer,MMU, 时钟模式等都可以通过操作CP15来完成的。ARM提供了一组专门的操作协处理器指令。CP15协处理器内部有16个寄存器*/
void start_mmu(void)
{
	unsigned int ttb = L1_PTR_BASE_ADDR;
	asm(//代码列表
	"mcr p15, 0, %0, c2, c0, 0\n"  //将页表的基地址L1_PTR_BASE_ADDR给CP15：C2保存  C2是CP15Y用来保存页面基地址的寄存器 （%0代表输入运算符的第一个，则是上面定义的ttb)
	"mvn r0, #0\n"   //mvn是 先将目的操作数取反 在给源操作数 ，r0 = ~0 = 0xffffffff 
	"mcr p15, 0, r0, c3, c0, 0\n"  //r0 给 CP15：C3寄存器，CP15：C3寄存器是控制内存域权限的 全部为1  则 可读写 不受AP控制
	"mov r0, #0x1\n"      //r0 = 0x1
	"mcr p15, 0, r0, c1, c0, 0\n"  //r0 =0x1 给 CP15：C1寄存器的第一位CP15：C1寄存器的第一位，其掌握了激活MMU的大全 也就是该位置1，MMU激活
				//后面运行的程序的地址都要是虚拟地址（前面的程序则用的是物理地址)虚拟地址到物理地址的转化由CP15协处理器控制。
					// init_sys_mmu（）的作用是建立页表 也就是初始化页表的每一项，而CP15协处理器则负责接受虚拟地址将其高12位
					//左移18位和页表基地址相加 找到对应的页表项 读取后 取高12 位和虚拟地址的低20位结合则完成转换
	"mov r0, r0\n"  //可忽略
	"mov r0, r0\n"
	"mov r0, r0\n"
	:		//输出运算符列表 
	: "r" (ttb)	//输入运算符列表      把ttb作为输入参数， 给通用寄存器，r 代表通用寄存器R0-R15
	: "r0"    	//被更改资源列表 代码列表中 只有 r0被更改了
	);
} 

		
