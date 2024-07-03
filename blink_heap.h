#ifndef __BLINK_HEAP_H
#define __BLINK_HEAP_H

#include "blink_iterator.h"
#include "type_traits.h"

template<class RandomAccessIterator, class Distance, class T>
void __push_heap(RandomAccessIterator first, Distance holeIndex, 
	Distance topIndex, T value)
{
	Distance parent = (holeIndex - 1) / 2;  // �ҳ����ڵ�
	while (holeIndex > topIndex && *(first + parent) < value)
	{
		// ��δ����Ѷ����Ҹ��ڵ�С����ֵ
		*(first + holeIndex) = *(first + parent);
		holeIndex = parent;
		parent = (holeIndex - 1) / 2;
	}
	*(first + holeIndex) = value;
}

template<class RandomAccessIterator, class Distance, class T>
inline void __push_heap_aux(RandomAccessIterator first, 
	RandomAccessIterator last, Distance*, T*)
{
	__push_heap(first, Distance((last - first) - 1), Distance(0), T(*(last - 1)));
}

template<class RandomAccessIterator>
inline void push_heap(RandomAccessIterator first,
	RandomAccessIterator last)
{
	__push_heap_aux(first, last, distance_type(first), value_type(first));
}

template<class RandomAccessIterator, class Distance, class T, 
	class Compare>
void __push_heap(RandomAccessIterator first, Distance holeIndex,
	Distance topIndex, T value, Compare comp)
{
	Distance parent = (holeIndex - 1) / 2;  // �ҳ����ڵ�
	while (holeIndex > topIndex && comp(*(first + parent), value))
	{
		// ��δ����Ѷ����Ҹ��ڵ�С����ֵ
		*(first + holeIndex) = *(first + parent);
		holeIndex = parent;
		parent = (holeIndex - 1) / 2;
	}
	*(first + holeIndex) = value;
}

template<class RandomAccessIterator, class Compare, 
	class Distance, class T>
inline void __push_heap_aux(RandomAccessIterator first,
	RandomAccessIterator last, Compare comp, 
	Distance*, T*)
{
	__push_heap(first, Distance((last - first) - 1), 
		Distance(0), T(*(last - 1)), comp);
}

template<class RandomAccessIterator, class Compare>
inline void push_heap(RandomAccessIterator first,
	RandomAccessIterator last, Compare comp)
{
	__push_heap_aux(first, last, comp,
		distance_type(first), value_type(first));
}

template<class RandomAccessIterator, class Distance, class T>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex, Distance len, T value)
{
	Distance topIndex = holeIndex;
	Distance secondChild = 2 * holeIndex + 2; // ���ڵ���ҽڵ�
	while (secondChild < len)
	{
		if (*(first + secondChild) < *(first + (secondChild - 1)))
			secondChild--;
		*(first + holeIndex) = *(first + secondChild);
		holeIndex = secondChild;
		secondChild = 2 * (secondChild + 1);
	}
	if (secondChild == len) // û�����ӽڵ㣬ֻ�����ӽڵ�
	{
		*(first + holeIndex) = *(first + (secondChild - 1));
		holeIndex = secondChild - 1;
	}
	// ��������ֵ����Ŀ��λ��
	__push_heap(first, holeIndex, topIndex, value);
}

template<class RandomAccessIterator, class T, class Distance>
inline void __pop_heap(RandomAccessIterator first,
	RandomAccessIterator last,
	RandomAccessIterator result,
	T value,
	Distance*)
{
	/* pop�����Ľ��ӦΪ�ײ������ĵ�һ��Ԫ��
	* �����趨Ԥ����ֵΪβֵ
	* Ȼ����ֵ����β�ڵ�
	* Ȼ�����µ���[first, last-1��
	*/
	*result = *first;
	__adjest_heap(first, Distance(0), Distance(last - first), value);
}

template<class RandomAccessIterator, class T>
inline void __pop_heap_aux(RandomAccessIterator first, RandomAccessIterator last, T*)
{
	__pop_heap(first, last - 1, last - 1, T(*(last - 1)), distance(first));
}

