#include <cstdlib>
#include <sys/time.h>
#include <pthread.h>

using namespace std;
//macro from time-fork2.cpp
#define GET_TIME(now)\
{	struct timeval t; gettimeofday(&t, NULL);\
	now = t.tv_sec + t.tv_usec/1000000.0;}

//Global Area
double** A; //Lxm
double** B; //mxn
double** C; //Lxn
double** Btrans; //B transpose
int thread_count, L, m, n;
double start, stop, total;
pthread_mutex_t mutex1; //Used for updating C
pthread_mutex_t mutex2; //Used for cout statements

void *Slave(void* rank); //slave function 

int main(int argc, char* argv[])
{
	GET_TIME(start);
	long thread_id;
	L = atoi(argv[1]); //row A and C
	m = atoi(argv[2]);//row B and column A
	n = atoi(argv[3]); //column B and C
	thread_count = atoi(argv[4]); //total number of threads

	/*******declare and initialize A********/
	A = new double*[L];
	for(int i = 0; i<L; ++i)
	{
		A[i] = new double[m];
		for(int j = 0; j<m; ++j)
			A[i][j] = i+j+1;
	}

	/*******declare and initialize B*******/
	B = new double*[m];
	for(int i = 0; i<m; i++)
	{
		B[i] = new double[n];
		for(int j = 0; j<n; ++j)
			B[i][j] = i+j;
	}

	/********declare C********/
	C = new double*[L];
	for(int i = 0; i<L; i++)
		C[i] = new double[n];

	/********B transpose*********/
	Btrans = new double*[n];
	for(int i = 0; i<n; ++i)
	{
		Btrans[i] = new double[m];
		for(int j = 0; j<m; ++j)
			Btrans[i][j] = B[j][i];
	}

	cout<<"L="<<L<<"\nm="<<m<<"\nn="<<n<<endl;

	pthread_t myThreads[thread_count]; //define threads
	pthread_mutex_init(&mutex1, NULL);
	pthread_mutex_init(&mutex2, NULL);

	//Creating the threads
	for(thread_id=0; thread_id<thread_count; thread_id++)
		pthread_create(&myThreads[thread_id], NULL, Slave, (void*)thread_id);

	//wait till all threads have finished and joins back to main
	for(thread_id=0; thread_id<thread_count; thread_id++)
		pthread_join(myThreads[thread_id], NULL);

	GET_TIME(stop); //stop time
	total = stop - start;
	
	//output first 10x10 
	cout<<"----first 10x10----"<<endl;
	for(int i = 0; i<10; ++i)
	{
		for(int j = 0; j<10; ++j)
			cout<<C[i][j]<<"\t";
		cout<<endl;
	}

	//output last 10x10
	cout<<"----last 10x10----"<<endl;
	for(int i = L-10; i<L; ++i)
	{
		for(int j = n-10; j<n; ++j)
			cout<<C[i][j]<<"\t";
		cout<<endl;
	}

	cout<<"Execution Time: " <<total<<"s"<<endl;
	
	return 0;
}

//Slave Function
void *Slave(void* rank)
{
	int my_rank = (long)rank;
	long a;
	for(int i = my_rank;  i<L; i+=thread_count)
	{
		for(int j = 0; j<n; ++j)
		{
			a=0;
			for(int k = 0; k<m; ++k)
			{
				a+=A[i][k] * B[k][j];
				//a+=A[i][k] * Btrans[j][k];
			}
		pthread_mutex_lock(&mutex1);
		C[i][j]=a;
		pthread_mutex_unlock(&mutex1);
		}
	}
	pthread_mutex_lock(&mutex2);
	cout<<"Thread_"<<my_rank<<":"<<my_rank<<"~"<<L-1<<", step"<<thread_count<<endl;
	pthread_mutex_unlock(&mutex2);

	return NULL;
}
