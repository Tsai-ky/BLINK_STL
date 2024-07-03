#ifndef __BLINK_ALGOBASE_H
#define __BLINK_ALGOBASE_H
#include"type_traits.h"
#include<string.h>
#include"blink_iterator.h"
#include "blink_config.h"


__BLINK_BEGIN_NAMESPACE
// 以下是 copy() 实现
// 以下将__copy_dispatch()封装成结构体而不是函数的原因：
// 函数模板不支持偏特化，只能通过函数重载实现类似功能
// 结构体模板支持偏特化
template<class InputIterator, class OutputIterator>
struct __copy_dispatch
{
	OutputIterator operator()(InputIterator first, InputIterator last,
		OutputIterator result)
	{
		return __copy(first, last, result, iterator_category(first));
	}
};

// 指针所指对象具备 trivial assignment operator
template<class T>
inline T* __copy_t(const T* first, const T* last, T* result, __true_type)
{
	memmove(result, first, sizeof(T) * (last - first));
	return result + (last - first);
}

// 对两个参数都是T* 指针形式进行偏特化
template<class T>
struct __copy_dispatch<T*, T*>
{
	T* operator()(T* first, T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_t(first, last, result, t());
	}
};

// 对一个参数是const T*另一个参数是T* 进行偏特化
template<class T>
struct __copy_dispatch<const T*, T*>
{
	T* operator()(const T* first, const T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_t(first, last, result, t());
	}
};

// InputIterator 版本
template<class InputIterator, class OutputIterator>
inline OutputIterator __copy(InputIterator first, InputIterator last,
	OutputIterator result, input_iterator_tag)
{
	// 以迭代器是否相同决定循环是否继续，速度慢
	for (; first != last; ++result, ++first)
		*result = *first;
	return result;
}

template<class RandomAccessIterator, class OutputIterator, class Distance>
inline OutputIterator
__copy_d(RandomAccessIterator first, RandomAccessIterator last,
	OutputIterator result, Distance*)
{
	// 以 n 决定循环是否进行，速度快
	for (Distance n = last - first; n > 0; --n, ++result, ++first)
		*result = *first;
	return result;
}

// RandomAccessIterator 版本
template<class RandomAccessIterator, class OutputIterator>
inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last,
	OutputIterator result, random_access_iterator_tag)
{
	return __copy_d(first, last, result, distance_type(first));
}



// 指针所指对象不具备 trivial assignment operator
template<class T>
inline T* __copy_t(const T* first, const T* last, T* result, __false_type)
{
	// 原生指针是一种 RandomAccessIterator
	return __copy_d(first, last, result, (ptrdiff_t*)0);
}

// 完全泛化版本
template<class InputIterator, class OutputIterator>
inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
{
	// __copy_dispatch被封装成了结构体
	return __copy_dispatch<InputIterator, OutputIterator>()(first, last, result);
}

// 特殊化const char*
inline char* copy(const char* first, const char* last, char* result)
{
	memmove(result, first, last - first);
	return result + (last - first);
}

// 特殊化const wchar_t*
inline wchar_t* copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result, first, sizeof(wchar_t) * (last - first));
	return result + (last - first);
}


// 以下是copy_backward()实现
template<class BidirectionalIterator1, class BidirectionalIterator2>
struct __copy_backward_dispatch
{
	BidirectionalIterator2 operator()(BidirectionalIterator1 first, BidirectionalIterator1 last, 
		BidirectionalIterator2 result)
	{
		return __copy_backwawrd(first, last, result, iterator_category(result));
	}
};

template<class T1, class T2>
inline T2* __copy_backward_t(T1* first, T1* last,
	T2* result, __true_type)
{
	memmove(result - (last - first), first, (last - first));
	return result - (last - first);
}

template<class T1, class T2>
inline T2 __copy_backward_t(T1 first, T1 last,
	T2 result, __false_type)
{
	// 原生指针也是一种RandomAccessIterator
	return __copy_backward_d(first, last, result, (ptrdiff_t*)0);
}

template<class T1, class T2>
struct __copy_backward_dispatch<T1*, T2*>
{
	T2* operator()(T1* first, T1* last, T2* result)
	{
		typedef typename __type_traits<T2>::has_trivial_assignment_operator t;
		return __copy_backward_t(first, last, result, t());
	}
};

template<class T1, class T2>
struct __copy_backward_dispatch<const T1*, T2*>
{
	T2* operator()(T1* first, T1* last, T2* result)
	{
		typedef typename __type_traits<T2>::has_trivial_assignment_operator t;
		return __copy_backward_t(first, last, result, t());
	}
};

// InputIterator 版
template<class BidirectionalIterator1, class BidirectionalIterator2>
inline BidirectionalIterator2 __copy_backward(BidirectionalIterator1 first, BidirectionalIterator1 last,
	BidirectionalIterator2 result, input_iterator_tag)
{
	while (first != last)
		*(--result) = *(--last);
}

template<class RandomAccessIterator1, class RandomAccessIterator2, class Distance>
inline RandomAccessIterator2 __copy_backward_d(RandomAccessIterator1 first, RandomAccessIterator1 last,
	RandomAccessIterator2 result, Distance*)
{
	// 以 n 判断循环是否继续，速度快
	while (first != last)
		*(--result) = *(--last);
	return result;
}

// RandomAccessIterator 版
template<class RandomAccessIterator1, class RandomAccessIterator2>
inline RandomAccessIterator2 __copy_backward(RandomAccessIterator1 first, RandomAccessIterator1 last,
	RandomAccessIterator2 result, random_access_iterator_tag)
{
	return __copy_backward_d(first, last, result, distance_type(result));
}


