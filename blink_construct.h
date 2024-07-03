#ifndef __BLINK_CONSTRUCT_H
#define __BLINK_CONSTRUCT_H
#include<new.h>
#include"type_traits.h"
#include"blink_iterator.h"

template<class T1, class T2>
inline void construct(T1* p, const T2& value)
{
	// ��ָ�� p ��ָ���ڴ�ռ乹�����T1
	new (p) T1(value);
}

// ��һ�� destroy()��ֱ�ӵ�����T����������
template<class T>
inline void destroy(T* pointer)
{
	pointer->~T();
}

// ���� __type_traits<> �ж�Ԫ�ص���ֵ�����Ƿ���Ĭ�ϵ���������
// �����Ĭ�ϵ��������������Ϊ trivial_destructor����Ϊʲô����������˱���Ϊ�޹ؽ�Ҫ��
// ��������������ع��ģ���˵������������ǰ��Ҫ�������Ĳ���
template<class ForwardIterator, class T>
inline void __destroy(ForwardIterator first, ForwardIterator last, T*)
{
	typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
	__destroy_aux(first, last, trivial_destructor());
}

// ���Ԫ�ص���ֵ������ non-trivial destructor
template<class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __false_type)
{
	// ���ھ���non-trivial destructor��Ԫ�أ�����ʵʵ���õ�һ��destroy()
	for (; first < last; ++first)
		destroy(&*first);
}

// ���Ԫ�ص������� trivial destructor�������
// ��������Ԫ�ط�Χ�ܹ�ʱ���ܹ����Ϳ���
template<class ForwardIterator>
inline void __destroy_aux(ForwardIterator first, ForwardIterator last, __true_type) {}


// �ڶ���destroy()�� ��������������
// ����Ԫ�����ͣ�ʹ���ʵ��Ĵ�ʩ��������
template<class ForwardIterator>
inline void destroy(ForwardIterator first, ForwardIterator last)
{
	__destroy(first, last, value_type(first));
}


// ���Ԫ������Ϊchar* �� wchar_t* ���ض�destroy()
inline void destroy(char*, char*) {}
inline void destroy(wchar_t*, wchar_t*) {}
#endif // !BLINK_CONSTRUCT



