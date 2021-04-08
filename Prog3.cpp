/*Write a Pthreads parallel program for matrix multiplication.
The matrix multiplication problem is defined as:
 A(L*m) * B(m*n) = C(L*n)
 m-1
 where, A, B and C are L*m, m*n and L*n matrices, respectively, and Ci,j = å (Ai,k * Bj,k)
 k=0
Your program should access the command line arguments for the matrix dimensions, i.e., L, m 
and n, together with the number of threads used for the computation.
The matrices should be declared as global (shared among all threads), and your main process
initializes the values with the following formula: Ai,j = i+j+1 and Bi,j = i+j
For this, you need to declare the matrices as dynamic 2-dimensional arrays.
After initializing A and B matrices, the main process should create the requested number of 
threads and let them work for building C matrix, i.e., entries of the C matrix are computed and 
updated by those slave threads.
To distribute the task of completing the C matrix to those multiple threads fairly, please assign 
rows (total number of rows = L) of the C matrix to the threads in a cyclic way (jump-p way), i.e.,
for example with P=4, Th_0 is assigned with row_0, Th_1 is assigned with row_1, Th_2 is 
assigned with row_2, Th_3 is assigned with row3, then, Th_0 is assigned with row_4, Th_1 is 
assigned with row_5, and so on.
Check the correctness of your program with small size matrices first; and then use the following 
dimensions of the matrices for the final output to submit: 
 L=1003, m=2000, n=3000
Since the size of the output is extremely big (~36MB), please display only [first_10 * first_10]
and [last_10 * last_10] entries of matrix C; this should be done in the main process after joining 
the threads.
Run your program with number of threads = 1, 2 , 4, 8 and check/show the execution time for 
each run. For this, you need to use time checking codes in your program.
* Finally, please try the BT way of optimization, i.e., A(L*m) * BT(n*m) = C(L*n), and check 
 how it affects the execution time.*/

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