template<class RandomAccessIterator>
inline void pop_heap(RandomAccessIterator first,
	RandomAccessIterator last)
{
	__pop_heap_aux(first, last, value_type(first));
}

template<class RandomAccessIterator, class Distance, 
	class T, class Compare>
void __adjust_heap(RandomAccessIterator first, Distance holeIndex, 
	Distance len, T value, Compare comp)
{
	Distance topIndex = holeIndex;
	Distance secondChild = 2 * holeIndex + 2; // ���ڵ���ҽڵ�
	while (secondChild < len)
	{
		if (comp(*(first + secondChild), *(first + (secondChild - 1))))
			secondChild--;
		*(first + holeIndex) = *(first + secondChild);
		holeIndex = secondChild;
		secondChild = 2 * (secondChild + 1);
	}
	if (secondChild == len) // û�����ӽڵ㣬ֻ�����ӽڵ�
	{
		*(first + holeIndex) = *(first + (secondChild - 1));
		holeIndex = secondChild - 1;
	}
	// ��������ֵ����Ŀ��λ��
	__push_heap(first, holeIndex, topIndex, value, comp);
}

template<class RandomAccessIterator, class T, class Compare,
	class Distance>
inline void __pop_heap(RandomAccessIterator first, RandomAccessIterator last,
	RandomAccessIterator result, T value, Compare comp,
	Distance*)
{
	/* pop�����Ľ��ӦΪ�ײ������ĵ�һ��Ԫ��
	* �����趨Ԥ����ֵΪβֵ
	* Ȼ����ֵ����β�ڵ�
	* Ȼ�����µ���[first, last-1��
	*/
	*result = *first;
	__adjust_heap(first, Distance(0), Distance(last - first), value, comp);
}

template<class RandomAccessIterator, class T, class Compare>
inline void __pop_heap_aux(RandomAccessIterator first, 
	RandomAccessIterator last, T*, Compare comp)
{
	__pop_heap(first, last - 1, last - 1, T(*(last - 1)), comp, 
		distance_type(first));
}

template<class RandomAccessIterator, class Compare>
inline void pop_heap(RandomAccessIterator first,
	RandomAccessIterator last, Compare comp)
{
	__pop_heap_aux(first, last, value_type(first), comp);
}

template<class RandomAccessIterator, class T, class Distance>
void __make_heap(RandomAccessIterator first, 
	RandomAccessIterator last, T*, Distance*)
{
	if (last - first < 2)return; // �������Ϊ0��1��������������
	Distance len = last - first;
	Distance parent = (len - 2) / 2; 
	while (true)
	{
		__adjust_heap(first, parent, len, T(*(first + parent)));
		if (parent == 0)return;
		parent--;
	}
}

template<class RandomAccessIterator>
inline void make_heap(RandomAccessIterator first, RandomAccessIterator last)
{
	__make_heap(first, last, value_type(first), distance_type(first));
}

template<class RandomAccessIterator, class Compare, 
	class T, class Distance>
void __make_heap(RandomAccessIterator first,
	RandomAccessIterator last, Compare comp, T*, Distance*)
{
	if (last - first < 2)return; // �������Ϊ0��1��������������
	Distance len = last - first;
	Distance parent = (len - 2) / 2;
	while (true)
	{
		__adjust_heap(first, parent, len, T(*(first + parent)), comp);
		if (parent == 0)return;
		parent--;
	}
}

template<class RandomAccessIterator, class Compare>
inline void make_heap(RandomAccessIterator first, 
	RandomAccessIterator last, Compare comp)
{
	__make_heap(first, last, comp,
		value_type(first), distance_type(first));
}

template<class RandomAccessIterator>
void sort_heap(RandomAccessIterator first, 
	RandomAccessIterator last)
{
	while (last - first > 1)
		pop_heap(first, last--);
}

template<class RandomAccessIterator, class Compare>
void sort_heap(RandomAccessIterator first,
	RandomAccessIterator last, Compare comp)
{
	while (last - first > 1)
		pop_heap(first, last--, comp);
}
#endif
