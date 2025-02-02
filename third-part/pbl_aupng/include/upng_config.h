/* if enabled, loading png's from file are supported */
#define UPNG_USE_STDIO

#ifdef UPNG_MEM_MGR
//external defined alloc function
extern void* upng_mem_alloc(unsigned long size,const char * file,int line);
extern void upng_mem_free(void *p);
#define UPNG_MEM_ALLOC(size) upng_mem_alloc(size,__FILE__,__LINE__)
#define UPNG_MEM_FREE upng_mem_free
#else
#define UPNG_MEM_ALLOC(size) malloc(size)
#define UPNG_MEM_FREE free
#endif//UPNG_MEM_MGR


