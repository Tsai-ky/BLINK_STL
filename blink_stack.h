#ifndef __BLINK_STACK_H
#define __BLINK_STACK_H
#include"blink_deque.h"

template<class T, class Sequence = blink_deque<T>>
class blink_stack
{
	template<class U, class S>
	friend bool operator == (const blink_stack<U, S>&, const blink_stack<U, S>&);

	template<class U, class S>
	friend bool operator < (const blink_stack<U, S>&, const blink_stack<U, S>&);

public:
	typedef typename Sequence::value_type value_type;
	typedef typename Sequence::size_type size_type;
	typedef typename Sequence::reference reference;
	typedef typename Sequence::const_reference const_reference;

protected:
	Sequence c;

public:
	bool empty()const { return c.empty(); }
	size_type size()const { return c.size(); }
	reference top() { return c.back(); }
	const_reference top() const { return c.back(); }
	void push(const value_type& x) { c.push_back(x); }
	void pop() { c.pop_back(); }
};

template<class T, class Sequence>
bool operator == (const blink_stack<T, Sequence>& x, const blink_stack<T, Sequence>& y)
{
	return x.c == y.c;
}

template<class T, class Sequence>
bool operator < (const blink_stack<T, Sequence>& x, const blink_stack<T, Sequence>& y)
{
	return x.c < y.c;
}

#endif // !BLINK_STACK

