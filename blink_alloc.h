#ifndef __BLINK_ALLOC_H
#define __BLINK_ALLOC_H

// simple_alloc是一个工具类
template<class T, class Alloc>
class simple_alloc
{
public:
	static T* allocate(size_t n)
	{
		return 0 == n ? 0 : (T*)Alloc::allocate(n * sizeof(T));
	}

	static T* allocate(void)
	{
		return (T*)Alloc::allocate(sizeof(T));
	}

	static void deallocate(T* p, size_t n)
	{
		if (n != 0) Alloc::deallocate(p, n * sizeof(T));
	}

	static void deallocate(T* p)
	{
		Alloc::deallocate(p, sizeof(T));
	}
};

#if 0
#	include<new>
#	define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC)
#	include<iostream>
#	define __THROW_BAD_ALLOC std::cerr << "out of memory" << std::endl; exit(1);
#endif


// 第一级配置器
class __malloc_alloc_template
{
private:
	// 处理out of memory
	static void* oom_malloc(size_t);
	static void* oom_realloc(void*, size_t);
	static void(*__malloc_alloc_oom_handler)();

public:
	static void* allocate(size_t n)
	{
		void* result = malloc(n);	//第一级配置器直接使用malloc()
		// 当malloc失败时，尝试调用oom_malloc()
		if (result == 0)result = oom_malloc(n);
		return result;
	}

	static void deallocate(void* p, size_t /* n */)
	{
		free(p);	//第一级配置器直接使用free()
	}

	static void* reallocate(void* p, size_t /* old_sz */, size_t new_sz)
	{
		void* result = realloc(p, new_sz);	//第一级配置器直接使用 realloc()
		// 当 realloc 失败时，尝试调用oom_realloc()
		if (result == 0) result = oom_realloc(p, new_sz);
		return result;
	}

	static void (*set_malloc_handler(void (*f)()))()
	{
		void (*old)() = __malloc_alloc_oom_handler;
		__malloc_alloc_oom_handler = f;
		return (old);
	}

};

typedef __malloc_alloc_template malloc_alloc;

void (*__malloc_alloc_template::__malloc_alloc_oom_handler)() = 0;

void* __malloc_alloc_template::oom_malloc(size_t n)
{
	void (*my_malloc_handler)();
	void* result;

	for (;;)
	{// 不断尝试申请内存
		my_malloc_handler = __malloc_alloc_oom_handler;
		// 如果没有设置处理例程，则抛出异常
		if (my_malloc_handler == 0) { __THROW_BAD_ALLOC; }
		(*my_malloc_handler)();	// 调用处理例程，释放内存
		result = malloc(n);	//再次尝试申请内存
		if (result)return result;
	}
}

// alloc 是一个工具类
void* __malloc_alloc_template::oom_realloc(void* p, size_t n)
{
	void (*my_malloc_handler)();
	void* result;

	for (;;)
	{// 不断尝试申请内存
		my_malloc_handler = __malloc_alloc_oom_handler;
		// 如果没有设置处理例程，则抛出异常
		if (my_malloc_handler == 0) { __THROW_BAD_ALLOC; }
		(*my_malloc_handler)();	// 调用处理例程，释放内存
		result = realloc(p, n);	//再次尝试申请内存
		if (result) return result;
	}
}

enum { __ALIGN = 8 };	// 小型区块的上调边界
enum { __MAX_BYTES = 128 };	// 小型区块的上限
enum { __NFREELISTS = __MAX_BYTES / __ALIGN };	//free-lists 的个数

class __default_alloc_template
{
private:
	static size_t ROUND_UP(size_t bytes)
	{	// 将 bytes 上调至 8 的倍数
		// 上调之后的 (bytes + __ALIGN - 1) 与上 (__ALIGN - 1) 将 (bytes + __ALIGN - 1)二进制的后三位清零
		return (bytes + __ALIGN - 1) & ~(__ALIGN - 1);
	}

private:
	union obj
	{	/*、
		* 一物二用
		* obj 实际上保存的是一个内存块地址，指针不正是用来保存地址的吗
		* 而这个地址从不同的角度看起来，却给人不同的感觉
		* 从 free_list_link 看，obj 可以被视为一个指针，指向相同形式的另一个 obj
		* 从 client_data 看，obj 可以被视为一个指针，指向实际内存块
		*/

		union obj* free_list_link;
		char client_data[1];
	};

private:
	// free-lists
	/*
	* free_list[__NFREELISTS] 指向了每个 free-list 的第一个内存块
	*/
	static obj* volatile free_list[__NFREELISTS];
	// 根据内存区块大小，决定使用第n号free-list。n从0算起
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (bytes + __ALIGN - 1) / __ALIGN - 1;
	}

	// 返回一个大小为 n 的对象，并可能加入大小为 n 的其他区块到free-list
	static void* refill(size_t n);
	// 配置一大块空间，可容纳nobjs个大小为size的区块
	// 如果当前堆中空间不满足申请nobjs个size大小的区块，nobjs可能会下降
	static char* chunk_alloc(size_t size, int& nobjs);

	static char* start_free;	// 内存池起始位置，只在chunk_alloc()中变化
	static char* end_free;		// 内存池结束位置，只在chunk_alloc()中变化
	static size_t heap_size;

