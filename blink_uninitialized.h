#ifndef __BLINK_UNINITIALIZED_H
#define __BLINK_UNINITIALIZED_H
#include<string.h>
#include"type_traits.h"
#include"blink_iterator.h"
#include"blink_algobase.h"

/*
* uninitialized_copy(), uninitialized_fill()和uninitialized_fill_n()
* 都能使内存配置与对象构造行为分离开来
*/

/*
* uninitialized_copy()会使用copy constructor
* 为输入范围[first, last)的每个对象产生一份复制品，放进result中
*/

template<class InpurIterator, class ForwardIterator, class T>
inline ForwardIterator
__uninitialized_copy(InpurIterator first, InpurIterator last, ForwardIterator result, T*)
{
	typedef typename __type_traits<T>::is_POD_type is_POD;
	return __uninitialized_copy_aux(first, last, result, is_POD());
}

template<class InputIterator, class ForwardIterator>
ForwardIterator
__uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __true_type)
{
	// 对于POD类型，调用高效的内存拷贝函数copy()
	return __BLINK::copy(first, last, result);
}

template<class InputIterator, class ForwardIterator>
ForwardIterator
__uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __false_type)
{
	// 对于non-POD类型，使用对应的构造函数更安全
	ForwardIterator cur = result;
	for (; first != last; ++first, ++cur)
	{
		construct(&*cur, *first);
	}
	return cur;
}

template<class InputIterator, class ForwardIterator>
ForwardIterator
uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
{
	return __uninitialized_copy(first, last, result, value_type(result));
	// 利用value_type()取出result的类型
}

// 针对 const char* 的特别版本
inline char* uninitialized_copy(const char* first, const char* last, char* result)
{
	memmove(result, first, last - first);
	return result + (last - first);
}

// 针对 const wchar_t* 的特别版本
inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result, first, sizeof(wchar_t) * (last - first));
	return result + (last - first);
}

/*
* uninitialized_fill()会在[first, last)范围内产生x的复制品
*/
template<class ForwardIterator, class T>
void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x)
{
	__uninitialized_fill(first, last, x, value_type(first));
	// value()取得first的类型
}

template<class ForwardIterator, class T, class T1>
inline void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, T1*)
{
	typedef typename __type_traits<T1>::is_POD_type is_POD;
	__uninitialized_fill_aux(first, last, x, is_POD());
}

template<class ForwardIterator, class T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __true_type)
{
	//对于POD类型，使用更高效的高阶函数fill()
	fill(first, last, x);
}

template<class ForwardIterator, class T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __false_type)
{
	// 对于non-POD类型，调用对应的构造函数更安全
	ForwardIterator cur = first;
	for (; cur != last; ++cur)
		construct(&*cur, x);
}


template<class ForwardIterator, class Size, class T>
ForwardIterator
uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{
	// value_type 提取 first 类型
	return __uninitialized_fill_n(first, n, x, value_type(first));
}

template<class ForwardIterator, class Size, class T, class T1>
inline ForwardIterator
__uninitialized_fill_n(ForwardIterator first, Size n, const T& x, T1*)
{
	typedef typename __type_traits<T1>::is_POD_type is_POD;
	return __uninitialized_fill_n_aux(first, n, x, is_POD());
}

template<class ForwardIterator, class Size, class T>
ForwardIterator
__uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __true_type)
{
	// 对于POD 类型交由具有高效率填充初值的高阶函数fill_n
	return __BLINK::fill_n(first, n, x);
}

template<class ForwardIterator, class Size, class T>
ForwardIterator
__uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __false_type)
{
	// 对于 non-POD 类型调用对应的构造函数更加安全
	ForwardIterator cur = first;
	for (; n > 0; --n, ++cur)
	{
		construct(&*cur, x);
	}
	return cur;
}
#endif // !BLINK_UNINITIALIZED

