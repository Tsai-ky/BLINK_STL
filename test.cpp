#include"blink_vector.h"
#include"blink_deque.h"
#include"blink_queue.h"
#include"blink_stack.h"
#include "blink_tree.h"
#include"blink_function.h"
#include "blink_set.h"
#include "blink_hashtable.h"
#include "blink_hash_func.h"
#include "blink_unordered_map.h"
#include<iostream>

struct eqstr
{
	bool operator()(const char* s1, const char* s2)const
	{
		return strcmp(s1, s2) == 0;
	}
};

int main()
{
	// test blink_vector
	std::cout << "test blink_vector" << std::endl;
	int a[] = { 0,1,2,3,4,5,6,7,8 };
	std::cout << "the value in the blink_vector:";
	for (int i = 0; i < 9; i++)
		std::cout << a[i] << ' ';
	std::cout << std::endl;

	// test blink_deque
	std::cout << "test blink_deque" << std::endl;
	blink_deque<int> dq;
	dq.push_back(1);
	dq.push_front(2);
	int len = dq.size();

	std::cout << "the value in the blink_deque:";
	for (int i = 0; i < len; ++i)
		std::cout << dq[i] << ' ';
	std::cout << std::endl;

	// test blink_queue
	std::cout << "test blink_queue" << std::endl;
	blink_queue<int> q;
	q.push(1);
	q.push(2);
	std::cout << "the value at the front of blink_queue:";
	std::cout << q.front();
	std::cout << std::endl;

	std::cout << "test blink_priority_queue" << std::endl;
	int ia[9] = { 0,1,2,3,4,8,9,3,5 };
	blink_priority_queue<int> ipq(ia, ia + 9);
	std::cout << "size=" << ipq.size() << std::endl;
	while (!ipq.empty())
	{
		std::cout << ipq.top() << ' ';
		ipq.pop();
	}
	std::cout << std::endl;

	
	// test blink_stack
	std::cout << "test blink_stack" << std::endl;
	blink_stack<int> s;
	s.push(2);
	s.push(1);
	std::cout << "the value at the top of blink_stack:";
	std::cout << s.top() << std::endl;

	std::cout << "test blink_tree" << std::endl;
	blink_rb_tree<int, int, identity<int>, less<int>>itree;
	std::cout << "the size of blink_tree:";
	std::cout << itree.size() << std::endl;
	itree.insert_unique(10);
	itree.insert_unique(7);
	itree.insert_unique(8);
	itree.insert_unique(15);
	itree.insert_unique(5);
	itree.insert_unique(6);
	itree.insert_unique(11);
	itree.insert_unique(13);
	itree.insert_unique(12);

	std::cout << "the size of blink_tree after insertion:";
	std::cout << itree.size() << std::endl;

	blink_rb_tree<int, int, identity<int>, less<int>>::iterator
		ite1 = itree.begin();
	blink_rb_tree<int, int, identity<int>, less<int>>::iterator
		ite2 = itree.end();

	for (; ite1 != ite2; ++ite1)
	{
		std::cout << *ite1 << ' ';
	}
	std::cout << std::endl;

	std::cout << "test blink_set" << std::endl;
	blink_set<int> iset(a, a + 9);
	std::cout << "size=" << iset.size() << std::endl;
	std::cout << "3 count=" << iset.count(3) << std::endl;

	std::cout << "test blink_hashtable" << std::endl;
	hashtable<int,
		int,
		hash<int>,
		identity<int>,
		equal_to<int>,
		alloc>
		iht(50, hash<int>(), equal_to<int>());

	std::cout << iht.size() << std::endl;
	std::cout << iht.bucket_count() << std::endl;
	std::cout << iht.max_bucket_count() << std::endl;

	iht.insert_unique(59);
	iht.insert_unique(63);
	iht.insert_unique(108);
	iht.insert_unique(2);
	iht.insert_unique(53);
	iht.insert_unique(55);

	std::cout << iht.size() << std::endl;

	hashtable<int,
		int,
		hash<int>, 
		identity<int>, 
		equal_to<int>, 
		alloc>::iterator ite = iht.begin();

	for (int i = 0; i < iht.size(); ++i, ++ite)
	{
		std::cout << *ite << ' ';
	}
	std::cout << std::endl;

	std::cout << "test blink_unordered_map" << std::endl;
	blink_unordered_map<const char*, int, hash<const char*>, eqstr>days;
	days["january"] = 31;
	days["february"] = 28;
	days["march"] = 31;
	days["april"] = 30;
	days["may"] = 31;
	days["june"] = 30;
	days["july"] = 31;
	days["august"] = 31;
	days["september"] = 30;
	days["october"] = 31;
	days["november"] = 30;
	days["december"] = 31;

	std::cout << "june->" << days["june"] << std::endl;

	blink_unordered_map<const char*, int, hash<const char*>, eqstr>::iterator
		ihte1 = days.begin();
	blink_unordered_map<const char*, int, hash<const char*>, eqstr>::iterator
		ihte2 = days.end();

	for (; ihte1 != ihte2; ++ihte1)
		std::cout << ihte1->first << ' ';
	return 0;
}