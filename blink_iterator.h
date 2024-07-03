#ifndef __BLINK_ITERATOR_H
#define __BLINK_ITERATOR_H

#include<cstddef>

// ���ֵ���������
/*
* Input Iterator����������ָ�Ķ���ֻ�������ڴ����ݽṹ�ж�ȡ����(ie: a = *iter, iter == iter)
* Output Iterator����������ָ�Ķ���ֻд�����������ݽṹ��д������(ie: *iter = value)
* �����Input��ָ����������������Ϊ���������Դ��Output��ָ����������Ϊ�����Ŀ�ĵ�
* Forward Iterator������д���͡��㷨�ڴ��ֵ��������γ������Ͻ��ж�д����
* Bidirectional Iterator����˫���ƶ��ĵ�����
* Random Access Iterator����������ʵĵ�����
* ���ο�Input Iterator��Output Iteratorʱ��ΪΪʲôҪ��ְ�𻮷ֵ��������
* �����������Լ��̼��ˣ�ְ��Խ����ֻ���Ǹ��õ�ѡ��
* ÿ��������ֻ����Լ��صĹ������Ҳ�Ҫ��ͼԽ�磬���Ա������ʹ��
*/

struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

template<class Category, class T, class Distance = ptrdiff_t,
	class Pointer = T*, class Reference = T&>
struct iterator
{
	typedef Category	iterator_category;
	typedef T			value_type;
	typedef Distance	difference_type;
	typedef Pointer		pointer;
	typedef Reference	reference;
};

/*
* iterator_category��
* value_type����ʾ������ ��ָ���� ������
* difference_type����ʾ����������֮��ľ���
* pointer�����ص�������ָ֮���ַʱʹ��pointer
* reference��ȡָ����ָ��Ԫ��ֵ *p ʱʹ��pass by reference
*/
template<class Iterator>
struct iterator_traits
{
	typedef typename Iterator::iterator_category	iterator_category;
	typedef typename Iterator::value_type			value_type;
	typedef typename Iterator::difference_type		difference_type;
	typedef typename Iterator::pointer				pointer;
	typedef typename Iterator::reference			reference;
};

/*
* ���������еĵ��������� class type��ԭ��ָ��Ͳ��ǣ��������class type�����޷�Ϊ��������Ƕ����
* ΪʲôҪ����ԭ��ָ�룬��������һ����Ϊ����ָ��Ķ������ԭ��ָ�뵱ȻҲӦ�ÿ�����Ϊ������
*/
// ���ԭ��ָ��(native pointer) ����Ƶ�traits�ر��
template<class T>
struct iterator_traits<T*>
{
	typedef random_access_iterator_tag	iterator_category;
	typedef T							value_type;
	typedef ptrdiff_t					difference_type;
	typedef T* pointer;
	typedef T& reference;
};

// ���ԭ���� pointer-to-const ����Ƶ�traits�ر��
/*
* ���� pointer-to-const��iterator tracts�������࿼��
* 1. value_type���ص���T����const T
*	value_type Ӧ��ʾ��������ָ���Ԫ�ص�ԭʼ���ͣ����������Ԫ�صķ���Ȩ��(const)
* 2. pointer �� reference���ص�������const
*	pointer-to-const ��ʾָ���Ԫ�ز���ͨ�����ָ�뱻�޸�
*	����ڷ�������ʱҲҪȷ��Ԫ�ز��ᱻ����ָ������������޸�
*/
template<class T>
struct iterator_traits<const T*>
{
	typedef random_access_iterator_tag	iterator_category;
	typedef T							value_type;
	typedef ptrdiff_t					difference_type;
	typedef const T* pointer;
	typedef const T& reference;
};

// ��õ���������
// iterator_traits���Ƶ�Ŀ���ǵõ�һ��������iterator��ͬ����ʱ����
// ��Щ���ǿն���
template<class Iterator>
inline typename iterator_traits<Iterator>::iterator_category
iterator_category(const Iterator&)
{
	typedef typename iterator_traits<Iterator>::iterator_category category;
	return category();
}

template<class Iterator>
inline typename iterator_traits<Iterator>::difference_type*
distance_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::difference_type*>(0);
}

template<class Iterator>
inline typename iterator_traits<Iterator>::value_type*
value_type(const Iterator&)
{
	return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
}

template<class InputIterator, class Distance>
inline void __distance(InputIterator first, InputIterator last, Distance& n, input_iterator_tag)
{
	while (first != last) { ++first; ++n; }
}

template<class RandomAccessIterator, class Distance>
inline void __distance(RandomAccessIterator first, 
	RandomAccessIterator last, 
	Distance& n, random_access_iterator_tag)
{
	n += (last - first);
}

template<class InputIterator, class Distance>
inline void distance(InputIterator first, InputIterator last, Distance& n)
{
	__distance(first, last, n, iterator_category(first));
}

template<class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
__distance(InputIterator first, InputIterator last, input_iterator_tag)
{
	typename iterator_traits<InputIterator>::difference_type n = 0;
	while (first != last)
	{
		first++;
		n++;
	}
	return n;
}

template<class RandomAccessIterator>
inline typename iterator_traits<RandomAccessIterator>::difference_type
__distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
{
	return last - first;
}


template<class InputIterator>
inline typename iterator_traits<InputIterator>::difference_type
distance(InputIterator first, InputIterator last)
{
	typedef typename iterator_traits<InputIterator>::iterator_category category;
	return __distance(first, last, category());
}

template<class InputIterator, class Distance>
inline void __advance(InputIterator& i, Distance n, input_iterator_tag)
{
	while (n--)++i;
}

template<class BidirectionalIterator, class Distance>
inline void __advance(BidirectionalIterator& i, Distance n, bidirectional_iterator_tag)
{
	if (n > 0)
		while (n--)i++;
	else
		while (n--)i--;
}

template<class RandomAccessIterator, class Distance>
inline void __advance(RandomAccessIterator& i, Distance n, random_access_iterator_tag)
{
	i += n;
}

template<class InputIterator, class Distance>
inline void advance(InputIterator& i, Distance n)
{
	typedef typename iterator_traits<InputIterator>::iterator_category category;
	return __advance(i, n, category());
}

#endif // !BLINK_ITERATOR

