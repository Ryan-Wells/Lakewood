#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define QUEUE_LIMIT 5

//global variables
int availableLifeVests = 10;
int maxLifeVests = 10;
int minLifeVests = 0;
int groupsWaiting = 0;
int groupNumber = 0;
int arriveTime = 10;
int frontOfLine = 0;
pthread_mutex_t mutex1;
pthread_cond_t condition;
struct queue myQueue;


struct node{
   int groupNo;
   int neededLifeVests;
   int done;
   struct node* next;
};


struct queue{
   struct node* head;
   struct node* tail;
};


void queue_init(struct queue* queue){
   queue->head = NULL;
   queue->tail = NULL;
}


bool queue_isEmpty(struct queue* queue){
   return queue->head == NULL;
}


//used to print the queue
void print_queue(struct queue* queue){
   struct node* tmp;
   tmp = queue->head;
   
   printf("    Queue: [%d", tmp->groupNo);
   tmp = tmp->next;
   while(tmp != NULL){
      printf(", %d", tmp->groupNo);
      tmp = tmp->next;
   }
   printf("]\n");

}


//used to add a renter group to the line
void queue_insert(struct queue* queue, int groupNo, int neededLifeVests){
   struct node* tmp = malloc(sizeof(struct node));
   if(tmp == NULL){
      fputs("malloc failed\n", stderr);
      exit(1);
   }

   /* create the node */
   tmp->groupNo = groupNo;
   tmp->done = 0;
   tmp->neededLifeVests = neededLifeVests;
   tmp->next = NULL;

   if(queue->head == NULL){
      queue->head = tmp;
   } 
   else{
      queue->tail->next = tmp;
   }
   queue->tail = tmp;
   print_queue(queue);
   groupsWaiting++;
}


//used to remove the first renter group in the line
int queue_remove(struct queue* queue){
   int retval = 0;
   struct node* tmp;
   if(!queue_isEmpty(queue)){
      tmp = queue->head;
      retval = tmp->groupNo;
      queue->head = tmp->next;
      free(tmp);
      groupsWaiting--;
      
   }
   
   if(!queue_isEmpty(&myQueue)){
      frontOfLine = myQueue.head->groupNo;
   }
   return retval;
}


//used if we cannot create a thread
void fatal(int n){
  printf ("Fatal error, lock or unlock error, thread %d.\n", n);
  exit(n);
}


