#include "espresso_sync.h"

void child_get_session_info(struct child_info *info, void *ptr)
{
	info->cont = ((struct child_info *)(ptr))->cont;
	info->iterations = ((struct child_info *)(ptr))->iterations;
	
}

void parent_set_session_info(void *ptr,
				unsigned int cont, int num)
{
	struct child_info *c_ptr = (struct child_info *)(ptr);
	c_ptr->cont = cont;
	c_ptr->iterations = num;
	
}

struct sem_pack *init_sempack(void *ptr, int offset, int l, int d, int r)
{
	struct sem_pack *pack = (struct sem_pack *)(ptr + offset);
	
	if ((sem_init(&(pack->mem_lock), 1, l)) == -1)
	{
		perror("sem_init");
		return NULL;
	}

	if ((sem_init(&(pack->mem_done), 1, d)) == -1)
	{
		perror("sem_init");
		return NULL;
	}

	if ((sem_init(&(pack->mem_ready), 1, r)) == -1)
	{
		perror("sem_init");
		return NULL;
	}

	return pack;
}


void *alloc_shmem(size_t length)
{
	void *ptr = mmap(NULL, length, PROT_READ|PROT_WRITE, 
				MAP_SHARED|MAP_ANONYMOUS, -1, 0);
	
	if (ptr == MAP_FAILED)
	{
		perror("mmap");
		return NULL;
	}
	
	return ptr;
}
