#include <stdio.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <search.h>

#define EXPECTED_ARGS (1 + 4)
#define RWP 0666 // read and write permissions for everyone

// shared memory variables
int msg_count_svar;
int queue_svar;
int returned_svar;
int hitched_svar;
int closed_svar;

int *msg_count_svar_ptr;
int *queue_svar_ptr;
int *returned_svar_ptr;
int *hitched_svar_ptr;
int *closed_svar_ptr;

// access to shared variables
sem_t *msg_count_sem;
sem_t *queue_sem;
sem_t *returned_sem;
sem_t *hitched_sem;
sem_t *closed_sem;

// access to output file
sem_t *output_sem;

// santa's activities
sem_t *reindeer_sem;
sem_t *elf_sem;
sem_t *santa_sem;

typedef struct
{
    int elves_count;
    int reindeer_count;
    int max_work_time;
    int max_holiday_time;
} args_t;

int args_processing(args_t *args, int argc, char *argv[])
{
    if (argc != EXPECTED_ARGS)
    {
        fprintf(stderr, "Wrong number of arguments.\n");
        return 1;
    }
    char *endptr;
    args->elves_count = (int)strtol(argv[1], &endptr, 10);
    if (*endptr)
    {
        fprintf(stderr, "Conversion of 1st argument to number failed.\n");
        return 1;
    }
    args->reindeer_count = (int)strtol(argv[2], &endptr, 10);
    if (*endptr)
    {
        fprintf(stderr, "Conversion of 2nd argument to number failed.\n");
        return 1;
    }
    args->max_work_time = (int)strtol(argv[3], &endptr, 10);
    if (*endptr)
    {
        fprintf(stderr, "Conversion of 3rd argument to number failed.\n");
        return 1;
    }
    args->max_holiday_time = (int)strtol(argv[4], &endptr, 10);
    if (*endptr)
    {
        fprintf(stderr, "Conversion of 4th argument to number failed.\n");
        return 1;
    }
    if (0 < args->reindeer_count && 0 < args->elves_count && 0 <= args->max_holiday_time && 0 <= args->max_work_time &&
        20 > args->reindeer_count && 1000 > args->elves_count && 1000 >= args->max_holiday_time && 1000 >= args->max_work_time)
    {
        return 0;
    }
    else
    {
        fprintf(stderr, "Wrong values of arguments.\n");
        return 1;
    }
}

void semaphores_delete()
{
    sem_destroy(msg_count_sem);
    sem_destroy(queue_sem);
    sem_destroy(returned_sem);
    sem_destroy(hitched_sem);
    sem_destroy(output_sem);
    sem_destroy(reindeer_sem);
    sem_destroy(elf_sem);
    sem_destroy(santa_sem);
    sem_destroy(closed_sem);
}

void shared_memory_clear()
{
    // shmdt - detaches memory
    // shmctl - System V shared memory control
    // IPC_RMID  Mark the segment to be destroyed.  The segment will actually be destroyed only after
    // the last process detaches it, The caller must be the owner or creator of the segment, or be pri
    // vileged.  The buf (3rd argument) is ignored.
    shmdt(msg_count_svar_ptr);
    shmctl(msg_count_svar, IPC_RMID, NULL);
    shmdt(queue_svar_ptr);
    shmctl(queue_svar, IPC_RMID, NULL);
    shmdt(returned_svar_ptr);
    shmctl(returned_svar, IPC_RMID, NULL);
    shmdt(hitched_svar_ptr);
    shmctl(hitched_svar, IPC_RMID, NULL);
    shmdt(closed_svar_ptr);
    shmctl(closed_svar, IPC_RMID, NULL);
}

/*
int semaphore_create(sem_t* sem) {
    int error; //auxiliary variable for checking failures of semaphore initialization

    //mmap()  creates  a  new  mapping  in  the virtual address space of the calling process
    //If  1st argument  is NULL, kernel chooses address at which to create the mapping;
    //                     this is the most portable method of creating a new mapping.
    //2nd length of mapping, 3rd memory protection, 4th flags, 5th file descriptor, in examples just 0, 6th is offset
    //sem_init - initialize semaphore (1st is name, 2nd is 1 because i work with processes, 3rd is initial value)

    sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if(sem == MAP_FAILED){
        semaphores_delete();
        fprintf(stderr,"Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(sem, 1, 1);

    if(error == -1){
        semaphores_delete();
        fprintf(stderr,"Error. Creating semaphore filed.\n");
        return 1;
    }
    return 0;
}
int shared_memory_create(int *svar, int *svar_ptr) {
    //shmget - allocates a System V shared memory segment
    (*svar) = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | RWP);
    if ((*svar) == -1) {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }

    //shmat() attaches the System V shared memory segment identified by shmid(1st arg) to the address space of the
    // calling process.
    //If  shmaddr(second argument)  is  NULL, the system chooses a suitable (unused) page-aligned address to attach
    //the segment.
    //RETURN:On  success,  shmat()  returns  the  address  of  the attached shared memory segment; on error,
    //       (void *) -1 is returned
    //NOTE: Using  shmat()  with shmaddr equal to NULL is the preferred, portable way of attaching a shared
    //      memory segment.
    svar_ptr = shmat((*svar), NULL, 0);
    if (svar_ptr == (void *) -1) {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }
    return 0;
}*/

