#ifndef __BLINK_PAIR_H
#define __BLINK_PAIR_H

template<class T1, class T2>
struct blink_pair
{
	typedef T1 first_type;
	typedef T2 second_type;

	T1 first;
	T2 second;
	blink_pair():first(T1()), second(T2()){}
	blink_pair(const T1& a, const T2& b):first(a),second(b){}

	template<class U1, class U2>
	blink_pair(const blink_pair<U1, U2>& p):first(p.first),second(p.second){}

};
#endif