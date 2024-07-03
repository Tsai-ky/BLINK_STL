#ifndef __BLINK_CONSTRUCT_H
#define __BLINK_CONSTRUCT_H
#include<new.h>
#include"type_traits.h"
#include"blink_iterator.h"

template<class T1, class T2>
inline void construct(T1* p, const T2& value)
{
	// 在指针 p 所指的内存空间构造对象T1
	new (p) T1(value);
}

// 第一版 destroy()，直接调用类T的析构函数
template<class T>
inline void destroy(T* pointer)
{
	pointer->~T();
}

// 利用 __type_traits<> 判断元素的数值类型是否有默认的析构函数
// 如果是默认的析构函数，则成为 trivial_destructor，因为什么都不做，因此被称为无关紧要的
// 如果析构函数是重构的，那说明在析构对象前需要有其他的操作
template<class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*)
{
	typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
	__destroy_aux(first, last, trivial_destructor());
}

// 如果元素的数值类型有 non-trivial destructor
template<class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __false_type)
{
	// 对于具有non-trivial destructor的元素，老老实实调用第一版destroy()
	for (; first < last; ++first)
		destroy(&*first);
}

// 如果元素的类型有 trivial destructor，则忽略
// 当给定的元素范围很广时，能够降低开销
template<class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __true_type) {}


// 第二版destroy()， 接受两个迭代器
// 根据元素类型，使用适当的措施析构对象
template<class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
	__destroy(first, last, value_type(first));
}


// 针对元素类型为char* 和 wchar_t* 的特定destroy()
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}
#endif // !BLINK_CONSTRUCT



