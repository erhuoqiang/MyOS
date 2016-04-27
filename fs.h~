#include"storage.h"

#define MAX_SUPER_BLOCK (8)   //系统允许注册的文件系统种类
#define ROMFS	0        //ROMFS文件系统的ID

struct super_block;
extern struct super_block *fs_type[];
//描诉文件的结构体  所有文件的共同需要描诉的部分
struct inode
{
	char * name;  //文件名
	unsigned int flags; //控制标志位
	size_t dsize; //文件大小
 	unsigned int daddr; //在存储器的位置
	struct super_block * super;  
};

//描诉文件类型的结构体 包括用目录找文件的inode和所属于设备等  所有文件系统的共同需要描诉的部分  先描诉中的 在根据不同文件系统细分描诉
struct super_block
{
	struct inode *( *namei)(struct super_block * super, char *p); //namei 通过文件目录的到文件的inode结构体
	unsigned int (* get_daddr)(struct inode *);  //返回数据所在位置
	struct storage_device * device; 	//文件系统类型所依存的储存设备
	char * name;	//文件系统类型名
};


//上面inode和super_block结构体都是用于描诉和操作文件的 不包含在文件内容里面而是在内存中
