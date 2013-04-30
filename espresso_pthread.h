#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
void start_thread(pthread_t *tptr, pid_t *c_pid);
void *child_watch(void *arg);

