/*
* һ���򵥵�allocator
* ʵ���˶�new��delete�ļ򵥷�װ���Լ��Թ��캯��������������ֱ�ӵ���
*/

#ifndef __BLINK_ALLOCATOR_H
#define __BLINK_ALLOCATOR_H

#include<new>       // for placement new
#include<cstddef>   // for ptrdiff_t, size_t
#include<cstdlib>   // for exit()
#include<climits>   // for UINT_MAX
#include<iostream>  // for cerr

using namespace std;

namespace blink
{
    template <class T>
    inline T* _allocate(ptrdiff_t size, T*)
    {
        /*
        * set_new_handler(new handler new_p)noexcept;
        * ����new_pָ��ĺ���Ϊnew������new[]����ʧ��ʱ���õĴ�����
        */
        set_new_handler(0);
        /*
        * ʹ��ȫ�ֵ�operator new(size_t size)�����ڴ�
        * ϵͳĬ�ϵ�ȫ��::operator new(size_t size)ֻ�ǵ���malloc�����ڴ�
        */
        T* tmp = (T*)(::operator new((size_t)(size * sizeof(T))));
        if (tmp == 0)
        {
            cerr << "out of memory" << endl;    // �����׼����
            exit(1);
        }
        return tmp;
    }

    template<class T>
    inline void _deallocate(T* buffer)
    {
        ::operator delete(buffer);
    }

    template<class T1, class T2>
    inline void _construct(T1* p, const T2& value)
    {
        /*
        * p ָ��һ���ѷ�����ض��ڴ�
        * �� value ��Ϊ������ p ��ָ����ڴ�Ŀռ乹��T1�����
        */
        new(p) T1(value); 
    }

    template<class T>
    inline void _destroy(T* ptr)
    {
        ptr -> ~T();
    }

    template<class T>
    class allocator
    {
    public:
        typedef T           value_type;
        typedef T*          pointer;
        typedef const T*    const_pointer;
        typedef T&          reference;
        typedef const T&    const_reference;
        typedef size_t      size_type;
        typedef ptrdiff_t   difference_type;

        // ���캯��
        allocator() {}
        template <class U>
        allocator(const allocator<U>& c){}

        template<class U>
        struct rebind
        {
            typedef allocator<U> other;
        };

        pointer allocate(size_type n, const void* hint = 0)
        {
            /*
            * hint �����Ż��ڴ�����һ����ѡ������
            * ������ҪĿ���Ǹ��������ṩһ����ʾ�������·�����ڴ�������λ�á�
            * ���������£����ʹ���� hint�����������Գ��Խ��µ��ڴ������������ڴ��п��� hint ָ����λ�á�
            * ����������߾ֲ���ԭ��locality of reference�����Ӷ�������߻����Ч�ʺ��������ܡ�
            */
            return _allocate((difference_type)n, (pointer)0);
        }

        void deallocate(pointer p, size_type n) { _deallocate(p); }

        void construct(pointer p, const_reference value)
        {
            _construct(p, value);
        }

        void destroy(pointer p) { _destroy(p); }

        pointer address(reference x) { return (pointer)&x; }

        const_pointer const_address(const_reference x) { return (const_pointer)&x; }

        size_type max_size() const { return size_type(UINT_MAX / sizeof(T)); }
    };
}
#endif 