int semaphores_create()
{
    /*
    if(semaphore_create(msg_count_sem)     ){ return 1; }
    if(semaphore_create(queue_sem)         ){ return 1; }
    if(semaphore_create(returned_sem)      ){ return 1; }
    if(semaphore_create(hitched_sem)       ){ return 1; }
    if(semaphore_create(output_sem)        ){ return 1; }
    if(semaphore_create(reindeer_sem)      ){ return 1; }
    if(semaphore_create(elf_sem)       ){ return 1; }
    if(semaphore_create(santa_sem)      ){ return 1; }
     */
    int error; // auxiliary variable for checking failures of semaphore initialization

    // mmap()  creates  a  new  mapping  in  the virtual address space of the calling process
    // If  1st argument  is NULL, kernel chooses address at which to create the mapping;
    //                      this is the most portable method of creating a new mapping.
    // 2nd length of mapping, 3rd memory protection, 4th flags, 5th file descriptor, in examples just 0, 6th is offset
    // sem_init - initialize semaphore (1st is name, 2nd is 1 because i work with processes, 3rd is initial value)

    // msg_count_sem
    msg_count_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (msg_count_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(msg_count_sem, 1, 1);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // output_sem
    output_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (output_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(output_sem, 1, 1);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // queue_sem
    queue_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (queue_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(queue_sem, 1, 1);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // returned_sem
    returned_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (returned_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(returned_sem, 1, 1);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // hitched_sem
    hitched_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (hitched_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(hitched_sem, 1, 1);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // reindeer_sem
    reindeer_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (reindeer_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(reindeer_sem, 1, 0);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // elf_sem
    elf_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (elf_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(elf_sem, 1, 0);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // santa_sem
    santa_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (santa_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(santa_sem, 1, 0);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    // closed_sem
    closed_sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (closed_sem == MAP_FAILED)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }
    error = sem_init(closed_sem, 1, 1);
    if (error == -1)
    {
        semaphores_delete();
        fprintf(stderr, "Error. Creating semaphore filed.\n");
        return 1;
    }

    return 0;
}

int shared_memories_create()
{
    /*
    if(shared_memory_create(&msg_count_svar, msg_count_svar_ptr)  ) { return 1; }
    if(shared_memory_create(&queue_svar, queue_svar_ptr)          ) { return 1; }
    if(shared_memory_create(&returned_svar, returned_svar_ptr)    ) { return 1; }
    if(shared_memory_create(&hitched_svar, hitched_svar_ptr)      ) { return 1; }
    */
    // shmget - allocates a System V shared memory segment
    msg_count_svar = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | RWP);
    if (msg_count_svar == -1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }

    // shmat() attaches the System V shared memory segment identified by shmid(1st arg) to the address space of the
    //  calling process.
    // If  shmaddr(second argument)  is  NULL, the system chooses a suitable (unused) page-aligned address to attach
    // the segment.
    // RETURN:On  success,  shmat()  returns  the  address  of  the attached shared memory segment; on error,
    //        (void *) -1 is returned
    // NOTE: Using  shmat()  with shmaddr equal to NULL is the preferred, portable way of attaching a shared
    //       memory segment.
    msg_count_svar_ptr = shmat(msg_count_svar, NULL, 0);
    if (msg_count_svar_ptr == (void *)-1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }

    queue_svar = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | RWP);
    if (queue_svar == -1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }
    queue_svar_ptr = shmat(queue_svar, NULL, 0);
    if (queue_svar_ptr == (void *)-1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }

    returned_svar = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | RWP);
    if (returned_svar == -1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }
    returned_svar_ptr = shmat(returned_svar, NULL, 0);
    if (returned_svar_ptr == (void *)-1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }

    hitched_svar = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | RWP);
    if (hitched_svar == -1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }
    hitched_svar_ptr = shmat(hitched_svar, NULL, 0);
    if (hitched_svar_ptr == (void *)-1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }

    closed_svar = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | RWP);
    if (closed_svar == -1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }
    closed_svar_ptr = shmat(closed_svar, NULL, 0);
    if (closed_svar_ptr == (void *)-1)
    {
        shared_memory_clear();
        semaphores_delete();
        fprintf(stderr, "Error. Cannot get shared memory.\n");
        return 1;
    }

    return 0;
}
void santa_main(FILE *output, args_t args)
{
    sem_wait(output_sem);
    sem_wait(msg_count_sem);
    (*msg_count_svar_ptr)++;
    fprintf(output, "%d: Santa: going to sleep\n", (*msg_count_svar_ptr));
    fflush(NULL);
    sem_post(msg_count_sem);
    sem_post(output_sem);

    while (1)
    {
        sem_wait(santa_sem);

        // woken up by reindeer
        sem_wait(returned_sem);
        if ((*returned_svar_ptr) == -1)
        {
            break;
        }
        sem_post(returned_sem);
        // woken up by elves

        sem_wait(output_sem);
        sem_wait(msg_count_sem);
        (*msg_count_svar_ptr)++;
        fprintf(output, "%d: Santa: helping elves\n", (*msg_count_svar_ptr));
        fflush(NULL);
        sem_post(msg_count_sem);
        sem_post(output_sem);

        sem_post(elf_sem);
        sem_wait(santa_sem);
        sem_post(elf_sem);
        sem_wait(santa_sem);
        sem_post(elf_sem);
        sem_wait(santa_sem);

        //=========
        sem_wait(closed_sem);
        sem_wait(output_sem);
        sem_wait(msg_count_sem);
        //---
        (*msg_count_svar_ptr)++;
        fprintf(output, "%d: Santa: going to sleep\n", (*msg_count_svar_ptr));
        fflush(NULL);
        (*closed_svar_ptr) = 0;
        //---
        sem_post(msg_count_sem);
        sem_post(output_sem);
        sem_post(closed_sem);
        //============
    }
    sem_wait(closed_sem);
    sem_wait(output_sem);
    sem_wait(msg_count_sem);
    (*msg_count_svar_ptr)++;
    fprintf(output, "%d: Santa: closing workshop\n", (*msg_count_svar_ptr));
    fflush(NULL);
    (*closed_svar_ptr) = 2;
    for (int i = 0; i < args.elves_count; i++)
    {
        sem_post(elf_sem);
    }
    sem_post(msg_count_sem);
    sem_post(output_sem);
    sem_post(closed_sem);

    for (int i = 0; i < args.reindeer_count; i++)
    {
        sem_post(reindeer_sem);
        sem_wait(santa_sem);
    }
    exit(0);
}

void elf_main(FILE *output, int elfs_number, args_t args)
{

    sem_wait(output_sem);
    /**/ sem_wait(msg_count_sem);
    /******/ (*msg_count_svar_ptr)++;
    /******/ fprintf(output, "%d: Elf %d: started\n", (*msg_count_svar_ptr), elfs_number);
    /******/ fflush(NULL);
    /**/ sem_post(msg_count_sem);
    sem_post(output_sem);

    while (1)
    {

        // work alone duration
        if (args.max_work_time != 0)
        {
            time_t tim;
            srand((unsigned)time(&tim));
            usleep(rand() % args.max_work_time);
        }

        // go to queue
        //========
        sem_wait(queue_sem);
        sem_wait(output_sem);
        sem_wait(msg_count_sem);
        //---
        (*msg_count_svar_ptr)++;
        fprintf(output, "%d: Elf %d: need help\n", (*msg_count_svar_ptr), elfs_number);
        fflush(NULL);
        (*queue_svar_ptr)++;
        //---
        sem_post(msg_count_sem);
        sem_post(output_sem);
        sem_post(queue_sem);
        //========

        // check conditions
        sem_wait(queue_sem);
        sem_wait(closed_sem);
        if ((*closed_svar_ptr) == 2)
        {
            sem_post(queue_sem);
            sem_post(closed_sem);
            break;
        }
        else if ((*closed_svar_ptr) == 0 && (*queue_svar_ptr) >= 3)
        { // if santa sleeps and 3+ elves are waiting
            (*queue_svar_ptr) -= 3;
            (*closed_svar_ptr) = 1;
            sem_post(closed_sem);
            sem_post(queue_sem);

            sem_post(santa_sem);
            //*******
        }
        else
        {
            sem_post(closed_sem);
            sem_post(queue_sem);
        }

        sem_wait(elf_sem); // go to sleep until santa comes to help

        sem_wait(closed_sem);
        if ((*closed_svar_ptr) == 2)
        {
            sem_post(closed_sem);
            break;
        }
        else
        {
            sem_post(closed_sem);

            sem_wait(output_sem);
            /**/ sem_wait(msg_count_sem);
            /******/ (*msg_count_svar_ptr)++;
            /******/ fprintf(output, "%d: Elf %d: get help\n", (*msg_count_svar_ptr), elfs_number);
            /******/ fflush(NULL);
            sem_post(santa_sem);
            /**/ sem_post(msg_count_sem);
            sem_post(output_sem);
        }
    }
    // as soon as they exit the cycle, they go on vacation
    sem_wait(output_sem);
    /**/ sem_wait(msg_count_sem);
    /******/ (*msg_count_svar_ptr)++;
    /******/ fprintf(output, "%d: Elf %d: taking holidays\n", (*msg_count_svar_ptr), elfs_number);
    /******/ fflush(NULL);
    /**/ sem_post(msg_count_sem);
    sem_post(output_sem);

    exit(0);
}

void reindeer_main(FILE *output, int reindeers_number, args_t args)
{
    // RD start
    sem_wait(output_sem);
    sem_wait(msg_count_sem);
    /**/ (*msg_count_svar_ptr)++;
    /**/ fprintf(output, "%d: RD %d: rstarted\n", (*msg_count_svar_ptr), reindeers_number);
    /**/ fflush(NULL);
    sem_post(msg_count_sem);
    sem_post(output_sem);

    // holiday duration
    if (args.max_holiday_time != 0)
    {
        time_t tim;
        srand((unsigned)time(&tim));
        usleep(rand() % args.max_holiday_time);
    }

    // return of the RDv
    while (1)
    {
        sem_wait(returned_sem);
        sem_wait(output_sem);
        sem_wait(msg_count_sem);
        /**/ (*msg_count_svar_ptr)++;
        /**/ fprintf(output, "%d: RD %d: return home\n", (*msg_count_svar_ptr), reindeers_number);
        /**/ fflush(NULL);
        (*returned_svar_ptr)++;
        sem_post(output_sem);
        sem_post(msg_count_sem);
        if ((*returned_svar_ptr) == args.reindeer_count)
        {
            (*returned_svar_ptr) = -1;
            sem_post(returned_sem);
            sem_post(santa_sem);
            sem_wait(reindeer_sem);
            break;
        }
        sem_post(returned_sem);

        // waiting for hitching
        sem_wait(reindeer_sem);
        break;
    }

    //==========
    sem_wait(output_sem);
    sem_wait(msg_count_sem);
    //---
    (*msg_count_svar_ptr)++;
    fprintf(output, "%d: RD %d: get hitched\n", (*msg_count_svar_ptr), reindeers_number);
    fflush(NULL);
    (*hitched_svar_ptr)++;
    sem_post(santa_sem); // wakes up Santa
    //---
    sem_post(msg_count_sem);
    sem_post(output_sem);
    //=========

    exit(0);
}

int main(int argc, char *argv[])
{
    // args processing
    args_t args;
    if (args_processing(&args, argc, argv))
    {
        return 1;
    }

    // preparing output file
    FILE *output = fopen("proj2.out", "w");
    if (output == NULL)
    {
        fprintf(stderr, "File open failed.\n");
        return 1;
    }

    // creating semaphores
    if (semaphores_create())
    {
        return 1;
    }

    // creating shared memory
    if (shared_memories_create())
    {
        return 1;
    }
    // creating processes

    // creating santa
    switch (fork())
    {
    case 0:
        santa_main(output, args);
        return 2;
    case -1:
        fprintf(stderr, "Error during creating santa\n");
        return 1;
    }
    // creating elves
    for (int i = 1; i <= args.elves_count; i++)
    {
        switch (fork())
        {
        case 0:
            elf_main(output, i, args);
            return 2;
        case -1:
            fprintf(stderr, "Error during creating elf\n");
            return 1;
        }
    }
    // creating reindeer
    for (int i = 1; i <= args.reindeer_count; i++)
    {
        switch (fork())
        {
        case 0:
            reindeer_main(output, i, args);
            return 2;
        case -1:
            fprintf(stderr, "Error during creating reindeer\n");
            return 1;
        }
    }

    // wait for all child processes to die
    int x;
    do
    {
        x = wait(NULL);
    } while (x != -1);

    (*msg_count_svar_ptr)++;
    fprintf(output, "%d: Santa: Christmas started\n", (*msg_count_svar_ptr));
    fflush(NULL);

    fclose(output);
    shared_memory_clear();
    semaphores_delete();
    return 0;
}
