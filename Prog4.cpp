/*Parallel merge sort with OpenMP.
Consider the merge sort algorithm that you studied in data structures/algorithm course(s). 
For a given problem size n, each thread is assigned a local list of n/p elements.
In this version of parallel merge sort, each thread performs ‘qsort’ (GNU library quicksort) to 
sort the local list instead of implementing the recursion-based method.
Merging step is a tree reduction, e.g., thread_0 merges locally sorted lists from thread_0 and 
thread_1, thread_2 merges locally sorted lists from thread_2 and thread_3, and so on. 
Eventually, thread_0 produces the final globally sorted list.
A more detailed guide is shown below.
Programming guide -----------------------------------------------------------------------------------------
1. Access command line arguments for n (total number of elements in the list) and p (total number of
 threads); create a list (dynamic array) of n integers; initialize the list with random numbers – each 
 random number should be less than or equal to n, and it is suggested to use ‘parallel for’ loop. 
2. Time checking starts at this point; please use ‘omp_get_wtime()’ that OpenMP supports.
3. Launch multiple threads; in each thread, create a local list of size n/p, copy list[my_start ~ my_end] to 
 the local list, and perform qsort (GNU library quicksort); each thread is responsible for updating the 
 global list with the sorted local list. It is suggested to use ‘parallel for’ loop for local list copy steps.
4. Merging steps need a while loop, which you practiced in HW1; pseudo-code is shown below:
 divisor = 2; core-difference = 1;
 while (divisor <= thread_count)
 { //use barrier synchronization here;
 //determine whether this thread is sender or receiver;
 //if receiver, perform merging operation by calling merge(..) function;
 //there is nothing to do for a sender;
 divisor *= 2; core_difference *= 2;
 }
 Merge(..) function should be defined separately; one suggested idea is that the function updates the 
 global list with the merged result.
5. Time checking ends at this point; please use ‘omp_get_wtime()’ that OpenMP supports.
6. Finally, please make a function to check whether the final list is completely sorted or not.
 This function is easy to define, i.e., using a for loop (i=0 ~ n-1), check whether (list[i] > list[i+1]).
----------------------------------------------------------------------------------------------------------------------------*/

#include <iostream>
#include <time.h> // for time
#include <omp.h>  // openmp library
#include <cstdlib> //for ascii to integer
#include <stdlib.h> // for srand, rand, for qsort
using namespace std;

// function prototypes

void sort_Local_list(int n, int p, int* globalList, int remainingNumbers);
void merge_list(int receiver, int sender, int listSize, int coreDiff, int divisor, int* globalList, int n, int p);
void check_final_sort(int* globalList, int n);
int comparison (const void * a, const void * b);


// ------- BEGIN LOCAL SORT FUNCTION -----------
void sort_Local_list(int n, int p, int* globalList, int remainingNumbers) {
  int threadRank = omp_get_thread_num(); // obtain thread rank using omp
  int listSize, start, end;
  bool remainFlag = false;

  // remaining numbers added to list
  if(remainingNumbers != 0 && threadRank == p - 1) {
    listSize = n/p + remainingNumbers;
    start = (listSize-remainingNumbers)*threadRank;  // start from global
  }
  else {  // otherwise listSize set to n/p & start set to listSize*threadRank
    listSize = n/p;
    start = listSize*threadRank;  // start from global
  }

  int* localList = new int[listSize];
  end = start + listSize; //end from global
  int i = 0;
  int tempStart = start;
  // move from local list to global list
  for(; tempStart < end; tempStart++, i++) {
    localList[i] = globalList[tempStart];
  }
  // qsort local List
  qsort(localList, listSize, sizeof(int), comparison);

  // update global list from local
  i = 0;  // reset i to 0 to be reused
  #pragma parallel for
    for (; start < end; start++, i++) {
    globalList[start] = localList[i];
  }

  int divisor = 2;
  int coreDifference = 1;

  while(divisor <= p) {
    #pragma omp barrier
    if(threadRank % divisor == 0) {  // check to see which thread is reciever
      // call merge_list sort
      merge_list(threadRank, threadRank + coreDifference, listSize, coreDifference, divisor, globalList, n, p);
    }
    else {
      // nothing for sender
    }
    divisor *= 2;
    coreDifference *= 2;
  }
}
// -------- END LOCAL SORT FUNCTION ----

 // ------ BEGIN MERGE_LIST FUNCTION ----
