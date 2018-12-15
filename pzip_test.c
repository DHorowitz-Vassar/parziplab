/*********************************************************
 * Your Name: Wilson Housen
 * Partner Name: Dylan Horowitz
 *********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include <pthread.h>
#include <time.h>
#include <semaphore.h> 

//Including thread.h file from previous thread lab
#include "thread.h"

int fd;
int file_size;
struct stat sb;
pthread_t prod_tid[3];
pthread_t cons_tid;

struct dataIn {
    char* stack;
    char* input;    
    char* prod_chunk;
    int done;
    int ticket;
    int current_stack_element;
    int on_stack;

    sem_t mutex_sem;
    sem_t producer_sem; 
    struct lock producer_mutex;
    struct lock consumer_mutex;
    struct condition producer_cond;
    struct condition consumer_cond;
};


void
dataIn_init(struct dataIn *dataIn){
    printf("File_size: %d \n", file_size);
    dataIn->stack = malloc(file_size);
    dataIn->prod_chunk = malloc(10);
    dataIn->input = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    dataIn->done = 0;
    dataIn->ticket = 0;
    dataIn->current_stack_element = 0;
    dataIn->on_stack = 0;
    lock_init(&dataIn->producer_mutex);
    lock_init(&dataIn->consumer_mutex);
    cond_init(&dataIn->producer_cond);
    cond_init(&dataIn->consumer_cond);
    sem_init(&dataIn->mutex_sem, 0, 1);
    sem_init(&dataIn->producer_sem, 0, 3);
};

char* RLECompress(char* src) 
{ 
    int rLen; 
    int len = strlen(src); 
  
    char* dest = (char*)malloc(len * 5);
  
    int i, j = 0; 
  
    /* traverse the input string one by one */
    for (i = 0; i < len; i++) { 
  
        /* Count the number of occurrences of the new character */
        rLen = 1; 
        while (src[i] == src[i + 1]) { 
            rLen++; 
            i++; 
        } 
  
        dest[j] = (char) rLen;
        j = j + 4;
        dest[j] = src[i];
        j = j + 1;
    } 
    return dest;
}

void*
producer(void *arg) {
    printf("Entered producer \n");
    struct dataIn *dataIn = (struct dataIn*)arg;
    if(sem_post(&dataIn->producer_sem) != 0){exit(1);}
    printf("Exiting producer \n");
    return NULL;
};

void*
consumer(void *arg) {
    struct dataIn *dataIn = (struct dataIn*)arg;
    printf("Consumer reached \n");
    return NULL;
};

void argument_overload_error(int argc, char *argv[]){
	fprintf(stderr, "Error: Unexpected arguments: ");
	for(int x = 2; x < argc; x++){fprintf(stderr, " %s ", argv[x]);}
	fprintf(stderr, "\n");
	exit(1);
}

void file_not_found_error(const char *filename){
	fprintf(stderr, "Error: file %s not found. \n", filename);
	exit(1);
}

int main(int argc, char *argv[]) {
    if(argc > 2){argument_overload_error(argc, argv);}

    if(access(argv[1], F_OK) != 0){file_not_found_error(argv[1]);}

    fd = open(argv[1], O_RDONLY);
    if(fstat(fd, &sb) == -1)
    {fprintf(stderr, "Problem reading file. \n"); exit(1);}
    file_size = sb.st_size;
    struct dataIn *dataIn;
    dataIn = malloc(sizeof(*dataIn));
    dataIn_init(dataIn);

    if(pthread_create(&cons_tid, NULL, &consumer, &dataIn) != 0)
    {
    	    fprintf(stderr, "Failed to create consumer thread. \n");
    	    exit(1);
    }

    memcpy(dataIn->prod_chunk, (dataIn->input + 1), 10);

    int j = 0;
    for(int i = 0; i < file_size; i++){
	    printf("In producer for loop \n");
            sem_trywait(&dataIn->producer_sem);
            if(pthread_create(&prod_tid[j], NULL, &producer, &dataIn) != 0)
	    {
	    	fprintf(stderr, "Failed to create producer thread. \n");
	    	exit(1);
	    }
        if(j == 2){j = 0;}
        else j++;
    }


    if (pthread_join(prod_tid[0], NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}
    if (pthread_join(prod_tid[1], NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}
    if (pthread_join(prod_tid[2], NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}
    dataIn->done = 1;
    printf("About to join 2 \n");
    if (pthread_join(cons_tid, NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}
    free(&dataIn->stack);
    munmap(dataIn->input, file_size);
    free(dataIn);

    close(fd);
    printf("Done! \n"); 

}
