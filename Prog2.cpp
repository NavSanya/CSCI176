#include <iostream>
#include <cstdlib> //for atoi()
#include <pthread.h> 
#include <sys/wait.h>//for wait
#include<sys/time.h>//macro get time
using namespace std;

//globals --accessible to all threads
int thread_count;       //for command line arg
pthread_mutex_t mutex1; 
pthread_mutex_t mutex2;//to protect c.s. (e.g., cout statement should be atomic)
int arr[500000000];
double global_sum;
void *Slave(void* rank); //prototype for a Thread (slave) function

//macro (in-line expansion) for GET_TIME(double); needs <sys/time.h>
#define GET_TIME(now)\
{ struct timeval t; gettimeofday(&t, NULL);\
  now = t.tv_sec + t.tv_usec/1000000.0; }

/////////////////////main 
int main(int argc, char* argv[]) 
{
    double start1, stop1;
	double time1;
  long thread_id; //long is needed for type casting to void* type
 thread_count = atoi(argv[1]); //tot number of threads - from command line
 //cin>>thread_count;
  pthread_t myThreads[thread_count]; //define threads 
  pthread_mutex_init(&mutex1, NULL); //initialize mutex1
  pthread_mutex_init(&mutex2, NULL); //initialize mutex2
  for(int i=0; i<500000000;++i)
  {
      arr[i]=i+1;
  }
  
  GET_TIME(start1);
  //creates a certain number of threads
  for(thread_id = 0; thread_id < thread_count; thread_id++)  
     pthread_create(&myThreads[thread_id], NULL, Slave, (void*)thread_id);  

  //wait until all threads finish
  for (thread_id = 0; thread_id < thread_count; thread_id++) 
     pthread_join(myThreads[thread_id], NULL); 
     
    pthread_mutex_lock(&mutex1);
    GET_TIME(stop1); 
    //display the global_sum
    cout<<"The sum is "<<global_sum<<endl;
    time1 = (stop1-start1);
	//Display the execution time
    cout<<"Total Execution Time"<<time1<<endl;
    pthread_mutex_unlock(&mutex1); //release mutex
  return 0;
}//main()

//////////////////slave function
void *Slave(void* rank) 
{
	
    double partial_sum =0;
    
  int my_rank = (long)rank; //rank is void* type, so can cast to (long) type only; 
  int quotient = 500000000/thread_count;
  int remainder = 500000000%thread_count;
  int count, my_first_i, my_last_i;
  
  //calculate my first rank and my last rank
  if(my_rank<remainder)
  {
	  count = quotient+1;
	  my_first_i = my_rank * count;
  }
  else
  {
	  count = quotient;
	  my_first_i = my_rank * count - 1;
  }
  my_last_i = my_first_i + count - 1;
  
  
  for (int i = my_first_i; i<=my_last_i;++i) {
    partial_sum += arr[i];
  }
	pthread_mutex_lock(&mutex2);   //reserve muterx before cout statement
  cout<<"Thread_id"<<my_rank<<"\nstart index "<<my_first_i<<"\nend index "<<my_last_i<<"\nPartial sum "<<partial_sum<<"\n\n--------------"<<endl; //thread_count is global
  pthread_mutex_unlock(&mutex2); //release mutex
  
  pthread_mutex_lock(&mutex1);   
   global_sum += partial_sum;
    pthread_mutex_unlock(&mutex1); //release mutex
   
  return NULL;
}//Slave
