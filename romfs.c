#include "fs.h"
#include "storage.h"
#include "string.h"

extern void * kmalloc( unsigned int size);
extern void kfree(void * addr);
extern  void SendStr_VIR(const char * str);

#ifndef NULL
#define NULL ((void *)0)
#endif

#define uint unsigned int

//大端转小断
#define be32_to_le32(x) ( (unsigned int)(  \
			 (((unsigned int)(x) & (unsigned int)0x000000ffUL)<<24) | \
			 (((unsigned int)(x) & (unsigned int)0x0000ff00UL)<<8) | \
		         (((unsigned int)(x) & (unsigned int)0x00ff0000UL)>>8) | \
		         (((unsigned int)(x) & (unsigned int)0xff000000UL)>>24)   ))
//romfs_super_block 不同与super_block  ，romfs_super_block只是一个文件系统信息头，是文件的内容，但是super_block是
//用于描诉整个文件系统的结构体
struct romfs_super_block
{
	uint word0;//文件系统的表示符
	uint word1; 
	uint size; //整块数据的大小
	uint checksum;	  //校验和
	char name[0]; //不知长度的卷名 用可越界数组表示
};
//romfs_inode也只是一个文件信息头是，是文件的内容，而inode结构体是用于描诉一个文件的属性信息的
struct romfs_inode
{
	uint next; //下一个头文件地址   BIT0-2 表示文件类型 BIT3文件执行属性
	uint spec; // 如果该文件类型是目录 也就是next的BIT0-2是001 则这里表示的附加信息是目录内首文件地址
	uint size; //文件大小
	uint checksum; //文件校验和
	char name[0]; //文件名
};

struct super_block romfs_super_block;

#define ROMFS_MAX_FILE_NAME (128)
#define ROMFS_NAME_ALIGN_SIZE (16)
#define ROMFS_SUPER_UP_MARGIN (16)
#define ROMFS_NAME_MASK (~(ROMFS_NAME_ALIGN_SIZE -1))
#define ROMFS_NEXT_MASK (0xfffffff0)

//romfs_get_first_file_header 用于从文件系统信息头计算出第一个文件头的位置
//系统信息头的卷名 加上系统信息头结构体大小 然后向下16位对其 得出
#define romfs_get_first_file_header(p)  ((( strlen(((struct romfs_super_block *)(p))->name) \
						 + ROMFS_NAME_ALIGN_SIZE  \
						 + ROMFS_SUPER_UP_MARGIN ) & ROMFS_NAME_MASK) + (char *)(p))

#define romfs_get_file_data_offset(p,num)   ((( (num) \
                                                 + ROMFS_NAME_ALIGN_SIZE  \
                                                 + ROMFS_SUPER_UP_MARGIN ) & ROMFS_NAME_MASK) + (char *)(p)) 


//去除最顶层的目录名，让其保存载buf，然后返回余下的目录名
//如 xxx/ccc/zzz  调用函数后buf位xxx  返回ccc/zzz
static char * bmap(char * buf,const char *dir)
{
	int n = 0;
	char * p = NULL;
	if( buf == NULL || dir == NULL)
		return NULL;
	if(*dir == '/')   //出现 /xxx/aaa 第一个字符就是/的情况直接跳过 接着找下一个
		dir++;
	if( ( p = strchr(dir, '/')) == NULL) 
	{
		strcpy(buf,dir);
		return NULL;
	}
	n = p - dir;
	strncpy(buf, dir, n);
	buf[n] = '\0';
	return p + 1;
}

//找到最底层的目录名 保存在name,如 xxxx/aaa/ccc 则保存ccc
static char * get_the_file_name( const char *dir, char * name)
{
	const char * position = NULL;
	if(name == NULL || dir == NULL)
		return NULL;	
	while(*dir)
	{
		if(*dir  == '/')
		 	position = dir;
		dir++;
	}
	if(position == NULL)
	{
		strcpy(name, dir);
	}
	else
	{
		strcpy(name, position+1);
	}
	return name;
}

//描诉文件romfs系统类型的结构体中 namei函数
struct inode * simple_romfs_namei(struct super_block *block, char *dir)
{
	struct inode *inode;
	struct romfs_inode *p;
	unsigned int temp, next,num;
	char name[ROMFS_MAX_FILE_NAME], fname[ROMFS_MAX_FILE_NAME];
	unsigned int max_p_size = (ROMFS_MAX_FILE_NAME + sizeof(struct romfs_inode));
	
        if( block == NULL || dir == NULL )
        {
                return NULL;
        }

	max_p_size = max_p_size>(block->device->sector_size)\
		     ?max_p_size:(block->device->sector_size);
	
