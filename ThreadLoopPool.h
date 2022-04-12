#pragma once
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <condition_variable>
#include "CpuUsage.h" // lib \ cpp + include directories
// ע"מ לגרום לקיצורים האלו לעבוד צריך: 1) להכניס את התיקיה בכניסה של התיקיות + להכניס את הקובץ ליב או סי פי פי 
// את התיקיה צריך להכניס ע"מ שלא נצתרך לכתוב את כל הנתיב כל פעם כמו בשורה למטה
// וככה ניתן לכתוב בקיצור כמו בשורה למעלה
//#include "C:\Users\zniri\Desktop\Coding\Languages\C++\C++ Libs\CpuUsage\CpuUsage.h" <- lib \ cpp
#include "C:\Users\zniri\Desktop\Coding\Languages\C++\C++ Libs\BigInt\BigInt.h"

#ifdef _DEBUG
#define LOG(x) cout << x << endl
#else
#define LOG(x) 
#endif // _DEBUG


using namespace std;
using namespace literals::chrono_literals;
using MapIter = unordered_map<thread::id, bool>::iterator;

/*Class*/

template<typename FuncType=void(*)(), typename BigNumType=int, typename WorkType = int>
class ThreadLoopPool
{
public:
	
	ThreadLoopPool();

	ThreadLoopPool(FuncType func, WorkType obj);

	//ThreadLoopPool(void(*f)());
	//~ThreadLoopPool();
	
	bool done;
	mutex muThreadRunNum;
	mutex muDone;
	mutex muMap;
	mutex muNull;
	condition_variable Tfunc;
	unordered_map<thread::id, bool> mp;
	CpuUsage usage;
	WorkType obj;
	FuncType func;
	//void(*func)(BigNumType range, WorkType obj);

	BigNumType thread_run_num = BigNumType(0);
	BigNumType step_leangth = BigNumType(1);
	unsigned int delay = 0;

	void linkTheFunc(FuncType f, WorkType obj);

	void wrapperFunc();

	template<typename AssigneFuncType, typename Targ>
	void wrapperAssigne(AssigneFuncType func, Targ arg);

	void update_map(unordered_map<thread::id, bool>& u_map, int range);

	short check_useg();

	void check_avalible_threads(unsigned short& avalible_threads);

	void dummyFunc(BigNumType& bi, bool& ddone);

	BigNumType set_step_leangth();

	void start();

private:

};


/*Definnitions*/

template<typename FuncType, typename BigNumType, typename WorkType>
ThreadLoopPool<FuncType, BigNumType, WorkType>::ThreadLoopPool()
{
	cout << "The empty constructer is working!\n";
}

template<typename FuncType, typename BigNumType, typename WorkType>
ThreadLoopPool<FuncType, BigNumType, WorkType>::ThreadLoopPool(FuncType f, WorkType obj) :
func{ f },
obj{ obj },
step_leangth(set_step_leangth())
{}

template<typename FuncType, typename BigNumType, typename WorkType>
void ThreadLoopPool<FuncType, BigNumType, WorkType>::linkTheFunc(FuncType f, WorkType obj)
{
	this->func = f;
	this->obj = obj;
	if (this->step_leangth == 1)
	{
		this->step_leangth = this->set_step_leangth();
	}
}

template<typename FuncType, typename BigNumType, typename WorkType>
template<typename AssigneFuncType, typename Targ>
void ThreadLoopPool<FuncType, BigNumType, WorkType>::wrapperAssigne(AssigneFuncType func, Targ arg)
{
	if (muDone.try_lock())
	{
		try
		{
			done = true;
			func(arg);
			muDone.unlock();
		}
		catch (...)
		{
			muDone.unlock();
		}
	}
}

template<typename FuncType, typename BigNumType, typename WorkType> // V
void ThreadLoopPool<FuncType, BigNumType, WorkType>::wrapperFunc()
{
	bool another_run;
	BigNumType range;

	while (!this->done)
	{
		this->muThreadRunNum.lock();
		range = this->thread_run_num * this->step_leangth;
		this->thread_run_num++;
		this->muThreadRunNum.unlock();

		if (this->mp.find(this_thread::get_id()) == this->mp.end())
		{
			this->mp[this_thread::get_id()] = true; // Inisialize the thread in the Map (Happend only the first time the thread reaches here)
		}

		this->func(range,this->obj);

		this->muMap.lock();
		another_run = this->mp[this_thread::get_id()] == true;
		this->muMap.unlock();
		if (!another_run)
		{
			unique_lock<mutex> ulNull(this->muNull);
			Tfunc.wait(ulNull, [this]() {return (this->mp[this_thread::get_id()] == true);}); // Wait for the bool value to be true
		}
		if (this->delay != 0)
		{
			this_thread::sleep_for(chrono::nanoseconds(this->delay));
		}
	}
}

