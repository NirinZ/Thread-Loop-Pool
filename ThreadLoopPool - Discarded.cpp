#include "ThreadLoopPool.h"
#include <iostream>
using namespace std;

template<typename BigNumType>
ThreadLoopPool<BigNumType>::ThreadLoopPool(void)
{
	cout << "Empty CNS \n";
}

template<typename BigNumType>
ThreadLoopPool<BigNumType>::ThreadLoopPool(void(*f)(BigNumType range)) : func{ f },
step_leangth(set_step_leangth())
{}

template<typename BigNumType>
template<typename Func, typename Targ> // voud(*Func)(int)
void ThreadLoopPool<BigNumType>::wrapperAssigne(Func func, Targ arg)
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

template<typename BigNumType>
void ThreadLoopPool<BigNumType>::wrapperFunc()
{
	bool another_run;
	BigNumType range;

	while (!this->done)
	{
		this->muThreadNum.lock();
		this->thread_run_num++;
		range = this->thread_run_num * this->step_leangth;
		this->muThreadNum.unlock();

		if (this->mp.find(this_thread::get_id()) == this->mp.end())
		{
			this->mp[this_thread::get_id()] = true;
		}

		this->func(range);

		this->muMap.lock();
		another_run = this->mp[this_thread::get_id()] == true;
		this->muMap.unlock();
		if (!another_run)
		{
			unique_lock<mutex> ulNull(this->muNull);
			Tfunc.wait(ulNull, [this]() {return (this->mp[this_thread::get_id()] == true);});
		}
		if (this->delay != 0)
		{
			this_thread::sleep_for(chrono::nanoseconds(this->delay));
		}
	}
}

template<typename BigNumType>
void ThreadLoopPool<BigNumType>::update_map(unordered_map<thread::id, bool>& u_map, int range)
{
	unsigned short prvT = 0;

	if (range > u_map.size())
	{
		range = u_map.size();
	}

	for (auto& kav : u_map)
	{
		if (kav.second == true)
		{
			prvT++;
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

	for (unsigned short i = 0; i < (range - prvT); i++)
	{
		this->Tfunc.notify_one();
	}

}

template<typename BigNumType>
short ThreadLoopPool<BigNumType>::check_useg()
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

template<typename BigNumType>
void ThreadLoopPool<BigNumType>::check_avalible_threads(unsigned short& avalible_threads)
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

template<typename BigNumType>
void ThreadLoopPool<BigNumType>::start()
{
	this->done = false;

	vector<future<void>> fl;

	unsigned short new_threads = 1; //start with this num of threads
	unsigned short total_num_of_threads = new_threads;
	unsigned short avalible_threads = new_threads;
	unsigned long PREV_thread_run_num = 0;
	//unsigned long long PREV_gap = 0;
	//long long gog = 0;
	//long long gap = 0;

		while (!this->done)
		{
			if (new_threads > 0)
			{
				fl.push_back(async(launch::async, [this]() {this->wrapperFunc();}));
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
			cout << "Avalible threads: " << avalible_threads << " \n";
			cout << "Delay = " << this->delay << "\n";
			//cout << "Num: " << ((long long)thread_run_num)*100 << " \n";
			//gap = num - ((long long)thread_run_num) * 100;
			//cout << "Gap: " << gap << " \n";
			//gog = PREV_gap - (num - ((long long)thread_run_num) * 100);
			//cout << "Gap of gap: " << gog << " \n";
			//PREV_gap = (num - (unsigned long long)(thread_run_num) * (unsigned long long)100);
			//PREV_thread_run_num = (long long)thread_run_num;
		}


}

template<typename BigNumType>
void ThreadLoopPool<BigNumType>::dummyFunc(RossiBigInt &bi, bool &ddone)
{
	while (!ddone)
	{
		this->func(BigNumType(1));
		bi++;
	}
}

template<typename BigNumType>
BigNumType ThreadLoopPool<BigNumType>::set_step_leangth()
{
	bool ddone = false;
	RossiBigInt bi(0);
	thread wrk([this](RossiBigInt& bi, bool& ddone){ this->dummyFunc(bi, ddone);}, ref(bi), ref(ddone));
	this_thread::sleep_for(10s);
	ddone = true;
	wrk.join();
	return bi;
}
