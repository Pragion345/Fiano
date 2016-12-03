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
	unsigned long l;
	auto start = std::chrono::high_resolution_clock::now();
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::nano> elapsed = end-start;
	while(1)
	{
		printf("Delay 입력 : ");
		scanf("%lu", &l);
		start = std::chrono::high_resolution_clock::now();
		delayNanoseconds(l);
		end = std::chrono::high_resolution_clock::now();
		elapsed = end - start;
		printf("1차: %lfns\n",elapsed.count());
		start = std::chrono::high_resolution_clock::now();
		delayNanoseconds(l);
		end = std::chrono::high_resolution_clock::now();
		elapsed = end - start;
		printf("2차: %lfns\n",elapsed.count());
		start = std::chrono::high_resolution_clock::now();
		delayNanoseconds(l);
		end = std::chrono::high_resolution_clock::now();
		elapsed = end - start;
		printf("3차: %lfns\n",elapsed.count());
	}
}
#endif