template<class BidirectionalIterator1, class BidirectionalIterator2>
inline BidirectionalIterator2 copy_backward(BidirectionalIterator1 first, BidirectionalIterator1 last, BidirectionalIterator2 result)
{
	return __copy_backward_dispatch<BidirectionalIterator1, BidirectionalIterator2>()(first, last, result);
}

// memmove内部实现机制
// 1. 当源内存的首地址等于目标内存的首地址时，不进行任何拷贝
// 2. 当源内存的首地址大于目标内存的首地址时，实行正向拷贝
// 3. 当源内存的首地址小于目标内存的首地址时，实行反向拷贝
// 特殊化 const char*
inline char* copy_backward(const char* first, const char* last, char* result)
{
	memmove(result - (last - first), first, (last - first));
	return result - (last - first);
}

// 特殊化 const wchar*
inline wchar_t* copy_backward(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result - (last - first), first, sizeof(wchar_t) * (last - first));
	return result - (last - first);
}



// 将[first, last)内所有元素改填新值
template<class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, const T& value)
{
	for (; first != last; ++first)
		*first = value;
}

// 将[first, last) 内的前n个元素改填新值，
// 返回的迭代器指向被填入的最后一个元素的下一个元素
template<class OutputIterator, class Size, class T>
OutputIterator fill_n(OutputIterator first, Size n, const T& value)
{
	// 注意 n 不能超越容器的现有大小
	for (; n > 0; --n, ++first)
		*first = value;
	return first;
}

template<class ForwardIterator1, class ForwardIterator2>
inline void iter_swap(ForwardIterator1 a, ForwardIterator2 b)
{
	__iter_swap(a, b, value_type(a));
}

template<class ForwardIterator1, class ForwardIterator2, class T>
inline void __iter_swap(ForwardIterator1 a, ForwardIterator2 b, T*)
{
	T tmp = *a;
	*a = *b;
	*b = tmp;
}

template<class T>
inline const T& max(const T& a, const T& b)
{
	return a < b ? b : a;
}

template<class T, class Compare>
inline const T& max(const T& a, const T& b, Compare comp)
{
	// 自定义comp来判断大小
	return comp(a, b) ? b:a;
}

template<class T>
inline const T& min(const T& a, const T& b)
{
	return b < a ? b : a;
}

template<class T, class Compare>
inline const T& min(const T& a, const T& b, Compare comp)
{
	// 自定义comp来判断大小
	return comp(b, a) ? b : a;
}

// 对调两个元素
template<class T>
inline void swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

// 计算[first, last)区间内与value值相等的元素个数
template<class InputIterator, class T>
typename iterator_traits<InputIterator>::difference_type
count(InputIterator first, InputIterator last, const T& value)
{
	typename iterator_traits<InputIterator>::difference_type n = 0;
	for (; first != last; ++first)
		if (*first != value)
			++n;
	return n;
}

// 在[first, last)查找值为value的元素
// 若存在，则返回一个指向该元素的InputIterator
// 若不存在，则返回last
template<class InputIterator, class T>
InputIterator find(InputIterator first, InputIterator last,
	const T& value)
{
	while (first != last && *first != value)
		++first;
	return first;
}


template<class BidirectionalIterator>
inline void reverse(BidirectionalIterator first, BidirectionalIterator last)
{
	__reverse(first, last, iterator_category(first));
}

template<class BidirectionalIterator>
void __reverse(BidirectionalIterator first, BidirectionalIterator last,
	bidirectional_iterator_tag)
{
	while (true)
		if (first != last || first == --last)
			return;
		else
			iter_swap(first++, last);
}

template<class RandomAccessIterator>
void __reverse(RandomAccessIterator first, RandomAccessIterator last,
	random_access_iterator_tag)
{
	// 只有RandomAccessIterator可以进行first < last;
	while (first < last) iter_swap(first++, --last);
}

template<class ForwardIterator, class T, class Distance>
ForwardIterator __lower_bound(ForwardIterator first,
	ForwardIterator last, const T& value, Distance*, forward_iterator_tag)
{
	Distance len = 0;
	distance(first, last, len);
	Distance half;
	ForwardIterator middle;

	while (len > 0)
	{
		half = len >> 1;
		middle = first;
		advance(middle, half);
		if (*middle < value)
		{
			first = middle;
			++first;
			len = len - half - 1;
		}
		else
			len = half;
	}
	return first;
}

template<class RandomAccessIterator, class T, class Distance>
RandomAccessIterator __lower_bound(RandomAccessIterator first,
	RandomAccessIterator last, const T& value, Distance*, random_access_iterator_tag)
{
	Distance len = last - first;
	Distance half;
	RandomAccessIterator middle;

	while (len > 0)
	{
		half = len >> 1;
		middle = first + half;
		if (*middle < value)
		{
			first = middle + 1;
			len = len - half - 1;
		}
		else
			len = half;
	}
	return first;
}
/* lower_bound 
* [first, last)内元素非降序排列
* 在[first, last)中具有与value相同的元素，则返回指向该元素的迭代器
* 若不存在，则在不破坏排序状态的原则下，找到可插入value的第一个位置
*/
template<class ForwardIterator, class T>
inline ForwardIterator lower_bound(ForwardIterator first, 
	ForwardIterator last, const T& value)
{
	return __lower_bound(first, last, value, distance_type(first), iterator_category(first));
}

template<class ForwardIterator, class T, class Compare>
inline ForwardIterator lower_bound(ForwardIterator first, 
	ForwardIterator last, const T& value, Compare comp)
{
	return __lower_bound(first, last, value, comp, distance_type(first), iterator_category(first));
}
__BLINK_END_NAMESPACE
#endif // !BLINK_ALGOBASE