	get_the_file_name(dir,fname);   //获取目录下最底层的文件名给fname   目录也文件
	if( ( p = (struct romfs_inode *)kmalloc(max_p_size) ) == NULL )
		goto ERR_OUT_NULL;
	
	
	dir = bmap(name, dir); //name为目录最顶层， dir为余下部分
	
	if(block->device->dout(block->device, p,0,block->device->sector_size))  //读文件的512字节给p
		goto ERR_OUT_KMALLOC;
	
	next = romfs_get_first_file_header(p);// 获取第一个信息头
	temp = next;
	
	while(1)
	{
		
		if(temp >= block->device->storage_size)  //判断地址是否超出存储范围
			goto ERR_OUT_KMALLOC;
		if(temp != 0)
		{
			if(block->device->dout(block->device, p,temp,block->device->sector_size))
				goto ERR_OUT_KMALLOC;
			if(!strcmp(p->name, name))  //当前p指向的文件名是否是name
			{
				if(!strcmp(name, fname)) // 当前的name是不是最终的文件名	如果是则p就是要找的文件，如果不是则p切换到下一层目录继续找
				{	
					goto FOUND_FILE;   //只有aaa只有文件名的这种情况才会进这里， 但是/xxx/aaa这种情况的话的fname不会进入这里
						     //需要到else里面进一步判断 而/xxx/aaa/这种情况也不会进这里 fname为nul，需要进一步判断
				}
				else
				{
					dir = bmap(name, dir);
					next = p->spec;
					if(dir == NULL)//判断dir是不是NULL如果是代表找到文件的上一层目录 对应/XXX/AAA这种情况则进入next进一步查找
						goto FOUND_FILE_DIR;
					else if( *dir == '\0') //这是对应/xxx/aaa/这种情况
					{
						SendStr_VIR("It's not a file\n");
						goto ERR_OUT_KMALLOC;
					}
						
				}
			}
			else
			{
				next = p->next;
			}
		}
		else
		{
			SendStr_VIR("Cann't find the file\n");
			goto ERR_OUT_KMALLOC;
		}
		temp = (be32_to_le32(next)) & ROMFS_NEXT_MASK; //**将下一个地址由大端转化为小端，并且去掉后四位，因为对其问题，其次后四位代表文件类型和权限
	}

FOUND_FILE_DIR:
	temp = next;
	while(1)
	{	
		
		if(temp >= block->device->storage_size)  //判断地址是否超出存储范围
			goto ERR_OUT_KMALLOC;
		if(temp != 0)
		{
			if(block->device->dout(block->device, p,temp,block->device->sector_size))
				goto ERR_OUT_KMALLOC;
			if(!strcmp(p->name, name))  //当前p指向的文件名是否是name
			{
				goto FOUND_FILE;
			}	
			else
			{
				next = p->next;
			}
		}
		else
		{
			SendStr_VIR("Cann't find the file\n");
			goto ERR_OUT_KMALLOC;
		}
		temp = (be32_to_le32(next)) & ROMFS_NEXT_MASK; //**将下一个地址由大端转化为小端，并且去掉后四位，因为对其问题，其次后四位代表文件类型和权限
	}
		
FOUND_FILE: //找到文件后就填充描诉文件的那个inode结构体
	if((inode = (struct inode *)kmalloc(sizeof(struct inode ))) == NULL )
		goto ERR_OUT_KMALLOC;
	num = strlen(p->name);
	if( ( inode->name = (char *)kmalloc(num + 1) == NULL ))
		goto ERR_OUT_KMEM_CACHE_ALLOC;
	strcpy(inode->name, p->name);
	inode->dsize = be32_to_le32(p->size);
	inode->daddr = temp;
	inode->super = &romfs_super_block;
	kfree(p);
	return inode;	
	
		
ERR_OUT_KMEM_CACHE_ALLOC:
	kfree(inode);   //把ERR_OUT_KMEM_CACHE_ALLOC放在ERR_OUT_KMALLOC前面是由原因的因为goto ERR_OUT_KMEM_CACHE_ALLOC；执行完
			//因为没有跳出指令有点像switch中的break一样，就会接着往下执行。所以因该放在前面
ERR_OUT_KMALLOC:
	kfree(p);
ERR_OUT_NULL:
	return NULL;				
}

//从inode中获取文件数据开始的位置
unsigned int romfs_get_daddr(struct inode *node)
{
	int name_size = strlen(node->name);
	return romfs_get_file_data_offset(node->daddr,name_size);
}
//填充fs.h中fs_type[ROMFS]结构体
struct super_block romfs_super_block = 
{
	.namei = simple_romfs_namei,
	.get_daddr = romfs_get_daddr,
	.name = "romfs",
};

int romfs_init(void)
{
	int ret;
	ret = register_file_system(&romfs_super_block, ROMFS);
	romfs_super_block.device = storage[RAMDISK];
	return ret;
}


	


