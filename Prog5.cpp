#include<iostream>
#include<mpi.h>
#include<algorithm>
#include<stdlib.h>
using namespace std;

int* makeArr(int num, int n, int rank)
{
    int i;
    int size = n/num;
    int *Arr = new int[size];

    srand(rank+1);
    for(i = 0; i < size; ++i)
    {
        Arr[i] = rand()%100;
    }
    return Arr;
}

void Merge(int *A, int *B, int size)
{
    int *x = new int [size];
    merge(A, (A+size), B, (B+size), A);
}

int Compare(const void *a, const void *b)
{
    return (*(int*)a - *(int*)b);
}


int main(int argc, char *argv[])
{
    int n = atoi(argv[1]);
    
    int* local_Array = new int[n];


    int Comm_size, Comm_rank;

    
    int part;
    int coreDiff = 1;
    int div = 2;
    int *temp;
    int ArrSizeFTR = n;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &Comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &Comm_rank);

    if(Comm_rank==0)
    {

        //MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

        for(int p = 1; p<Comm_size; ++p)
        {
	    MPI_Send(&n,1,MPI_INT,p,0,MPI_COMM_WORLD);
            local_Array = makeArr(Comm_size, n, Comm_rank);
            qsort(local_Array, n/Comm_size, sizeof(int), Compare);
        }
    }

    else
    {
        MPI_Recv(&n, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        local_Array = makeArr(Comm_size,n, Comm_rank);
        qsort(local_Array, n/Comm_size, sizeof(int), Compare);
    }  

    cout<<"process_"<<Comm_rank<<":";
    for(int i = 0; i<n/Comm_size; ++i)
    {
        cout<<local_Array[i]<<"\t";
    }
    cout<<"\n\n";

    bool done = false;
    while(Comm_size>=div && !done)
    {
        if(Comm_rank % div == 0)
        {
            temp = new int [n];
            part = Comm_rank + coreDiff;
            MPI_Recv(temp, n, MPI_INT, part, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            Merge(local_Array,temp, n);
            ArrSizeFTR*=2;
            delete() temp;
        }
        else
        {
            part = Comm_rank - coreDiff;
            MPI_Send(local_Array, n, MPI_INT, part, 0, MPI_COMM_WORLD);
            done = true;
        }
        div*=2;
        coreDiff*=2;
    }

    if(Comm_rank==0)
    {
        cout<<"Sorted List: "<<endl;
        for(int i = 0; i< n;++i)
            cout<<local_Array[i]<<"\t";
        cout<<endl;
    }
    
    delete() local_Array;
    MPI_Finalize();
    
    return 0;

}
