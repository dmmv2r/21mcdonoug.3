//Author: Donovan McDonough
//Date: 3/14/2021
//monitor.c

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
#include <stdbool.h>
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


typedef int bufferItem;
#define BUFFER_SIZE 5

int sigNum = 0;

//function for event handler
void sigHandler(int sig);

//function to free shared memory when done
void freeshm(struct shmem* ptr, int shmid);


int main(int argc, char* argv[]) {

   printf("IN MONITOR MAIN\n");

   int opt;
   int producers;
   int consumers;
   int time;
   char* logfile;

   logfile = "logfile";
   producers = 2;
   consumers = 6;
   time = 100;

   //parses command line arguments
   while((opt = getopt(argc, argv, "ho:p:c:t:")) != -1) {
      switch(opt) {
         case 'h':
            printf("Usage: ./monitor [-h] [-o logfile] [-p m] [-c n] [-t time]\n");
            printf("[-h]: Usage help\n");
            printf("[-o logfile]: preferred logfile - Default: logfile\n");
            printf("[-p m]: m = number of producers - Default: 2\n");
            printf("[-c n]: n = number of consumers - Default: 6\n");
            printf("[-t time]: time = process terminate time (seconds) - Default: 100\n");
            return -1;
         case 'o':
            logfile = optarg;
            break;
         case 'p':
            producers = atoi(optarg);
            break;
         case 'c':
            consumers = atoi(optarg);
            break;
         case 't':
            time = atoi(optarg); 
            break;
         default:
            printf("INVALID INPUTS: EXITING\n");
            return -1;
      }
   }

   //printf("Number args: %i\n", argc);
   //printf("Logfile: %s\n", logfile);
   //printf("Producers: %i\n", producers);
   //printf("Consumers: %i\n", consumers);
   //printf("Time: %i\n", time);

   //exits if number of producers is greater than consumers
   if(producers >= consumers) {
      printf("Must have more consumers than producers\n");
      printf("EXITING\n");
      return -1;
   }

   //waits for signal
   if(signal(SIGINT, sigHandler) == SIG_ERR) {
      printf("CANNOT CATCH SIGNAL\n");
   }


   //shared memory section
   key_t key;
   int shmid;
   struct shmem* ptr;
   
   if((key = ftok(".", 'B')) == -1) {
      perror("ftok");
      exit(1);
   }

   if((shmid = shmget(key, 1048, 0600|IPC_CREAT|IPC_EXCL)) == -1) {
      perror("shmget");
      exit(1);
   }
  
   if((ptr = (struct shmem*) shmat(shmid, NULL, 0)) == (void*)-1) {
      perror("shmat");
      freeshm(ptr, shmid);
      exit(1);
   }

   //sets given values in shared memory
   ptr->file = logfile;
   ptr->prodNum = 0;
   ptr->consNum = 0;
   ptr->numProd = 0;
   ptr->numCons = 0;

   int procCounter = 1;

   //5 used for testing
   while(procCounter < 5) {

      pid_t ppid = fork();
      procCounter++;

      //determines if fork was successful or not
      switch(ppid) {
         case -1:
            procCounter--;
            perror("fork");
            freeshm(ptr, shmid);
            exit(1);
            break;
         case 0:
            ptr->prodNum++;
            ptr->numProd++;
            if(ptr->numProd < producers) {
               if(execlp("producer", "producer", NULL) == -1) {
                  perror("producer execlp");
                  procCounter--;
                  freeshm(ptr, shmid);
                  exit(1);
               }
            }

            ptr->prodNum--;
            ptr->numProd--;
            ptr->consNum++;
            ptr->consNum++;

            if(ptr->numCons < consumers) {
               if(execlp("consumer", "consumer", NULL) == -1) {
                  perror("Consumer execlp");
                  procCounter--;
                  freeshm(ptr, shmid);
                  exit(1);
               }
            }

            ptr->consNum--;
            ptr->numCons--;
         
            break;
         default:
            printf("PARENT WAITING\n");
            wait(NULL);
            printf("PARENT DONE WAITING\n");
            break;
      }
   }


/*
   //processes section
   int proCounter = 0;
   int max = 0;

   ptr->clockSec = 0;
   ptr->pgid = 0;

   while(max < 2 && ptr->clockSec < 3) {
      if(sigNum) {
         break;
      }

      if(procCounter < 20) {
         const pid_t = mypid = fork();

         switch(mypid) {
            case -1:
               perror("fork");
               freeshm(ptr, shmid);
               exit(1);
               break;
            case 0:
               //call the producer and consumer
               printf("good output\n");
               break;
            default:
               procCounter++;
               break;
         }

         printf("out of proc if\n");
         max++;
      }

   
   }
   
*/   

/*
   if(pthread_mutex_init(&mutex, NULL) != 0) {
      fprintf(stderr, "cannot initialize mutex\n");
   }

   if(sem_init(&full, 0, 0) != 0) {
      fprintf(stderr, "cannot initialize full\n");
   }

   pthread_t producerThreads[producers];
   pthread_t consumerThreads[consumers];

   int producerCount[producers];
   int consumerCount[consumers];

   int i;
   for(i = 1; i <= producers; i++) {
      producerCount[i-1] = i;
   }

   for(i = 1; i <= consumers; i++) {
      consumerCount[i-1] = i;
   }

   int iretP, iretC;

   //--------------------------------
   //calls producer
   //calls consumer


   //

   sleep(10);
   printf("sleep over\n");
   active = false;

   int count = 0;
   while(count < consumers) {
      iretC = pthread_join(consumerThreads[count], NULL);
      printf("Consumer thread %d returns: %d\n", count, iretC);
      count++;
   }

   count = 0;

   while(count < producers) {
      iretP = pthread_join(producerThreads[count], NULL);
      printf("Producer thread %d returns: %d\n", count, iretP);
      count++;
   }

   pthread_mutex_destroy(&mutex);
   sem_destroy(&empty);
   sem_destroy(%full);
*/

   //destroys semaphores
   printf("DESTROYING SEMAPHORES\n");
   sem_destroy(&ptr->empty);
   sem_destroy(&ptr->full);

   //frees shared memory that was used
   printf("CLEANING MEMORY\n");
   freeshm(ptr, shmid);

   printf("LEAVING MONITOR\n");
   exit(0);
   
}

void freeshm(struct shmem* ptr, int shmid) {
   if(shmdt(ptr) == -1) {
      perror("shm detach\n");
      exit(1);
   }
   if(shmid != -1) {
      if(shmctl(shmid, IPC_RMID, NULL) == -1) {
         perror("shm destroy\n");
         exit(1);
      }
   }
}

void sigHandler(int sig) {
   if(sig == SIGINT) {
      printf("CANCELLING\n");
      sigNum = 1;
   }
   //freeshm(ptr, shmid);
}
