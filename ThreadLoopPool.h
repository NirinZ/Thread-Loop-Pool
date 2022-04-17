/**********************************************
* ~ DinamicThreadPool ~
* A generic class ment to help with threading over a certion function.
* This the the Dinamic Thread Loop version.
* On this version, the main functoin accures outside this file by the user.
* Most of the parameters, including the `range` the functoin check,
* should be implomented manually by the user.
* This making DinamicLoopPool more versitile for other
* dinamic threding necessities, that don't necceserily
* include repitting the same loop.
*
* The function needs to receve 2 arguments:
* 1) range - Usfule especially for loops
* 2) other - This will mostly be a struct of the parameters the function should receive
* 
* The assigne finction works the same...
***********************************************/


#pragma once
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <mutex>
#include <vector>
//#include <concepts>
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


template<typename T>
concept Number = requires(T t1, T t2)
{
	t1++;
	t1--;
	t1 + t2;
	t1 - t2;
	t1* t2;
	t1 / t2;
	t1 == t2;
	t1 != t2;
	t1 < t2;
	t1 > t2;
	t1 <= t2;
	t1 >= t2;
};

/*Class*/

template<typename FuncType=void(*)(int, int), Number BigNumType=int, typename Struct = int>
class ThreadLoopPool
{
public:
	
	ThreadLoopPool();

	ThreadLoopPool(FuncType func, Struct obj);

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
	Struct obj;
	FuncType func;
	//void(*func)(BigNumType range, WorkType obj);

	BigNumType thread_run_num = BigNumType(0);
	BigNumType step_leangth = BigNumType(1);
	unsigned int delay = 0;

	void linkTheFunc(FuncType f, Struct obj);

	void wrapperFunc();

	template<typename AssigneFuncType, typename TypeArg>
	void wrapperAssigne(AssigneFuncType func, TypeArg arg);

	void update_map(unordered_map<thread::id, bool>& u_map, int range);

	short check_useg();

	void check_avalible_threads(unsigned short& avalible_threads);

	void dummyFunc(BigNumType& bi, bool& ddone);

	BigNumType set_step_leangth();

	void start();

private:

};


/*Definnitions*/

template<typename FuncType, typename BigNumType, typename Struct>
ThreadLoopPool<FuncType, BigNumType, Struct>::ThreadLoopPool()
{
	LOG("The empty constructer is working!");
}

template<typename FuncType, typename BigNumType, typename Struct>
ThreadLoopPool<FuncType, BigNumType, Struct>::ThreadLoopPool(FuncType f, Struct obj) :
func{ f },
obj{ obj },
step_leangth(set_step_leangth())
{
	LOG("The ACTUAL constructer is working!");
}

template<typename FuncType, typename BigNumType, typename Struct>
void ThreadLoopPool<FuncType, BigNumType, Struct>::linkTheFunc(FuncType f, Struct obj)
{
	this->func = f;
	this->obj = obj;
	if (this->step_leangth == 1)
	{
		this->step_leangth = this->set_step_leangth();
	}
}

template<typename FuncType, typename BigNumType, typename Struct>
template<typename AssigneFuncType, typename TypeArg>
void ThreadLoopPool<FuncType, BigNumType, Struct>::wrapperAssigne(AssigneFuncType func, TypeArg arg)
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

template<typename FuncType, typename BigNumType, typename Struct> // V
void ThreadLoopPool<FuncType, BigNumType, Struct>::wrapperFunc()
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

									 // The main function
		func(range, obj); // לפי הגרסה הזו, בתוך הפונק של הלקוח אמורה להיות הבדיקה של נכון או לא

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

template<typename FuncType, typename BigNumType, typename Struct> // V
void ThreadLoopPool<FuncType, BigNumType, Struct>::update_map(unordered_map<thread::id, bool>& u_map, int range)
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

