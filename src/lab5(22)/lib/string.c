#include "string.h"


/* memset参数解释:
	void* dst:内存空间首地址
	int c:内存的内容全部初始化为c
	uint64 n:内存长度 
 */
void *memset(void *dst, int c, uint64 n) {
    char *cdst = (char *)dst;
    for (uint64 i = 0; i < n; ++i)
        cdst[i] = c;

    return dst;
}
