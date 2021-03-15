//Author: Donovan McDonough
//Date: 3/14/2021
//consumer.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <time.h>

//shared memory struct
struct shmem {
   pid_t pid;
   int clockSec;
   int numProd;
   int numCons;
   int prodNum;
   int consNum;
   char* file;
   sem_t empty;
   sem_t full;
};

#define BUFFER_SIZE 5
typedef int bufferItem;


void signalHandler(int sig);
void remover(bufferItem *item);


int main(int argc, char* argv[]) {

   printf("IN CONSUMER\n");

   //shared memory
   key_t key;
   int shmid;
   struct shmem* ptr;
   
   signal(SIGINT, signalHandler);

   if((key = ftok(".", 'B')) == -1) {
      perror("consumer ftok");
      exit(1);
   }
   if((shmid = shmget(key, 0, 0)) == -1) {
      perror("consumer shmget");
      exit(1);
   }
   if((ptr = shmat(shmid, NULL, 0)) == (void*)-1) {
      perror("consumer shmat");
      exit(1);
   }

   //consumer
   FILE* fp;

   fp = fopen(ptr->file, "w");

   sem_wait(&ptr->full);

   printf("consuming\n");
   fprintf(fp, "Consuming Item: %d -- Time: %d -- Consumer: %d\n", ptr->item, ptr->clockSec, ptr->consNum);

   sem_post(&ptr->full);

   sleep(srand() % 10 + 1); //sleeps for random time between 1 and 10 seconds
   

/*
   bufferItem item;

   while(1) {
      sleep(rand() % 5 + 1);

      if(remove(&item)) {
         printf("ERROR REMOVING\n");
      } else {
         printf("Consumed %d\n", item);
      }
   }
*/

   fclose(fp);
   printf("LEAVING CONSUMER\n");
   exit(0);
}

void signalHandler(int sig) {
   if(sig == SIGINT) {
      exit(1);
   }
}

/*
void remover(bufferItem *item) {
   int success;

   sem_wait(&full);
   pthread_mutex_lock(&mutex);

   if(count != 0) {
      *item = buffer[out];
      out = (out + 1) % BUFFER_SIZE;
      count--;
      success = 0;
   } else {
      success = -1;
   }

   pthread_mutex_unlock(&mutex);
   sem_post(&empty);

   return success;
}
*/