public:
	static void* allocate(size_t n)
	{
		obj* volatile* my_free_list;
		obj* result;

		// 当申请的空间大于128时调用第一级配置器
		if (n > (size_t)__MAX_BYTES)
			return malloc_alloc::allocate(n);
		// obj* volatile * my_free_list = free_list + FREELIST_INDEX(n)可以使 my_free_list 指向一个指向目标free-list的指针
		// 有点晕？其实是这样的
		// free_list 是一个指针数组，free_list + FREELIST_INDEX(n)实际是free_list得到的实际是存放free_list元素的地址
		// 所以my_free_list 实际是一个指针的指针
		// 当然为了简单也可以写成 obj * my_free_list = free_list[FREELIST_INDEX(n)]
		// 但是在调整free-list时，就得写成 free_list[FREELIST_INDEX(N)] = result->free_list_link
		// 二维指针的写法似乎效率更高，看上去也更加美丽
		my_free_list = free_list + FREELIST_INDEX(n);
		result = *my_free_list; // 取内存块的地址
		if (result == 0)
		{
			// free-list 没有可用区块时，重新填充free-list
			void* r = refill(ROUND_UP(n));
			return r;
		}
		// 调整free-list
		*my_free_list = result->free_list_link;
		return result;
	}

	static void deallocate(void* p, size_t n)
	{
		obj* q = (obj*)p;
		obj* volatile* my_free_list;

		// 当待销毁的内存块大于128时调用第一级配置器
		if (n > (size_t)__MAX_BYTES)
		{
			malloc_alloc::deallocate(p, n);
			return;
		}
		// 寻找相应的free-list
		my_free_list = free_list + FREELIST_INDEX(n);
		// 调整free-list回收区块
		q->free_list_link = *my_free_list;
		*my_free_list = q;
	}

	static void* reallocate(void* p, size_t old_sz, size_t new_sz);
};


char* __default_alloc_template::start_free = 0;
char* __default_alloc_template::end_free = 0;
size_t __default_alloc_template::heap_size = 0;
__default_alloc_template::obj* volatile
__default_alloc_template::free_list[__NFREELISTS] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
typedef __default_alloc_template alloc;

void* __default_alloc_template::refill(size_t n)
{
	int nobjs = 20;
	// 调用chunk_alloc()尝试申请nobjs个区块作为free-list的新节点
	// nobjs 是 pass by reference
	char* chunk = chunk_alloc(n, nobjs);
	obj* volatile* my_free_list;

	obj* result;
	obj* current_obj, * next_obj;
	int i;

	// 如果只获得一个区块，直接将这个区块返回给申请者
	if (nobjs == 1)return chunk;

	// 准备调整free-list，纳入新节点
	my_free_list = free_list + FREELIST_INDEX(n);

	result = (obj*)chunk; //第一块返回给申请者
	*my_free_list = next_obj = (obj*)(chunk + n);
	// 连接各节点
	for (i = 1;; ++i)
	{
		current_obj = next_obj;
		next_obj = (obj*)((char*)next_obj + n);
		if (nobjs - 1 == i)
		{
			current_obj->free_list_link = 0;
			break;
		}
		else
		{
			current_obj->free_list_link = next_obj;
		}
	}
	return result;
}

char* __default_alloc_template::chunk_alloc(size_t size, int& nobjs)
{	// 在内 存池中为free-list分配空间
	// size 为__ALIGN的倍数
	// nobjs 是 pass by reference
	char* result;
	size_t total_bytes = size * nobjs;
	size_t bytes_left = end_free - start_free;

	if (bytes_left >= total_bytes)
	{	// 内存池剩余空间完全满足free-list的申请
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else if (bytes_left >= size)
	{	// 内存池剩余空间不能完全满足free-list的申请，但足够供应一个及以上的区块
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else
	{
		// 内存池内空间不足以满足一个区块的申请
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		if (bytes_left > 0)
		{	// 将内存池内剩余的空间分配给适当的free-list
			obj* volatile* my_free_list = free_list + FREELIST_INDEX(bytes_left);
			((obj*)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj*)start_free;
		}
		// 在heap中申请内存空间来填充内存池
		start_free = (char*)malloc(bytes_to_get);
		if (start_free == 0)
		{	// heap中内存空间不足
			obj* volatile* my_free_list, * p;
			for (int i = size; i <= __MAX_BYTES; i += __ALIGN)
			{	// 检索区块空闲区块且区块大小大于 size 的 free-list
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (p != 0)
				{
					*my_free_list = p->free_list_link;
					start_free = (char*)p;
					end_free = start_free + i;
					// 此处递归调用chunk_alloc，可以减少不必要代码的编写，同时修正 nobjs
					// 超级帅
					return chunk_alloc(size, nobjs);
				}
			}
			// 山穷水尽，到处都没有内存可以使用了
			// 调用第一级配置器，第一级配置器有 out of memory 机制，在内存不足时可以抛出异常
			end_free = 0;
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);
		}
		// 调整 heap_size 大小，放在此处是极好的，耐人寻味
		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		return chunk_alloc(size, nobjs);
	}
}
#endif // !__BLINK_ALLOC_H

