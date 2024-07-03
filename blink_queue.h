#ifndef __BLINK_QUEUE_H
#define __BLINK_QUEUE_H
#include"blink_deque.h"
#include"blink_function.h"
#include "blink_vector.h"
#include "blink_heap.h"

template<class T, class Sequence = blink_deque<T>>
class blink_queue
{
	template<class U, class S>
	friend bool operator==(const blink_queue<U, S>& x, const blink_queue<U, S>& y);

	template<class U, class S>
	friend bool operator<(const blink_queue<U, S>& x, const blink_queue<U, S>& y);

public:
	typedef typename Sequence::value_type value_type;
	typedef typename Sequence::size_type size_type;
	typedef typename Sequence::reference reference;
	typedef typename Sequence::const_reference const_reference;
protected:
	Sequence c;	//µ×²ãÈÝÆ÷
public:
	bool empty()const { return c.empty(); }
	size_type size()const{return c.size(); }
	reference front() { return c.front(); }
	const_reference front()const { return c.front(); }
	reference back() { return c.back(); }
	const_reference back()const { return c.back(); }
	void push(const value_type& x) { c.push_back(x); }
	void pop() { c.pop_front(); }
};

template<class T, class Sequence>
bool operator == (const blink_queue<T, Sequence>& x, const blink_queue<T, Sequence>& y)
{
	return x.c == y.c;
}

template<class T, class Sequence>
bool operator < (const blink_queue<T, Sequence>& x, const blink_queue<T, Sequence>& y)
{
	return x.c < y.c;
}

template<class T, class Sequence = blink_vector<T>,
	class Compare = less<typename Sequence::value_type>>
class blink_priority_queue
{
public:
	typedef typename Sequence::value_type value_type;
	typedef typename Sequence::size_type size_type;
	typedef typename Sequence::reference reference;
	typedef typename Sequence::const_reference const_reference;

protected:
	Sequence c;
	Compare comp;

public:
	blink_priority_queue():c(){}
	explicit blink_priority_queue(const Compare& x):c(),comp(x){}

	template<class InputIterator>
	blink_priority_queue(InputIterator first, InputIterator last, const Compare& x)
		: c(first, last), comp(x) 
	{
		make_heap(c.begin(), c.end(), comp);
	}

	template<class InputIterator>
	blink_priority_queue(InputIterator first, InputIterator last)
		: c(first, last) {make_heap(c.begin(), c.end(), comp); }

	bool empty()const { return c.empty(); }
	size_type size()const { return c.size(); }
	const_reference top()const { return c.front(); }

	void push(const value_type& x)
	{
		try
		{
			c.push_back(x);
			push_heap(c.begin(), c.end(), comp);
		}
		catch (...)
		{
			c.clear();
			throw;
		}
	}

	void pop()
	{
		try
		{
			pop_heap(c.begin(), c.end(), comp);
			c.pop_back();
		}
		catch (...)
		{
			c.clear();
			throw;
		}
	}
};
#endif 
