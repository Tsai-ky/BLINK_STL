#ifndef __BLINK_ITERATOR_H
#define __BLINK_ITERATOR_H

#include<cstddef>

// 物种迭代器类型
/*
* Input Iterator：迭代器所指的对象只读，用于从数据结构中读取数据(ie: a = *iter, iter == iter)
* Output Iterator：迭代器所指的对象只写，用于向数据结构中写入数据(ie: *iter = value)
* 这里的Input是指将迭代器的内容作为输入的数据源，Output是指将迭代器作为输出的目的地
* Forward Iterator：允许“写入型”算法在此种迭代器所形成区间上进行读写操作
* Bidirectional Iterator：可双向移动的迭代器
* Random Access Iterator：可随机访问的迭代器
* 初次看Input Iterator和Output Iterator时认为为什么要将职责划分得如此清晰
* 后来觉得是自己短见了，职责越清晰只会是更好的选择
* 每个部件都只完成自己地的工作并且不要企图越界，可以避免错误使用
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
* iterator_category：
* value_type：表示迭代器 所指对象 的类型
* difference_type：表示两个迭代器之间的距离
* pointer：返回迭代器所指之物地址时使用pointer
* reference：取指针所指的元素值 *p 时使用pass by reference
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
* 并不是所有的迭代器都是 class type，原生指针就不是，如果不是class type，就无法为它定义内嵌类型
* 为什么要考虑原生指针，迭代器是一种行为类似指针的对象，因此原生指针当然也应该可以作为迭代器
*/
// 针对原生指针(native pointer) 而设计的traits特别版
template<class T>
struct iterator_traits<T*>
{
	typedef random_access_iterator_tag	iterator_category;
	typedef T							value_type;
	typedef ptrdiff_t					difference_type;
	typedef T* pointer;
	typedef T& reference;
};

// 针对原生的 pointer-to-const 而设计的traits特别版
/*
* 对于 pointer-to-const的iterator tracts设计有诸多考虑
* 1. value_type返回的是T而非const T
*	value_type 应表示迭代器所指向的元素的原始类型，而不是这个元素的访问权限(const)
* 2. pointer 和 reference返回的类型是const
*	pointer-to-const 表示指向的元素不能通过这个指针被修改
*	因此在返回类型时也要确保元素不会被其他指针或引用意外修改
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

// 获得迭代器类型
// iterator_traits机制的目的是得到一个类型与iterator相同的暂时对象！
// 这些都是空对象
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

