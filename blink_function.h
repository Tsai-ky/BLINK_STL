#ifndef __BLINK_FUNCTION_H
#define __BLINK_FUNCTION_H

template<class Arg, class Result>
struct unary_function
{
	typedef Arg argument_type;
	typedef Result result_type;
};

template<class Arg1, class Arg2, class Result>
struct binary_function
{
	typedef Arg1 first_argument_type;
	typedef Arg2 second_argument_type;
	typedef Result result_type;
};

template<class T>
struct plus : public binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)const { return x + y; }
};

template<class T>
struct minus : public binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)const { return x - y; }
};

template<class T>
struct mulitplies :public binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)const { return x * y; }
};

template<class T>
struct devides :public binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)const { return x / y; }
};

template<class T>
struct modulus :public binary_function<T, T, T>
{
	T operator()(const T& x, const T& y)const { return x % y; }
};

// 证同运算
template<class T>
struct negate :public unary_function<T, T>
{
	T operator()(const T& x)const { return -x; }
};

template<class T>
inline T identity_element(plus<T>) { return T(0); }
template<class T>
inline T identity_element(mulitplies<T>) { return T(1); }

// 关系运算
template<class T>
struct equal_to :public binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x == y; }
};

template<class T>
struct not_equal_to :public binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x != y; }
};

template<class T>
struct greater : public binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x > y; }
};

template<class T>
struct less :public binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x < y; }
};

template<class T>
struct greater_equal :public binary_function < T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x >= y; }
};

template<class T>
struct less_equal :public binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x <= y; }
};

// 逻辑运算
template<class T>
struct logical_and :public binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x && y; }
};

template<class T>
struct logical_or :public binary_function<T, T, bool>
{
	bool operator()(const T& x, const T& y)const { return x || y; }
};

template <class T>
struct logical_not :public unary_function<T, bool>
{
	bool operator()(const T& x)const { return !x; }
};

// 证同运算
template<class T>
struct identity :public unary_function<T, T>
{
	const T& operator()(const T& x)const { return x; }
};

// 选择运算
// 接受一个pair，返回其第一元素
template<class Pair>
struct select1st :public unary_function<Pair, typename Pair ::first_type>
{
	const typename Pair::first_type& operator()(const Pair& x)const
	{
		return x.first;
	}
};

// 接受一个pair，返回其第二元素
template<class Pair>
struct select2nd :public unary_function<Pair, typename Pair :: second_type>
{
	const typename Pair::second_type& operator()(const Pair& x)const
	{
		return x.second;
	}
};

//投射运算
//返回第一参数，忽略第二参数
template<class Arg1, class Arg2>
struct project1st :public binary_function<Arg1, Arg2, Arg1>
{
	Arg1 operator()(const Arg1& x, const Arg1&)const { return x; }
};

// 返回第二参数，忽略第一参数
template<class Arg1, class Arg2>
struct project2nd :public binary_function<Arg1, Arg2, Arg2>
{
	Arg2 operator()(const Arg1&, const Arg2& y)const { return y; }
};

#endif