template<typename FuncType, typename BigNumType, typename WorkType> // V
void ThreadLoopPool<FuncType, BigNumType, WorkType>::update_map(unordered_map<thread::id, bool>& u_map, int range)
{ // range = now avalible threads
	unsigned short prviosOnThreads = 0;

	if (range > u_map.size())
	{
		range = u_map.size();
	}

	for (auto& kav : u_map)
	{
		if (kav.second == true)
		{
			prviosOnThreads++;
		}
	}

	this->muMap.lock();
	for (auto& kav : u_map)
	{
		kav.second = false;
	}
	int i = 0;
	for (MapIter itr = u_map.begin(); i < range; itr++)
	{
		itr->second = true;
		i++;
	}
	this->muMap.unlock();

	for (unsigned short i = 0; i < (range - prviosOnThreads); i++)
	{
		this->Tfunc.notify_one(); // Make some already created threads that was asleep, wake.
	}

}

template<typename FuncType, typename BigNumType, typename WorkType> // V
short ThreadLoopPool<FuncType, BigNumType, WorkType>::check_useg()
{
	short program_usaged;
	short all_usaged;
	short* cpuUsage;
	cpuUsage = this->usage.GetUsage();
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

	if (gap > 25)
	{
		return 3;
	}
	if (gap > 18)
	{
		return 2;
	}
	if (gap > 7)
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
	if (gap < -7)
	{
		return -1;
	}
	return 0;

}

template<typename FuncType, typename BigNumType, typename WorkType> // V
void ThreadLoopPool<FuncType, BigNumType, WorkType>::check_avalible_threads(unsigned short& avalible_threads)
{
	short change = this->check_useg();
	cout << "Chabge -> " << change << "\n";

	if (change == 0)
		return;

	if (change > 0 && this->delay != 0)
	{
		this->delay = 0;
		return;
	}
	else if (avalible_threads + change > 0)
	{
		avalible_threads += change;
		this->delay = 0;
		return;
	}
	else
	{
		avalible_threads = 1;
		this->delay += (-change);
	}
}

template<typename FuncType, typename BigNumType, typename WorkType> // V
void ThreadLoopPool<FuncType, BigNumType, WorkType>::start()
{
	this->done = false;

	unsigned short max_threads = (short)thread::hardware_concurrency() * 5;

	vector<future<void>> fl;
	fl.reserve(max_threads);

	unsigned short new_threads = 1; //start with this num of threads
	unsigned short total_num_of_threads = new_threads;
	unsigned short avalible_threads = new_threads;
	unsigned long PREV_thread_run_num = 0;
	BigNumType PREV_gap(0);
	BigNumType gog(0); // Gap of gap
	BigNumType gap(0);

	while (!this->done)
	{
		if (new_threads > 0)
		{
			fl.emplace_back(async(launch::async, [this]() {this->wrapperFunc();}));
			new_threads--;
		}
		else
		{
			check_avalible_threads(avalible_threads);
			if (avalible_threads > total_num_of_threads)
			{
				new_threads = avalible_threads - total_num_of_threads;
				total_num_of_threads = avalible_threads;
			}
			update_map(this->mp, avalible_threads);
		}
		this_thread::sleep_for(5s);
		//cout << "\nThread run num: " << thread_run_num << "\n";
		//cout << "\nRate: " << ((long long)thread_run_num - PREV_thread_run_num) * Range / 5000000.0f << " MN/s \n";
		cout << "\nAvalible threads: " << avalible_threads << " \n";
		cout << "Delay = " << this->delay << "\n";
		cout << "Check num: " << (this->thread_run_num)*this->step_leangth << " \n";
		cout << "Steap: " << this->step_leangth << " \n";
		//gap = PREV_thread_run_num - (thread_run_num) * 100;
		//cout << "Gap: " << gap << " \n";
		//gog = PREV_gap - (PREV_thread_run_num - (thread_run_num) * 100);
		//cout << "Gap of gap: " << gog << " \n";
		//PREV_gap = (PREV_thread_run_num - (thread_run_num) * 100);
		//PREV_thread_run_num = thread_run_num;
	}


}

template<typename FuncType, typename BigNumType, typename WorkType> // V
void ThreadLoopPool<FuncType, BigNumType, WorkType>::dummyFunc(BigNumType& steap, bool& ddone)
{
	while (!ddone)
	{
		this->func(BigNumType(0), this->obj);
		steap++;

	}
}

template<typename FuncType, typename BigNumType, typename WorkType> // V
BigNumType ThreadLoopPool<FuncType, BigNumType, WorkType>::set_step_leangth()
{
	bool ddone = false;
	BigNumType steap(0);
	thread wrk([this](BigNumType& steap, bool& ddone) { this->dummyFunc(steap, ddone);}, ref(steap), ref(ddone)); 
	// /\ might have a prolem here with calling the refrence 2 times...
	// The main func
	this_thread::sleep_for(10s);
	ddone = true;
	wrk.join();
	LOG("Finished set_step_leangth");
	return steap;
}

