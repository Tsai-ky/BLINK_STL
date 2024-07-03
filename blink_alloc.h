#ifndef __BLINK_ALLOC_H
#define __BLINK_ALLOC_H

// simple_alloc��һ��������
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


// ��һ��������
class __malloc_alloc_template
{
private:
	// ����out of memory
	static void* oom_malloc(size_t);
	static void* oom_realloc(void*, size_t);
	static void(*__malloc_alloc_oom_handler)();

public:
	static void* allocate(size_t n)
	{
		void* result = malloc(n);	//��һ��������ֱ��ʹ��malloc()
		// ��mallocʧ��ʱ�����Ե���oom_malloc()
		if (result == 0)result = oom_malloc(n);
		return result;
	}

	static void deallocate(void* p, size_t /* n */)
	{
		free(p);	//��һ��������ֱ��ʹ��free()
	}

	static void* reallocate(void* p, size_t /* old_sz */, size_t new_sz)
	{
		void* result = realloc(p, new_sz);	//��һ��������ֱ��ʹ�� realloc()
		// �� realloc ʧ��ʱ�����Ե���oom_realloc()
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
	{// ���ϳ��������ڴ�
		my_malloc_handler = __malloc_alloc_oom_handler;
		// ���û�����ô������̣����׳��쳣
		if (my_malloc_handler == 0) { __THROW_BAD_ALLOC; }
		(*my_malloc_handler)();	// ���ô������̣��ͷ��ڴ�
		result = malloc(n);	//�ٴγ��������ڴ�
		if (result)return result;
	}
}

// alloc ��һ��������
void* __malloc_alloc_template::oom_realloc(void* p, size_t n)
{
	void (*my_malloc_handler)();
	void* result;

	for (;;)
	{// ���ϳ��������ڴ�
		my_malloc_handler = __malloc_alloc_oom_handler;
		// ���û�����ô������̣����׳��쳣
		if (my_malloc_handler == 0) { __THROW_BAD_ALLOC; }
		(*my_malloc_handler)();	// ���ô������̣��ͷ��ڴ�
		result = realloc(p, n);	//�ٴγ��������ڴ�
		if (result) return result;
	}
}

enum { __ALIGN = 8 };	// С��������ϵ��߽�
enum { __MAX_BYTES = 128 };	// С�����������
enum { __NFREELISTS = __MAX_BYTES / __ALIGN };	//free-lists �ĸ���

class __default_alloc_template
{
private:
	static size_t ROUND_UP(size_t bytes)
	{	// �� bytes �ϵ��� 8 �ı���
		// �ϵ�֮��� (bytes + __ALIGN - 1) ���� (__ALIGN - 1) �� (bytes + __ALIGN - 1)�����Ƶĺ���λ����
		return (bytes + __ALIGN - 1) & ~(__ALIGN - 1);
	}

private:
	union obj
	{	/*��
		* һ�����
		* obj ʵ���ϱ������һ���ڴ���ַ��ָ�벻�������������ַ����
		* �������ַ�Ӳ�ͬ�ĽǶȿ�������ȴ���˲�ͬ�ĸо�
		* �� free_list_link ����obj ���Ա���Ϊһ��ָ�룬ָ����ͬ��ʽ����һ�� obj
		* �� client_data ����obj ���Ա���Ϊһ��ָ�룬ָ��ʵ���ڴ��
		*/

		union obj* free_list_link;
		char client_data[1];
	};

private:
	// free-lists
	/*
	* free_list[__NFREELISTS] ָ����ÿ�� free-list �ĵ�һ���ڴ��
	*/
	static obj* volatile free_list[__NFREELISTS];
	// �����ڴ������С������ʹ�õ�n��free-list��n��0����
	static size_t FREELIST_INDEX(size_t bytes)
	{
		return (bytes + __ALIGN - 1) / __ALIGN - 1;
	}

	// ����һ����СΪ n �Ķ��󣬲����ܼ����СΪ n ���������鵽free-list
	static void* refill(size_t n);
	// ����һ���ռ䣬������nobjs����СΪsize������
	// �����ǰ���пռ䲻��������nobjs��size��С�����飬nobjs���ܻ��½�
	static char* chunk_alloc(size_t size, int& nobjs);

