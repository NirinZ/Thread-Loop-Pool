#include <iostream>
#include <chrono>
#include <future>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <condition_variable>
#include "bigint.h"
#include "C:\Users\zniri\Desktop\Coding\Languages\C++\C++ Libs\CpuUsage\CpuUsage.h"
#include <string>
#include <thread>
#include "ThreadLoopPool.h"
#include <typeinfo>

struct Timer
{
	chrono::time_point<chrono::steady_clock> start, end;
	chrono::duration<double> duration;

	Timer()
	{
		start = chrono::high_resolution_clock::now();
	}
	~Timer()
	{
		end = chrono::high_resolution_clock::now();
		duration = end - start;

		double ms = duration.count() * 1000.0f;
		cout << "Timer took " << ms << "ms \n";
	}
};

void assigne(unsigned long long t_num);

unsigned long long num = 51473836470514738; // 51473836470

void func(unsigned long long range, int o);

void assigne(unsigned long long t_num)
{
	cout << "\n\nThis is the num!\n----------\n" << t_num <<  endl;
}

void foo(unsigned short s)
{
	cout << "func: " << s << endl;
}

ThreadLoopPool<void(*)(unsigned long long, int), unsigned long long> tlp;

int main()
{
	{
		//Timer timer1;
		//func(1l, 2); // 70s -> 70779.6ms
	}
	//cout << typeid(ref(a)).name() << endl;
	//return 0;
	{
		Timer timer2;
		
		tlp.linkTheFunc(func, 9);
		cout << "Starting!\n";
		tlp.start();
	}
	//ThreadLoopPool<decltype(&func), int, int> tlp;
}
void func(unsigned long long range, int o)
{
	for (size_t i = range; i < range + tlp.step_leangth && !tlp.done; i++)
	{
		if (i == num)
		{
			//tlp.wrapperAssigne<void(__cdecl*)(unsigned long long), unsigned long long>(assigne, i);
			tlp.wrapperAssigne(assigne, i);
			break;
		}
	}
}