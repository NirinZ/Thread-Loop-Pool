/*
* This program check the usages and testing the Thread loop pool (tlp)
*/


//#include "head.h"
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

using namespace std;
using namespace literals::chrono_literals;
using MapIter = unordered_map<thread::id, bool>::iterator;


static bool done;
mutex muThreadRunNum;
condition_variable Tfunc;
mutex muDone;
mutex muMap;
mutex muNull;
unordered_map<thread::id, bool> mp;
CpuUsage usage;

const unsigned int MAX = 119;
unsigned long long Range = 100;
const int max_num = 120;
const unsigned long long num = 19990099; // Should be 9999909002
unsigned long thread_run_num = 0;
unsigned int delay = 0;
bool ddone = false;


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

void dammi(unsigned long long& intSSL)
{
	while (!ddone)
	{
		string str = to_string((int)pow(pow(intSSL, 234), Range)).substr(0, 3); // Usless but complicated operation
		if (intSSL == num)
		{
			intSSL++;
		}
		intSSL++;
	}
}

unsigned long long ssl()
{
	unsigned long long intSSL = 0;
	thread wrk(dammi, ref(intSSL));
	//async(launch::async, dammi, intSSL);
	this_thread::sleep_for(7s);
	ddone = true;
	wrk.join();
	cout << "ssl - " << intSSL << "\n";
	return intSSL;
}

void assign(unsigned long long a)
{
	if (muDone.try_lock())
	{
		try
		{
			done = true;
			cout << a << " - Assingned!\n";
			muDone.unlock();
		}
		catch (...)
		{
			muDone.unlock();
		}
	}
}

void* operator new(size_t s)
{

	//cout << "Allocating " << s << "\n";

	return malloc(s);
}

void func(unsigned long long range)
{
	bool another_run;
	while (!done)
	{
		if (mp.find(this_thread::get_id()) == mp.end())
		{
			mp[this_thread::get_id()] = true; // Inisialize the thread in the Map (Happend only the first time the thread reaches here)
		}
		for (unsigned long long i = range; i < range + Range && !done; i++)
		{
			string str = to_string((int)pow(pow(i, range), Range)).substr(0, 3);
			if (i == num)
			{
				assign(i);
			}
		}
		muMap.lock();
		another_run = mp[this_thread::get_id()] == true;
		muMap.unlock();
		if (another_run)
		{
			muThreadRunNum.lock();
			thread_run_num++;
			range = (unsigned long long) thread_run_num * (unsigned long long)Range;
			muThreadRunNum.unlock();
		}
		else
		{
			unique_lock<mutex> ulNull(muNull);
			Tfunc.wait(ulNull, []() {return (mp[this_thread::get_id()] == true);}); // Wait for the bool value to be true
		}
		if (delay != 0)
		{
			this_thread::sleep_for(chrono::nanoseconds(delay));
		}
	}
}

void update_map(unordered_map<thread::id, bool>& uu_map, int range)
{ // range = now avalible threads
	unsigned short prviosOnThreads = 0;

	if (range > uu_map.size())
	{
		range = uu_map.size();
	}

	for (auto& kav : uu_map)
	{
		if (kav.second == true)
		{
			prviosOnThreads++;
		}
	}

	muMap.lock();
	for (auto& kav : uu_map)
	{
		kav.second = false;
	}
	int i = 0;
	for (MapIter itr = mp.begin(); i < range; itr++)
	{
		itr->second = true;
		i++;
	}
	muMap.unlock();

	for (unsigned short i = 0; i < (range - prviosOnThreads); i++)
	{
		Tfunc.notify_one(); // Make some already created threads that was asleep, wake.
	}


}

short check_useg()
{
	short program_usaged;
	short all_usaged;
	short* cpuUsage;
	cpuUsage = usage.GetUsage();
	program_usaged = cpuUsage[0];
	all_usaged = cpuUsage[1];

	//30/100
	float should_be = (all_usaged * 30) / 100;
	float now = program_usaged * 100.0 / all_usaged;
	float gap = should_be - program_usaged;

	cout << "PROG -> " << program_usaged << "%\n";
	cout << "ALL -> " << all_usaged << "%\n";
	cout << "NOW -> " << now << "% (of 30%)\n";
	cout << "SHOULD -> " << should_be << "%\n";
	cout << "GAP -> " << gap << "%\n";

	if (gap > 30)
	{
		return 3;
	}
	if (gap > 20)
	{
		return 2;
	}
	if (gap > 10)
	{
		return 1;
	}
	if (gap < -26)
	{
		return -3;
	}
	if (gap < -17)
	{
		return -2;
	}
	if (gap < -8)
	{
		return -1;
	}
	return 0;

}

void check_avalible_threads(unsigned short& avalible_threads)
{
	short change = check_useg();
	cout << "Chabge -> " << change << "\n";

	if (change == 0)
		return;

	if (change > 0 && delay != 0)
	{
		delay = 0;
		return;
	}
	else if (avalible_threads + change > 0)
	{
		avalible_threads += change;
		delay = 0;
		return;
	}
	else
	{
		avalible_threads = 1;
		delay += (-change);
	}
}


int main()
{
	//RossiBigInt q(1000);

	//RossiBigInt rb = q * q;

	//cout << rb;

	//return 0;

	Range = ssl(); // Since every thread should last 7 sec, we check the PC speed

	cin.get();

	done = false;

	unsigned short max_threads = (short)thread::hardware_concurrency();
	max_threads = 5;
	vector<future<void>> fl(max_threads);


	unsigned short new_threads = 1; //start with this num of threads
	unsigned short total_num_of_threads = new_threads;
	unsigned short avalible_threads = new_threads;
	unsigned long PREV_thread_run_num = 0;
	//unsigned long long PREV_gap = 0;
	//long long gog = 0;
	//long long gap = 0;

	{
		Timer timer;
		while (!done)
		{
			if (new_threads > 0)
			{
				fl.push_back(async(launch::async, func, thread_run_num * Range));
				new_threads--;
				muThreadRunNum.lock();
				thread_run_num++;
				muThreadRunNum.unlock();
			}
			else
			{
				check_avalible_threads(avalible_threads);
				if (avalible_threads > total_num_of_threads)
				{
					new_threads = avalible_threads - total_num_of_threads; // Can be a negative value
					total_num_of_threads = avalible_threads;
				}
				update_map(mp, avalible_threads);
			}
			this_thread::sleep_for(5s);
			//cout << "\nThread run num: " << thread_run_num << "\n";
			cout << "\nRate: " << ((long long)thread_run_num - PREV_thread_run_num) * Range / 5000000.0f << " MN/s \n";
			cout << "Avalible threads: " << avalible_threads << " \n";
			cout << "Delay = " << delay << "\n";
			cout << "Num: " << ((long long)thread_run_num) * Range << " \n";
			long long gap = num - ((long long)thread_run_num) * Range;
			cout << "num: " << num << " \n";
			cout << "Gap: " << gap << " \n";
			//gog = PREV_gap - (num - ((long long)thread_run_num) * 100);
			//cout << "Gap of gap: " << gog << " \n";
			//PREV_gap = (num - (unsigned long long)(thread_run_num) * (unsigned long long)100);
			PREV_thread_run_num = (long long)thread_run_num;
		}
	}

	Tfunc.notify_all();
	std::cin.get();

	return 0;


}