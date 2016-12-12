#include"manual.h"
#include<stdio.h>
#include<thread>
#include<wiringPi.h>
#include<chrono>

using namespace std;
using namespace chrono;

void delayNanoseconds(unsigned long howlong)
{
	steady_clock::time_point begin = steady_clock::now();
	steady_clock::time_point present;

	while(1)
	{
		present = steady_clock::now();
		if(duration_cast<duration<int, nano>>(present - begin).count() >= howlong)
			break;
	}
	return;
}
#ifdef DEBUG
int main(void)
{
    int i, j;
    int a[10][20];
    unsigned long long l;
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<int, std::nano> elapsed = end-start;

    for (i = 0; i < 10; i ++)
	{
        l = (i+1) * 100;
        for (j = 0; j < 20; j++)
        {
		    start = std::chrono::high_resolution_clock::now();
		    delayNanoseconds(l);
	    	end = std::chrono::high_resolution_clock::now();
	    	elapsed = end - start;
            a[i][j] = elapsed.count();
        }
	}

    for (i = 0; i <10; i++)
    {
        int sum = 0;
        for (j = 0; j < 20; j++)
            sum += a[i][j];
        sum/=20;
        printf("%d\n", sum);
    }
		    start = std::chrono::high_resolution_clock::now();
	    	end = std::chrono::high_resolution_clock::now();
	    	elapsed = end - start;
			printf("\ntimeget delay : %d\n", elapsed.count());
		    start = std::chrono::high_resolution_clock::now();
		    delayNanoseconds(1000);
	    	end = std::chrono::high_resolution_clock::now();
	    	elapsed = end - start;
			printf("\n%d\n", elapsed.count());
		    start = std::chrono::high_resolution_clock::now();
		    delayMicroseconds(1);
	    	end = std::chrono::high_resolution_clock::now();
	    	elapsed = end - start;
			printf("\n%d\n", elapsed.count());

	struct timespec ends, starts;

	long b[10][20];
    for (i = 0; i < 10; i ++)
	{
        l = (i+1) * 100;
        for (j = 0; j < 20; j++)
        {
			clock_gettime(CLOCK_MONOTONIC, &starts);
		    delayNanoseconds(l);
			clock_gettime(CLOCK_MONOTONIC, &ends);
            b[i][j] = ends.tv_nsec - starts.tv_nsec;
        }
	}

    for (i = 0; i <10; i++)
    {
        long sum = 0;
        for (j = 0; j < 20; j++)
            sum += b[i][j];
        sum/=20;
        printf("%ld\n", sum);
    }
			clock_gettime(CLOCK_MONOTONIC, &starts);
			clock_gettime(CLOCK_MONOTONIC, &ends);
			printf("\ntimeget delay : %ld\n", ends.tv_nsec - starts.tv_nsec);
			clock_gettime(CLOCK_MONOTONIC, &starts);
		    delayNanoseconds(1000);
			clock_gettime(CLOCK_MONOTONIC, &ends);
			printf("\n%ld\n", ends.tv_nsec - starts.tv_nsec);
			clock_gettime(CLOCK_MONOTONIC, &starts);
		    delayMicroseconds(1);
			clock_gettime(CLOCK_MONOTONIC, &ends);
			printf("\n%ld\n\n", ends.tv_nsec - starts.tv_nsec);
}
#endif
