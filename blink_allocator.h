/*
* 一个简单的allocator
* 实现了对new，delete的简单封装，以及对构造函数和析构函数的直接调用
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
        * 设置new_p指向的函数为new操作或new[]操作失败时调用的处理函数
        */
        set_new_handler(0);
        /*
        * 使用全局的operator new(size_t size)申请内存
        * 系统默认的全局::operator new(size_t size)只是调用malloc分配内存
        */
        T* tmp = (T*)(::operator new((size_t)(size * sizeof(T))));
        if (tmp == 0)
        {
            cerr << "out of memory" << endl;    // 输出标准错误
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
        * p 指向一个已分配的特定内存
        * 以 value 作为输入在 p 所指向的内存的空间构造T1类对象
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

        // 构造函数
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
            * hint 用于优化内存分配的一个可选参数。
            * 它的主要目的是给分配器提供一个提示，关于新分配的内存块的理想位置。
            * 理想的情况下，如果使用了 hint，分配器可以尝试将新的内存块分配在物理内存中靠近 hint 指定的位置。
            * 这有助于提高局部性原理（locality of reference），从而可以提高缓存的效率和整体性能。
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