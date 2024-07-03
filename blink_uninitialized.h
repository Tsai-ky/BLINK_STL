#ifndef __BLINK_UNINITIALIZED_H
#define __BLINK_UNINITIALIZED_H
#include<string.h>
#include"type_traits.h"
#include"blink_iterator.h"
#include"blink_algobase.h"

/*
* uninitialized_copy(), uninitialized_fill()��uninitialized_fill_n()
* ����ʹ�ڴ��������������Ϊ���뿪��
*/

/*
* uninitialized_copy()��ʹ��copy constructor
* Ϊ���뷶Χ[first, last)��ÿ���������һ�ݸ���Ʒ���Ž�result��
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
	// ����POD���ͣ����ø�Ч���ڴ濽������copy()
	return __BLINK::copy(first, last, result);
}

template<class InputIterator, class ForwardIterator>
ForwardIterator
__uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, __false_type)
{
	// ����non-POD���ͣ�ʹ�ö�Ӧ�Ĺ��캯������ȫ
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
	// ����value_type()ȡ��result������
}

// ��� const char* ���ر�汾
inline char* uninitialized_copy(const char* first, const char* last, char* result)
{
	memmove(result, first, last - first);
	return result + (last - first);
}

// ��� const wchar_t* ���ر�汾
inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result, first, sizeof(wchar_t) * (last - first));
	return result + (last - first);
}

/*
* uninitialized_fill()����[first, last)��Χ�ڲ���x�ĸ���Ʒ
*/
template<class ForwardIterator, class T>
void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x)
{
	__uninitialized_fill(first, last, x, value_type(first));
	// value()ȡ��first������
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
	//����POD���ͣ�ʹ�ø���Ч�ĸ߽׺���fill()
	fill(first, last, x);
}

template<class ForwardIterator, class T>
void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, __false_type)
{
	// ����non-POD���ͣ����ö�Ӧ�Ĺ��캯������ȫ
	ForwardIterator cur = first;
	for (; cur != last; ++cur)
		construct(&*cur, x);
}


template<class ForwardIterator, class Size, class T>
ForwardIterator
uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{
	// value_type ��ȡ first ����
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
	// ����POD ���ͽ��ɾ��и�Ч������ֵ�ĸ߽׺���fill_n
	return __BLINK::fill_n(first, n, x);
}

template<class ForwardIterator, class Size, class T>
ForwardIterator
__uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, __false_type)
{
	// ���� non-POD ���͵��ö�Ӧ�Ĺ��캯�����Ӱ�ȫ
	ForwardIterator cur = first;
	for (; n > 0; --n, ++cur)
	{
		construct(&*cur, x);
	}
	return cur;
}
#endif // !BLINK_UNINITIALIZED

