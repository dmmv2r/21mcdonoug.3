//Author: Donovan McDonough
//Date: 3/14/2021
//producer.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>
#include <semaphore.h>
#include <pthread.h>
#include <time.h>

//shared memory struct
struct shmem {
   pid_t pgid;
   int clockSec;
   int item;
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

bufferItem buffer[BUFFER_SIZE];

int insert(bufferItem item);
void signalHandler(int sig);


int main(int argc, char* argv[]) {
   printf("IN PRODUCER\n");

   //shared memory
   key_t key;
   int shmid;
   struct shmem* ptr;
   signal(SIGINT, signalHandler);

   if((key = ftok(".", 'B')) == -1) {
      perror("producer ftok");
      exit(1);
   }
   if((shmid = shmget(key, 0, 0)) == -1) {
      perror("producer shmget");
      exit(1);
   }
   if((ptr = shmat(shmid, NULL, 0)) == (void*)-1) {
      perror("producer shmat");
      exit(1);
   }


   //producer
   sleep(rand() % 5 + 1); //sleeps for random time between 1 and 5 seconds
   
   FILE* fp;

   ptr->item = rand();
   fp = fopen(ptr->file, "w");

   sem_wait(&ptr->empty);

   printf("producing\n");
   fprintf(fp, "Item Produced: %d -- Time: %d -- Producer: %d\n", ptr->item, ptr->clockSec, ptr->prodNum);

   sem_post(&ptr->empty);



/*
   bufferItem item;

   while(1) {
      sleep(rand() % 5 + 1);

      item = rand();
      
      if(insert(item)) {
         printf("ERROR\n");
      } else {
         printf("Producer successful: %d\n", item);
      }
   }
*/


   fclose(fp);
   printf("LEAVING PRODUCER\n");
   exit(1);
}

void signalHandler(int sig) {
   if(sig == SIGINT) {
      exit(1);
   }
}

/*
int insert(bufferItem item) {
   int success;
   sem_wait(&empty);
   pthread_mutex_lock(&mutex);

   if(count != BUFFER_SIZE) {
      buffer[in] = item;
      in = (in + 1) % BUFFER_SIZE;
      count++;
      success = 0;
   } else {
      success = -1;
   }

   pthread_mutex_unlock(&mutex);
   sem_post(&full);

   return success;
}
*/

