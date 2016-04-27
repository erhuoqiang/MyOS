#include "string.h"

#define MAX_SUPER_BLOCK 8

#ifndef NULL
#define NULL ((void *)0)
#endif

struct super_block *fs_type[MAX_SUPER_BLOCK];

//注册文件系统结
int register_file_system(struct super_block * type, unsigned int id)
{
	if(fs_type[id] == NULL )
  	{
		fs_type[id] = type;
		return 0;
	}
	return -1;
}


//注销文件系统
void unregister_file_system(unsigned int id)
{
	fs_type[id] = NULL;
}