void merge_list(int receiver, int sender, int listSize, int coreDiff, int divisor, int* globalList, int n, int p) {
  int recieverStart = receiver * listSize;
  int recieverEnd = recieverStart + (listSize * coreDiff);
  int senderStart = sender * listSize;
  int senderEnd = senderStart + (listSize * coreDiff);
  int size = listSize*divisor;

  // check remining values from sender
  if(sender == (p - coreDiff) && (n%p) != 0) {
    senderEnd += n%p; // add to sender's end size
    size += n%p;  // add to size
  }

  int* localList = new int[size];
  int i = 0;
  int tempRecStart = recieverStart;
  int tempSendStart = senderStart;

  while(tempRecStart != recieverEnd && tempSendStart != senderEnd) {
    if(globalList[tempRecStart] <= globalList[tempSendStart]) {
      localList[i] = globalList[tempRecStart];
      i++;
      tempRecStart++;
    }
    else if(globalList[tempRecStart] >= globalList[tempSendStart]) {
      localList[i] = globalList[tempSendStart];
      i++;
      tempSendStart++;
    }
  }

  if(tempRecStart == recieverEnd && tempSendStart != senderEnd) {
    for(tempSendStart; tempSendStart < senderEnd; tempSendStart++, i++) {
      localList[i] = globalList[tempSendStart];
    }
  }
  else if(tempSendStart == senderEnd && tempRecStart != recieverEnd) {
    for(; tempRecStart < recieverEnd; tempRecStart++, i++) {
      localList[i] = globalList[tempRecStart];
    }
  }

  // update global List
  i = 0;
  #pragma parallel for
    for(; recieverStart < senderEnd; recieverStart++, i++) {
      globalList[recieverStart] = localList[i];
  }
}
// ----------- END MERGE_LIST FUCNTION -------

// --------- BEGIN CHECK SORT FUNCTION -----
void check_final_sort(int* globalList, int n) {
  for(int i = 0; i < n - 1; i++) {
    if(globalList[i] > globalList[i+1]) {
      cout << "NOT SORTED" << endl;
      return;
    }
  }
  cout << "SORTED" << endl;
}
// ------- END CHECK SORT FUNCTION -----

// -------- begin comparison function --------
int comparison (const void * a, const void * b) {
  return ( *(int*)a - *(int*)b );
}
// -------- end comparison function -----


// ------- BEGIN MAIN FUNCTION ----------
int main(int argc,char* argv[]) {
  int n = atoi(argv[1]);  // get n length
  int p = atoi(argv[2]);  // get total number of threads
  int remainingNumbers = 0;

  // check if n & p is evenly disvable
  if((n%p) != 0)
    remainingNumbers = n%p;

  double startTime, endTime;
  int* listNum = new int[n];

  srand(time(NULL));  // random seed
  #pragma parallel for
    for(int i = 0; i < n; i++) {
        listNum[i] = rand() % n + 1; // range from 1 <= n
    }

  // start time
  startTime = omp_get_wtime();
  #pragma omp parallel num_threads(p)
    sort_Local_list(n,p,listNum,remainingNumbers);

  // for display purpose
  for(int j = 0; j < n; j++) {
    cout << listNum[j] << " ";
  }
  endTime = omp_get_wtime();
  cout << "TIME ELAPSED: " << endTime - startTime << endl;
  check_final_sort(listNum, n);
  return 0;
}
