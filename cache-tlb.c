//Shaleen Garg
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define MB (1024*1024) //Size of MB in bytes
#define NSTEPS 16*MB //Number of Steps to take
#define INT_MIN  (-2147483640) //Minimum Integer possible
#define CACHE_LINE_SIZE 64 //Assume this is the size of cache in bytes
#define MAX_STRIDE 256 //This is the maximum we want to jump in access
#define MAX 10000000 //MAX for linked list

typedef struct mem{
    long long int A;
    struct mem *next;
    //char padding[CACHE_LINE_SIZE - 2*sizeof(struct mem*) - sizeof(int)];
    char padding[48]; //Make it a CachelineSize node
}Mem;

//Time in microseconds
double GetMicroTime(struct timespec *start, struct timespec *end)
{
    return ((end->tv_sec - start->tv_sec)*1000000000 + \
                end->tv_nsec - start->tv_nsec)/1000; //Time in microseconds
}

//This function returns the length of the cache line
int CacheLineSizeTest()
{
    char *array = (char *)malloc(CACHE_LINE_SIZE * NSTEPS);
    assert(array != NULL); //Error in malloc
    int i, Stride;

    for(i=0; i<CACHE_LINE_SIZE*NSTEPS; i+=4*1024)
        array[i] = 0; //Touch each page to remove minor faults

    int max = INT_MIN, CacheWord;

    bool First = false; //Checks if the loop is being run firsttime

    struct timespec start, end;
    double TimeTaken, PreviousTime;
    register int h; //never increase time due to h

    for (Stride=1; Stride <= MAX_STRIDE; Stride<<=1)
    {
        //printf("Stride = %d\n", Stride);
        clock_gettime(CLOCK_REALTIME, &start);
        for (i=1; i < NSTEPS; i++)
        {
            h = (i*Stride) & NSTEPS-1;
            //printf("h = %d\n", h);
            array[h] += i;
        }
        clock_gettime(CLOCK_REALTIME, &end);

        TimeTaken = GetMicroTime(&start, &end);
        //printf("Time taken = %lf microsec\n", TimeTaken);
        if(First)
        {
            if(max < (TimeTaken - PreviousTime))
            {
                max = TimeTaken - PreviousTime;
                CacheWord = Stride;
            }
        }
        else
            First = true;

        PreviousTime = TimeTaken;
    }
    free(array);
    printf("The Cacheline size = %d Bytes\n", CacheWord);
    return CacheWord;
}

//Returns the Size of different caches based on Start or Stop Size and AddSize
double CacheSizeTest(int StartSize, int StopSize, int AddSize)
{
    char *array = (char *)malloc(CACHE_LINE_SIZE * NSTEPS);
    assert(array != NULL); //Error in malloc
    int i; 
    int Stride;

    for(i=0; i<CACHE_LINE_SIZE*NSTEPS; i+=4*1024)
        array[i] = 0; //Touch each page to remove minor faults

    int max = INT_MIN, CacheSize;
    bool First = false; //Checks if the loop is being run firsttime

    struct timespec start, end;
    double TimeTaken, PreviousTime;
    register int h; //never increase time due to h

    for (Stride=StartSize; Stride <= StopSize; Stride+=AddSize)
    {
        clock_gettime(CLOCK_REALTIME, &start);
            for (i = 0; i < NSTEPS; i++)
            {
                array[(i*CACHE_LINE_SIZE) & Stride] += i;
            }
        clock_gettime(CLOCK_REALTIME, &end);
        TimeTaken = GetMicroTime(&start, &end);
        //printf("Time Taken for %d Cachesize = %lf\n", Stride, TimeTaken);
        if(First)
        {
            if(max < (TimeTaken - PreviousTime))
            {
                max = TimeTaken - PreviousTime;
                CacheSize = Stride;
                //printf("CacheSize = %d\n", CacheSize);
            }
        }
        else
            First = true;
        PreviousTime = TimeTaken;
    }
    free(array);
    return CacheSize/1024;
}


//Returns the approximate Memory Latency
double MemoryTimingTest()
{
    int i; 
    Mem *Llist, *traverse;
    Llist = (Mem *)malloc(sizeof(Mem));
    Llist->A = 0;
    traverse = Llist;
    for(i=1; i<MAX; i++)
    {
        traverse->next  = (Mem *)malloc(sizeof(Mem));
        traverse->A = i;
        traverse = traverse->next;
    }

    int j =0;
    struct timespec t1, t2;

    //gettimeofday(&t1, NULL);
    clock_gettime(CLOCK_REALTIME, &t1);

        traverse = Llist;
        for(i=0;i<MAX-1; i++)
        {
            //printf("%d\n", traverse->A);
            traverse = traverse->next;
            traverse->A += 1;
        }

    //gettimeofday(&t2, NULL);
    clock_gettime(CLOCK_REALTIME, &t2);

    //printf("%d\n", traverse->A);

    return GetMicroTime(&t1, &t2)/(MAX);
}


int main(int argc, char *argv[])
{
    //printf("Size of %d\n", sizeof(struct mem));
    CacheLineSizeTest();

    printf("L1 Cache Size %lf KB\n", CacheSizeTest(1024, 40*1024, 1024));
    printf("L2 Cache Size %lf KB\n", CacheSizeTest(38*1024, 2*MB, 10*1024));
    printf("L3 Cache Size %lf KB\n", CacheSizeTest(2*MB, 30*MB, MB));

    printf("Memory Latency = %lf nanoseconds\n", MemoryTimingTest()*1000);

    return 0;
}
