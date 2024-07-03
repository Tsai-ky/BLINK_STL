#ifndef __BLINK_ALGOBASE_H
#define __BLINK_ALGOBASE_H
#include"type_traits.h"
#include<string.h>
#include"blink_iterator.h"
#include "blink_config.h"


__BLINK_BEGIN_NAMESPACE
// ������ copy() ʵ��
// ���½�__copy_dispatch()��װ�ɽṹ������Ǻ�����ԭ��
// ����ģ�岻֧��ƫ�ػ���ֻ��ͨ����������ʵ�����ƹ���
// �ṹ��ģ��֧��ƫ�ػ�
template<class InputIterator, class OutputIterator>
struct __copy_dispatch
{
	OutputIterator operator()(InputIterator first, InputIterator last,
		OutputIterator result)
	{
		return __copy(first, last, result, iterator_category(first));
	}
};

// ָ����ָ����߱� trivial assignment operator
template<class T>
inline T* __copy_t(const T* first, const T* last, T* result, __true_type)
{
	memmove(result, first, sizeof(T) * (last - first));
	return result + (last - first);
}

// ��������������T* ָ����ʽ����ƫ�ػ�
template<class T>
struct __copy_dispatch<T*, T*>
{
	T* operator()(T* first, T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_t(first, last, result, t());
	}
};

// ��һ��������const T*��һ��������T* ����ƫ�ػ�
template<class T>
struct __copy_dispatch<const T*, T*>
{
	T* operator()(const T* first, const T* last, T* result)
	{
		typedef typename __type_traits<T>::has_trivial_assignment_operator t;
		return __copy_t(first, last, result, t());
	}
};

// InputIterator �汾
template<class InputIterator, class OutputIterator>
inline OutputIterator __copy(InputIterator first, InputIterator last,
	OutputIterator result, input_iterator_tag)
{
	// �Ե������Ƿ���ͬ����ѭ���Ƿ�������ٶ���
	for (; first != last; ++result, ++first)
		*result = *first;
	return result;
}

template<class RandomAccessIterator, class OutputIterator, class Distance>
inline OutputIterator
__copy_d(RandomAccessIterator first, RandomAccessIterator last,
	OutputIterator result, Distance*)
{
	// �� n ����ѭ���Ƿ���У��ٶȿ�
	for (Distance n = last - first; n > 0; --n, ++result, ++first)
		*result = *first;
	return result;
}

// RandomAccessIterator �汾
template<class RandomAccessIterator, class OutputIterator>
inline OutputIterator __copy(RandomAccessIterator first, RandomAccessIterator last,
	OutputIterator result, random_access_iterator_tag)
{
	return __copy_d(first, last, result, distance_type(first));
}



// ָ����ָ���󲻾߱� trivial assignment operator
template<class T>
inline T* __copy_t(const T* first, const T* last, T* result, __false_type)
{
	// ԭ��ָ����һ�� RandomAccessIterator
	return __copy_d(first, last, result, (ptrdiff_t*)0);
}

// ��ȫ�����汾
template<class InputIterator, class OutputIterator>
inline OutputIterator copy(InputIterator first, InputIterator last, OutputIterator result)
{
	// __copy_dispatch����װ���˽ṹ��
	return __copy_dispatch<InputIterator, OutputIterator>()(first, last, result);
}

// ���⻯const char*
inline char* copy(const char* first, const char* last, char* result)
{
	memmove(result, first, last - first);
	return result + (last - first);
}

// ���⻯const wchar_t*
inline wchar_t* copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result, first, sizeof(wchar_t) * (last - first));
	return result + (last - first);
}


// ������copy_backward()ʵ��
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
	// ԭ��ָ��Ҳ��һ��RandomAccessIterator
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

// InputIterator ��
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
	// �� n �ж�ѭ���Ƿ�������ٶȿ�
	while (first != last)
		*(--result) = *(--last);
	return result;
}

// RandomAccessIterator ��
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

// memmove�ڲ�ʵ�ֻ���
// 1. ��Դ�ڴ���׵�ַ����Ŀ���ڴ���׵�ַʱ���������κο���
// 2. ��Դ�ڴ���׵�ַ����Ŀ���ڴ���׵�ַʱ��ʵ�����򿽱�
// 3. ��Դ�ڴ���׵�ַС��Ŀ���ڴ���׵�ַʱ��ʵ�з��򿽱�
// ���⻯ const char*
inline char* copy_backward(const char* first, const char* last, char* result)
{
	memmove(result - (last - first), first, (last - first));
	return result - (last - first);
}

// ���⻯ const wchar*
inline wchar_t* copy_backward(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
	memmove(result - (last - first), first, sizeof(wchar_t) * (last - first));
	return result - (last - first);
}



// ��[first, last)������Ԫ�ظ�����ֵ
template<class ForwardIterator, class T>
void fill(ForwardIterator first, ForwardIterator last, const T& value)
{
	for (; first != last; ++first)
		*first = value;
}

// ��[first, last) �ڵ�ǰn��Ԫ�ظ�����ֵ��
// ���صĵ�����ָ����������һ��Ԫ�ص���һ��Ԫ��
template<class OutputIterator, class Size, class T>
OutputIterator fill_n(OutputIterator first, Size n, const T& value)
{
	// ע�� n ���ܳ�Խ���������д�С
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
	// �Զ���comp���жϴ�С
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
	// �Զ���comp���жϴ�С
	return comp(b, a) ? b : a;
}

// �Ե�����Ԫ��
template<class T>
inline void swap(T& a, T& b)
{
	T tmp = a;
	a = b;
	b = tmp;
}

// ����[first, last)��������valueֵ��ȵ�Ԫ�ظ���
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

// ��[first, last)����ֵΪvalue��Ԫ��
// �����ڣ��򷵻�һ��ָ���Ԫ�ص�InputIterator
// �������ڣ��򷵻�last
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
	// ֻ��RandomAccessIterator���Խ���first < last;
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
* [first, last)��Ԫ�طǽ�������
* ��[first, last)�о�����value��ͬ��Ԫ�أ��򷵻�ָ���Ԫ�صĵ�����
* �������ڣ����ڲ��ƻ�����״̬��ԭ���£��ҵ��ɲ���value�ĵ�һ��λ��
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