	static char* start_free;	// �ڴ����ʼλ�ã�ֻ��chunk_alloc()�б仯
	static char* end_free;		// �ڴ�ؽ���λ�ã�ֻ��chunk_alloc()�б仯
	static size_t heap_size;

public:
	static void* allocate(size_t n)
	{
		obj* volatile* my_free_list;
		obj* result;

		// ������Ŀռ����128ʱ���õ�һ��������
		if (n > (size_t)__MAX_BYTES)
			return malloc_alloc::allocate(n);
		// obj* volatile * my_free_list = free_list + FREELIST_INDEX(n)����ʹ my_free_list ָ��һ��ָ��Ŀ��free-list��ָ��
		// �е��Σ���ʵ��������
		// free_list ��һ��ָ�����飬free_list + FREELIST_INDEX(n)ʵ����free_list�õ���ʵ���Ǵ��free_listԪ�صĵ�ַ
		// ����my_free_list ʵ����һ��ָ���ָ��
		// ��ȻΪ�˼�Ҳ����д�� obj * my_free_list = free_list[FREELIST_INDEX(n)]
		// �����ڵ���free-listʱ���͵�д�� free_list[FREELIST_INDEX(N)] = result->free_list_link
		// ��άָ���д���ƺ�Ч�ʸ��ߣ�����ȥҲ��������
		my_free_list = free_list + FREELIST_INDEX(n);
		result = *my_free_list; // ȡ�ڴ��ĵ�ַ
		if (result == 0)
		{
			// free-list û�п�������ʱ���������free-list
			void* r = refill(ROUND_UP(n));
			return r;
		}
		// ����free-list
		*my_free_list = result->free_list_link;
		return result;
	}

	static void deallocate(void* p, size_t n)
	{
		obj* q = (obj*)p;
		obj* volatile* my_free_list;

		// �������ٵ��ڴ�����128ʱ���õ�һ��������
		if (n > (size_t)__MAX_BYTES)
		{
			malloc_alloc::deallocate(p, n);
			return;
		}
		// Ѱ����Ӧ��free-list
		my_free_list = free_list + FREELIST_INDEX(n);
		// ����free-list��������
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
	// ����chunk_alloc()��������nobjs��������Ϊfree-list���½ڵ�
	// nobjs �� pass by reference
	char* chunk = chunk_alloc(n, nobjs);
	obj* volatile* my_free_list;

	obj* result;
	obj* current_obj, * next_obj;
	int i;

	// ���ֻ���һ�����飬ֱ�ӽ�������鷵�ظ�������
	if (nobjs == 1)return chunk;

	// ׼������free-list�������½ڵ�
	my_free_list = free_list + FREELIST_INDEX(n);

	result = (obj*)chunk; //��һ�鷵�ظ�������
	*my_free_list = next_obj = (obj*)(chunk + n);
	// ���Ӹ��ڵ�
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
{	// ���� �����Ϊfree-list����ռ�
	// size Ϊ__ALIGN�ı���
	// nobjs �� pass by reference
	char* result;
	size_t total_bytes = size * nobjs;
	size_t bytes_left = end_free - start_free;

	if (bytes_left >= total_bytes)
	{	// �ڴ��ʣ��ռ���ȫ����free-list������
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else if (bytes_left >= size)
	{	// �ڴ��ʣ��ռ䲻����ȫ����free-list�����룬���㹻��Ӧһ�������ϵ�����
		nobjs = bytes_left / size;
		total_bytes = size * nobjs;
		result = start_free;
		start_free += total_bytes;
		return result;
	}
	else
	{
		// �ڴ���ڿռ䲻��������һ�����������
		size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
		if (bytes_left > 0)
		{	// ���ڴ����ʣ��Ŀռ������ʵ���free-list
			obj* volatile* my_free_list = free_list + FREELIST_INDEX(bytes_left);
			((obj*)start_free)->free_list_link = *my_free_list;
			*my_free_list = (obj*)start_free;
		}
		// ��heap�������ڴ�ռ�������ڴ��
		start_free = (char*)malloc(bytes_to_get);
		if (start_free == 0)
		{	// heap���ڴ�ռ䲻��
			obj* volatile* my_free_list, * p;
			for (int i = size; i <= __MAX_BYTES; i += __ALIGN)
			{	// ����������������������С���� size �� free-list
				my_free_list = free_list + FREELIST_INDEX(i);
				p = *my_free_list;
				if (p != 0)
				{
					*my_free_list = p->free_list_link;
					start_free = (char*)p;
					end_free = start_free + i;
					// �˴��ݹ����chunk_alloc�����Լ��ٲ���Ҫ����ı�д��ͬʱ���� nobjs
					// ����˧
					return chunk_alloc(size, nobjs);
				}
			}
			// ɽ��ˮ����������û���ڴ����ʹ����
			// ���õ�һ������������һ���������� out of memory ���ƣ����ڴ治��ʱ�����׳��쳣
			end_free = 0;
			start_free = (char*)malloc_alloc::allocate(bytes_to_get);
		}
		// ���� heap_size ��С�����ڴ˴��Ǽ��õģ�����Ѱζ
		heap_size += bytes_to_get;
		end_free = start_free + bytes_to_get;
		return chunk_alloc(size, nobjs);
	}
}
#endif // !__BLINK_ALLOC_H

