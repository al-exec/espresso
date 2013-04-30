#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "espresso_jpeg.h"
#include "espresso_sync.h"
#include "espresso_pthread.h"
#define PROTO_LEN 1000000


int main(int argc, char **argv)
{
	void *shared = alloc_shmem((size_t)PROTO_LEN);
	if (!shared)
		exit(-1);
	
	void *info_ptr = shared + (PROTO_LEN - sizeof(struct sem_pack) - sizeof(struct child_info));

	struct sem_pack *pack = init_sempack(shared, (PROTO_LEN - sizeof(struct sem_pack)), 
						1, 1, 0);
	if (!pack)
		exit(-1);


	struct jpeg_info *jpg = create_jpeg_from_file("../tt.jpg");
	jpeg_to_buf(jpg, shared);

	pid_t pid = fork();

	if (pid ==-1)
	{
		perror("fork");
		exit(-1);
	}
	else if(pid == 0)
	{
		int i;

	//	unsigned char *child_buf = (unsigned char *) malloc(sizeof(unsigned char) * 65535 * jpg->num_sections);

		struct child_info config;
		sleep(5);
		ChildGetLock(&(pack->mem_ready), &(pack->mem_lock))
		printf("child now reading jpeg:\n");
		for(i = 0; i < 660; i++)
		{
			printf("%02x ", (((unsigned char *)shared)[i]));
		}
		ChildUnlock(&(pack->mem_lock), &(pack->mem_done))
		for(;;);
	}
	else if (pid)
	{
		pthread_t th;
		start_thread(&th, &pid);
		ParentGetLock(&(pack->mem_done), &(pack->mem_lock))
		//parent_set_session_info(info_ptr, 9, 9);
		ParentUnlock(&(pack->mem_lock), &(pack->mem_ready))
		for(;;);
	}
	
	return 0;
}


