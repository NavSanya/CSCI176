#include <unistd.h> //for fork, pipe, wait
#include <cstdlib> //for exit(0)
#include <cstring>
#include <iostream>
#include <sys/wait.h>//for wait
#include<sys/time.h>//macro get time
using namespace std;
#define MAX 100

//macro (in-line expansion) for GET_TIME(double); needs <sys/time.h>
#define GET_TIME(now)\
{ struct timeval t; gettimeofday(&t, NULL);\
  now = t.tv_sec + t.tv_usec/1000000.0; }


//iterative fibonacci function
int fibo_iter(int N)
{
	int f1=1, f2=1;
	int temp;
	if (N==1 || N==2)
	return 1;
	else
	{
		for (int i=3; i<=N; ++i)
		{
			temp = f1 + f2;
			f1 = f2;
			f2 = temp;
		}
		return temp;
	}
}

//recursive fibonacci function
int fibo_rec(int N)
{ 
	if (N==1 || N==2)
		return 1;
	else
		return (fibo_rec(N-1) + fibo_rec(N-2));
}


int main(int argc, char** argv)
{ 
	//Parent process
	int pid, status, i;
	double start1, start2, stop1, stop2;
	double time1, time2;
	int p1[2], p2[2],p3[2], p4[2]; //pipe descriptors 1,2 writing 3,4 reading

	int N, N2, N3;//number to be inputted
	N = atoi(argv[1]);
	cout<<"N="<<N<<endl;
	pipe(p1); pipe(p2);//writing
	pipe(p3); pipe(p4);//reading
	
	
	for (i=1;i<4;i++) //divide 3 children
	{ pid = fork();
		if (pid==0 && i==1 ) //child process1 i.e. control process
		{ 
			//Recursion
			
			write(p1[1], &N, sizeof (int));//child 2
			write(p2[1], &N, sizeof (int));//child 3
			read(p3[0],&time1,MAX);
			read(p4[0],&time2,MAX);
			cout<<"Execution Time for Recursion is "<<time1<<"sec"<<endl;
			cout<<"Execution time for iteration is " <<time2<<"sec"<<endl;
		
			
			exit(0);
		}//if
 
		else if (pid==0 && i==2 ) //child process2 i.e. recursive
		{ 
			read(p1[0], &N2, sizeof (int));
		
			GET_TIME(start1);
			cout<<"Answer from Recurtion is "<<fibo_rec(N2)<<endl;
			GET_TIME(stop1);
			time1 = (stop1-start1);
			write(p3[1],&time1,MAX);
			
			exit(0);
		}//elseif

		else if (pid==0 && i==3 ) //child process3 i.e. iterative
		{ 
			read(p2[0], &N3, sizeof (int));
			
			GET_TIME(start2);
			cout<< "Answer from iteration is " <<fibo_iter(N3)<<endl;
			GET_TIME(stop2);
			
			time2 = (stop2-start2);
			write(p4[1],&time2,MAX);
			exit(0);
		}//elseif
	}//for_i
	
	//Now wait for the child processes to finish
	for (i=1; i<=3; i++)
	{ 
		pid = wait(&status);
		cout<<"Child (pid="<<pid<<") exited, status="<<status<<endl;
	}
	return 0;
}//main 
