
#ifndef __STRING_H__
#define __STRING_H__

#ifndef NULL 
#define NULL ((void *)0)
#endif

//这里加static 是因为 这是一个.h文件     加static可以保证其他.c文件 包含这个头文件的，调用函数不发生冲突
static inline int strcmp(const char * src, const char * dest)
{
	int ret = 0;

	if(src == NULL || dest == NULL)
		return -2;
	
	while( !(ret = (*src++ - *dest)) && *dest++ );
	
	if(ret < 0)
		return -1;
	else if( ret > 0)
		return 1;
	return 0;
}

static inline void * memset(void * buf, int c, unsigned int count)
{
	char * p = (char *)buf;

	if(buf == NULL)
		return NULL;
	
	while(count-- > 0)
	{
		*p++ = (char)c;
	}
	return buf;
}

static inline char * strcpy(char *dest, const char *src)
{
	char * temp = dest;
	if(dest == NULL || src == NULL)
		return NULL;
	while(*dest++ = *src++);
	return temp;
}

static inline char * strncpy(char *dest, const char *src,unsigned int count)
{
	char * temp = dest;
	if(dest == NULL || src == NULL )
		return NULL;
	while(count-- && (*dest++ = *src++));//这里要保证复制过程中有没有越界
	return temp;
}

static inline int strlen(const char *src)
{
	const char * temp = src;
	if(src == NULL)
		return -1;
	while(*src++);
	return src - temp - 1;
}

static inline char * strchr(const char *src, const char c)
{
	if(src == NULL)
		return NULL;
	
	while(*src)
	{	
		if(*src == c)
			return src;
		src++;
	}
	return NULL;
}
#endif