template<typename FuncType, typename BigNumType, typename Struct> // V
short ThreadLoopPool<FuncType, BigNumType, Struct>::check_useg()
{
	CpuUsage::CpuValues cpuValues = this->usage.GetUsage();

	//30/100
	float should_be = (cpuValues.system_usage * 30) / 100;
	float now = cpuValues.program_usage * 100.0 / cpuValues.system_usage;
	float gap = should_be - cpuValues.program_usage;

	LOG("PROG -> " << cpuValues.program_usage);
	LOG("ALL -> " << cpuValues.system_usage << "%");
	LOG("NOW -> " << now << "% (of 30%)");
	LOG("SHOULD -> " << should_be << "%");
	LOG("GAP -> " << gap << "%");

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

template<typename FuncType, typename BigNumType, typename Struct> // V
void ThreadLoopPool<FuncType, BigNumType, Struct>::check_avalible_threads(unsigned short& avalible_threads)
{
	short change = this->check_useg();
	LOG("Chabge -> " << change);

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

template<typename K, typename V>
inline void print_map(std::unordered_map<K, V> const& m)
{
	for (auto const& pair : m) {
		std::cout << "{" << pair.first << ": " << boolalpha << pair.second << "}\n";
	}
}

template<typename FuncType, typename BigNumType, typename Struct> // V
void ThreadLoopPool<FuncType, BigNumType, Struct>::start()
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
		while (new_threads--)
		{
			fl.emplace_back(async(launch::async, [this]() {this->wrapperFunc();}));
		} new_threads++; // because this operation decrece the value 2 times, so it return to the max of the U_short

		check_avalible_threads(avalible_threads);
		if (avalible_threads > total_num_of_threads)
		{
			new_threads = avalible_threads - total_num_of_threads;
			total_num_of_threads = avalible_threads;
		}
		update_map(this->mp, avalible_threads);
		
		//cout << "\nThread run num: " << thread_run_num << "\n";
		//cout << "\nRate: " << ((long long)thread_run_num - PREV_thread_run_num) * Range / 5000000.0f << " MN/s \n";
		LOG("Avalible threads : " << avalible_threads);
		LOG("Delay = " << this->delay);
		LOG("Check num: " << (this->thread_run_num)*this->step_leangth);
		LOG("Steap: " << this->step_leangth);
		LOG("Map: ");
		#ifdef DEBUG
		print_map(mp);
		#endif // DEBUG
		LOG("\n-------------------------\n");

		//gap = PREV_thread_run_num - (thread_run_num) * 100;
		//cout << "Gap: " << gap << " \n";
		//gog = PREV_gap - (PREV_thread_run_num - (thread_run_num) * 100);
		//cout << "Gap of gap: " << gog << " \n";
		//PREV_gap = (PREV_thread_run_num - (thread_run_num) * 100);
		//PREV_thread_run_num = thread_run_num;

		this_thread::sleep_for(5s);
	}

	Tfunc.notify_all();
}

template<typename FuncType, typename BigNumType, typename Struct> // V
void ThreadLoopPool<FuncType, BigNumType, Struct>::dummyFunc(BigNumType& steap, bool& ddone)
{
	while (!ddone)
	{
		this->func(BigNumType(0), this->obj);
		steap++;

	}
}

template<typename FuncType, typename BigNumType, typename Struct> // V
BigNumType ThreadLoopPool<FuncType, BigNumType, Struct>::set_step_leangth()
{
	bool ddone = false;
	BigNumType steap(0);
	thread wrk([this](BigNumType& steap, bool& ddone) { this->dummyFunc(steap, ddone);}, ref(steap), ref(ddone)); 
	// /\ The main func
	this_thread::sleep_for(10s); // 10
	ddone = true;
	wrk.join();
	LOG("Finished set_step_leangth\n");
	return steap;
	// יש כאן בעיה רציני אם איך שאני קורא לפונק' ומתשמש בטווח
	// אם הפונק לא מקבלת טווח הבדיקת טווח תרוץ לנצח
	// ואם כן, אני לא שולח טוב את הטווח בראפר למעלה! אבל אני חייב לישון
}

