#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include "espresso_jpeg.h"
#include "espresso_sync.h"
#include "espresso_pthread.h"
#define MemOpen(x, y) x.fp = fmemopen(x.shared, y, "rwb");
int do_fork(int recover, struct espresso_state *sptr);

void f()
{
	int i = 0;
	char buf[9999999];
	for(i = 0; i < sizeof(buf); i++)
		buf[i] = 'A';

	
}



int main(int argc, char **argv)
{
	struct espresso_state e_state;
	memset(&e_state, 0, sizeof(struct espresso_state));
        struct jpeg_info *jpg = create_jpeg_from_file("../tt.jpg");

	e_state.max_size = sizeof(unsigned char) * jpg->num_sections * 65535;
	e_state.shared = alloc_shmem(e_state.max_size);

	memset(e_state.shared, 0, e_state.max_size);

	MemOpen(e_state, jpg->jpeg_length_total)
	
	jpeg_to_buf(jpg, e_state.shared);


	do_fork(0, &e_state);
	return 0;

}

int do_fork(int recover, struct espresso_state *sptr)
{
	int s;
	int *ps = &s;

        pid_t pid = fork();

        if (pid ==-1)
        {
                perror("fork");
                exit(-1);
        }
        else if(pid == 0)
        {
                sleep(5);
		f();
        }
        else if (pid)
        {
                pthread_t th;
                start_thread(&th, &pid);
		if (pthread_join(th, (void **)(&ps)) != 0)
		{		
			perror("pthread_join");
			exit(-1);
		}

		do_fork(1, sptr);
        }
return 0;

}
