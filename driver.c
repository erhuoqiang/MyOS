#include"storage.h"

struct storage_device * storage[MAX_STORAGE_DEVICE];

//注册存储设备
int register_storage_device(struct storage_device * sd, unsigned int num)
{
	if(sd == NULL)
		return 0;
	if(storage[num] == NULL)
		storage[num] = sd;
	else
		return -1;
	return 1;
}

int unregister_storage_device(unsigned int num)
{
	if(storage[num] == NULL)	
		return;	
	else
	  	storage[num] = NULL;
}
