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
struct dataIn *dataIn;

//Struct passed to threads
struct dataIn {
    char* stack;
    char* input;    
    char* prod_chunk;
    int done;
    int ticket;
    int now_serving;
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
    //Initialize struct char* arrays
    dataIn->stack = malloc(file_size);
    dataIn->prod_chunk = malloc(10);
    dataIn->input = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    //Initialize struct ints
    dataIn->done = 0;
    dataIn->ticket = 0;
    dataIn->now_serving = 0;
    dataIn->current_stack_element = 0;
    dataIn->on_stack = 0;
    //Initialize struct locks and condition variables
    lock_init(&dataIn->producer_mutex);
    lock_init(&dataIn->consumer_mutex);
    cond_init(&dataIn->producer_cond);
    cond_init(&dataIn->consumer_cond);
    //Initialize struct semaphores 
    if(sem_init(&dataIn->mutex_sem, 0, 1) == -1){exit(-1);}
    if(sem_init(&dataIn->producer_sem, 0, 3) == -1){exit(-1);}
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
    struct dataIn *dataIn = (struct dataIn*)arg;
    //Acquire ticket and data chunk to compress
    lock_acquire(&dataIn->producer_mutex);
    int my_ticket = dataIn->ticket;
    dataIn->ticket++;
    char* src = dataIn->prod_chunk;
    lock_release(&dataIn->producer_mutex);
    //Compress given data
    src = RLECompress(src);
    //Make sure to place the data in the proper location
    while(my_ticket != dataIn->now_serving)
    {   
        cond_wait(&dataIn->producer_cond, &dataIn->producer_mutex);
    }
    //Place compressed data in stack and let next data chunk be placed
    dataIn->stack[my_ticket] = src;
    dataIn->now_serving++;
    cond_signal(&dataIn->producer_cond, &dataIn->producer_mutex);
    cond_signal(&dataIn->consumer_cond, &dataIn->consumer_mutex);
    sem_post(&dataIn->producer_sem);
    return NULL;
};

void*
consumer(void *arg) {
    struct dataIn *dataIn = (struct dataIn*)arg;);
    //Allow for Consumer thread to continue running until the end of the program
    while(dataIn->done != 0){
        //Check for the proper number of elements on the stack
        while(dataIn->on_stack < 2){
            cond_wait(&dataIn->consumer_cond, &dataIn->consumer_mutex);
        }
        if(dataIn->done != 0){
            sem_wait(&dataIn->mutex_sem);
            //Cutoff correction
            char* first = dataIn->stack[0];
            char* second = dataIn->stack[1];
            if(first[(sizeof(first) - 2)] == second[0]){
                int first_last = first[(sizeof(first) - 1)] - '0';
                int second_first = second[1] - '0'
                first[sizeof(first) - 1] = (char) (first_last + second_first);
                memcpy(second, (second + 5), (sizeof(second) - 5));
                dataIn->stack[1] = second;
            }
            fwrite(first, sizeof(first), 1, stdout);
            //Move every stack element up 1
            memcpy(dataIn->stack, (dataIn->stack + 1), sizeof(stack + 1));
            dataIn->on_stack--;
            sem_post(&dataIn->mutex_sem);
        }
        
    }
    //Once producers have finished, consumer runs on every element of the stack
    //left
    while(dataIn->on_stack > 1){
        char* first = dataIn->stack[0];
        char* second = dataIn->stack[1];
        if(first[(sizeof(first) - 2)] == second[0]){
            int first_last = first[(sizeof(first) - 1)] - '0';
            int second_first = second[1] - '0'
            first[sizeof(first) - 1] = (char) (first_last + second_first);
            memcpy(second, (second + 5), (sizeof(second) - 5));
            dataIn->stack[1] = second;
        }
        fwrite(chunk, sizeof(chunk), 1, stdout); 
        memcpy(dataIn->stack, (dataIn->stack + 1), sizeof(stack + 1));
        dataIn->on_stack--;
    }
    fwrite(dataIn->stack[0], sizeof(dataIn->stack[0]), 1, stdout);
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
    //Check for proper number of arguments and that the file given can be found
    if(argc > 2){argument_overload_error(argc, argv);}

    if(access(argv[1], F_OK) != 0){file_not_found_error(argv[1]);}

    fd = open(argv[1], O_RDONLY);
    if(fstat(fd, &sb) == -1)
    {fprintf(stderr, "Problem reading file. \n"); exit(1);}
    file_size = sb.st_size;

    //Initialize data struct
    dataIn = malloc(sizeof(*dataIn));
    dataIn_init(dataIn);

    //Create consumer thread, which will run continuously 
    if(pthread_create(&cons_tid, NULL, consumer, &dataIn) != 0)
    {
    	    fprintf(stderr, "Failed to create consumer thread. \n");
    	    exit(1);
    }

    int j = 0;
    for(int i = 0; i < file_size; i += 2){
            lock_acquire(&dataIn->producer_mutex);
            //Read set number of bytes from input file into prod_chunk, 
            //starting at offset i
            memcpy(dataIn->prod_chunk, (dataIn->input + i), 2);
            lock_release(&dataIn->producer_mutex);
            sem_trywait(&dataIn->producer_sem);
            if(pthread_create(&prod_tid[j], NULL, producer, &dataIn) != 0)
	    {
	    	fprintf(stderr, "Failed to create producer thread. \n");
	    	exit(1);
	    }
        if(j == 2){j = 0;}
        else j++;
        
    }


    //Make sure all Producer threads finish executing
    //Last parts of input file
    if (pthread_join(prod_tid[0], NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}
    if (pthread_join(prod_tid[1], NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}
    if (pthread_join(prod_tid[2], NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}

    sem_close(&dataIn->producer_sem);
    dataIn->done = 1;
    //Allow for consumer thread to finish writing compressed data to stdout
    if (pthread_join(cons_tid, NULL) != 0)
        {fprintf(stderr, "Failure in pthread_join for producer thread. \n");
        exit(1);}

    //Clean up
    munmap(dataIn->input, file_size);
    sem_close(&dataIn->mutex_sem);
    free(dataIn);

    close(fd); 
}
