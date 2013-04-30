#include "espresso_pthread.h"
#ifndef PTHREAD_H
#include <pthread.h>
#endif

void start_thread(pthread_t *tptr, pid_t *c_pid)
{
                int ret = pthread_create(tptr, NULL, child_watch, c_pid);
                if (ret == -1)
                {
                        perror("pthread_create");
                        exit(-1);
                }

}

void *child_watch(void *arg)
{
	int status, signum;
	pid_t pid = *((pid_t *)arg);

	printf("New thread created. PID: %d\n", pid);


	// make sure we're only responding to signals from our child
	waitpid(pid, &status, 0);	

	if (WIFSIGNALED(status))
	{
		signum = WTERMSIG(status);
		printf("pthread_exit called due to signal: %d\n", signum);
		pthread_exit(0);
	}
	else
	{
		printf("unexpected error in thread! exiting!\n");
		exit(-1);
	}


	return NULL;	
}
