#ifndef COMMON_H
#define COMMON_H
#include <stdlib.h>

#define UNIMPLEMENT \
	{printf("unimplement [%s:%d]-->%s\n", __FUNCTION__, __LINE__, __FILE__); \
	exit(-1);}while(0)
	

#endif

#define MY_DEVELOP 1

