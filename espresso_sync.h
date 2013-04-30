#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>

// *** MACROS ***

// this may be a premature optimization, but the goal of this code is to be fast, so, changing
// what used to be  functions into macros helps reduce overhead while we need the cpu most, and
// still provides error checking. it also makes writing the code and testing it much easier.
#define Sem_Wait(x) if (sem_wait(x)) { perror("sem_wait"); exit(-1); }
#define Sem_Post(x) if (sem_post(x)) { perror("sem_post"); exit(-1); }

#define ChildGetLock(x, y) Sem_Wait(x) Sem_Wait(y)
#define ChildUnlock(x, y) Sem_Post(x) Sem_Post(y)

#define ParentGetLock(x, y) Sem_Wait(x) Sem_Wait(y)
#define ParentUnlock(x, y) Sem_Post(x) Sem_Post(y)


// *** DATA STRUCTURES ***


// an instance of this struct will belong to child, and be filled with
// information pertaining to the current scan session. 
struct child_info
{

        // continuing from a previous scan/crash? 
        unsigned int cont;


        // number of iterations set to complete this session
        unsigned long long iterations;
};


// semaphores to be shared between parent and child to synchronize memory
// accesses to generated fuzz strings.
struct sem_pack
{
        sem_t mem_lock;
	sem_t mem_done;
	sem_t mem_ready;
};

struct espresso_state
{
	struct jpeg_info *jpg;
	int last_signal;
	int last_retval;
	FILE *fp;
	void *shared;
	unsigned long iteration;
	unsigned long max_size;
};









// *** FUNCTIONS ***



// allocate shared memory for storing fuzz input.
void *alloc_shmem(size_t length);


// allocate on the shared memory segment three semaphores 
// for synchronizing the parent and child processes. l, d, and r are
// the initial values to set the mem_(l)ock, mem_(d)one, and mem_(r)eady
// semaphores to.
struct sem_pack *init_sempack(void *ptr, int offset, int l, int d, int r);


// called by child before session begins to get session data in order to
// determine session behavior.
void child_get_session_info(struct child_info *info, void *ptr);


// called by parent before session. ^^
void parent_set_session_info(void *ptr, unsigned int cont, int num);