//renter thread (represents a group of renters)
void* renter_body(void* arg){

   if(pthread_mutex_lock(&mutex1)){ fatal(groupNumber); }
   int groupNo = groupNumber++;
   int craftType = random() % 3;
   int neededLifeVests;
   char* craft;
   int useTime = random() % 7;

   if(craftType == 0){
      neededLifeVests = 1; //kayak
      craft = "kayak";
   }
   else if(craftType == 1){
      neededLifeVests = 2; //canoe
      craft = "canoe";
   }
   else if(craftType == 2){
      neededLifeVests = 4; //sailboat
      craft = "sailboat";
   }
   
   printf("Group %d requesting a %s with %d lifevests.\n", groupNo, craft, neededLifeVests);


   //if there are enough life vests available and nobody is waiting, rent them out
   if(neededLifeVests <= availableLifeVests && groupsWaiting == 0){

      availableLifeVests -= neededLifeVests;
      printf("Group %d issued %d lifevests, %d remaining\n", groupNo, neededLifeVests, availableLifeVests);

      if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }

      //go out on the lake
      sleep(useTime);

      //report the return of life vests, add those back to life jacket count
      if(pthread_mutex_lock(&mutex1)){ fatal(groupNo); }

      availableLifeVests += neededLifeVests;
      printf("Group %d returning %d lifevests, now have %d\n", groupNo, neededLifeVests, availableLifeVests);

      //now other groups are able to rent life vests, so signal
      pthread_cond_broadcast(&condition);

      if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }
       pthread_exit((void*) arg);
   }



   //if there are enough life vests available but people are waiting, get in line
   else if(groupsWaiting > 0){

      //five groups waiting is too long of a line.
      if(groupsWaiting >= QUEUE_LIMIT){

         printf("Group %d leaves due to too long a line\n", groupNo);

         if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }

         pthread_exit((void*) arg);
      }

      printf("   Group %d waiting in line for %d lifevests\n", groupNo, neededLifeVests);

      queue_insert(&myQueue, groupNo, neededLifeVests);

      frontOfLine = myQueue.head->groupNo;

      //wait until their turn
      while(availableLifeVests < neededLifeVests || frontOfLine != groupNo){
         pthread_cond_wait(&condition, &mutex1);
      }

      printf("   Waiting group %d may now proceed.\n", groupNo);

      queue_remove(&myQueue);
      if(!queue_isEmpty(&myQueue)){
         frontOfLine = myQueue.head->groupNo;
      }

      availableLifeVests -= neededLifeVests;

      printf("Group %d issued %d lifevests, %d remaining\n", groupNo, neededLifeVests, availableLifeVests);

      if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }

      //go out on the lake
      sleep(useTime);

      //report the return of life vests, add those back to life jacket count
      if(pthread_mutex_lock(&mutex1)){ fatal(groupNo); }

      availableLifeVests += neededLifeVests;
      printf("Group %d returning %d lifevests, now have %d\n", groupNo, neededLifeVests, availableLifeVests);

      //now other groups are able to rent life vests, so signal
      pthread_cond_broadcast(&condition);

      if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }

      pthread_exit((void*) arg);
      
   }



   //if there aren't enough life vests, try to get in line
   else if(availableLifeVests < neededLifeVests){

      //if the line isn't too long, add this renting group to the queue
      if(groupsWaiting < QUEUE_LIMIT){

         printf("   Group %d waiting in line for %d lifevests\n", groupNo, neededLifeVests);

         queue_insert(&myQueue, groupNo, neededLifeVests);

         frontOfLine = myQueue.head->groupNo;

         //wait until there are enough life jackets
         while(availableLifeVests < neededLifeVests  || frontOfLine != groupNo){
            pthread_cond_wait(&condition, &mutex1);
         }

         //then remove from the waiting queue and rent:
         printf("   Waiting group %d may now proceed.\n", groupNo);

         queue_remove(&myQueue);
         if(!queue_isEmpty(&myQueue)){
            frontOfLine = myQueue.head->groupNo;
         }
         
         availableLifeVests -= neededLifeVests;

         printf("Group %d issued %d lifevests, %d remaining\n", groupNo, neededLifeVests, availableLifeVests);

         if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }


         //go out on the lake
         sleep(useTime);

         if(pthread_mutex_lock(&mutex1)){ fatal(groupNo); }

         availableLifeVests += neededLifeVests;

         printf("Group %d returning %d lifevests, now have %d\n", groupNo, neededLifeVests, availableLifeVests);

         //now other groups are able to rent life vests, so signal
         pthread_cond_broadcast(&condition);

         if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }

         pthread_exit((void*) arg);
      }

      //if too many groups are waiting, discard this renting group (report, exit thread).
      else{
         printf("Group %d leaves due to too long a line\n", groupNo);

         if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }
         pthread_exit((void*) arg);
      }
      pthread_exit((void*) arg);
   }


   else{
      printf("\nSOMETHING WENT HORRIBLY WRONG\n\n");
   }


   if(pthread_mutex_unlock(&mutex1)){ fatal(groupNo); }

   pthread_exit((void*) arg);
}



//the main thread. Creates and joins threads (renting groups)
int main(int argc, char** argv){

   if(argc == 1){
      printf("please enter the number of visitors.\n");
      return 1;
   }

   queue_init(&myQueue);

   int numGroups = atoi(argv[1]);
   int err;
   void* retval;
   int randGen = 0;
   pthread_mutex_init(&mutex1, NULL);
   pthread_t groups[numGroups];
   
   //process command line arguments
   if(argv[2] != NULL){
      arriveTime = atoi(argv[2]) / 2;
      if(argv[3] != NULL){
         srandom(time(NULL));
         randGen = 1;
      }
   }
   if(randGen == 0){
      srandom(0);
   }

   //create the threads
   for(long i = 0; i < numGroups; i++){
      err = pthread_create(&groups[i], NULL, renter_body, (void*) i);

      if(err){
         fprintf(stderr, "Can't create thread %ld\n", i);
         exit(1);
      }

      sleep(random() % arriveTime);

      //the loop that creates new threads should join threads that have finished

      //every single source (manual pages, online strangers, classmates, a previous professor) has unanimously told me not to do this, and instead join the threads in a separate loop. It also seems like extra unnecessary and unprotected work to get an incorrect output and a worse, more error-prone code design.
      
   }


   //join the threads
   for(long j = 0; j < numGroups; j++){
      pthread_join(groups[j], &retval);
   }

   pthread_mutex_destroy(&mutex1);  // Not needed, but here for completeness

   return 0;
}
