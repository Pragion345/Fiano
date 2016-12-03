//#include"manual.h"
#include<chrono>
#include<thread>

using namespace std;
using namespace chrono;

void delayNanoseconds(unsigned long howlong)
{
	steady_clock::time_point present = steady_clock::now();
	steady_clock::time_point end = present + nanoseconds(howlong);
	duration<long, nano> elapsed = end - present;

	while(elapsed.count() > 0)
	{
		present = steady_clock::now();
		elapsed = end - present;
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

}
#endif
