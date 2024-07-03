#ifndef __BLINK_DEQUE_H
#define __BLINK_DEQUE_H
#include"blink_alloc.h"
#include"blink_uninitialized.h"
#include"blink_algobase.h"

/*
* deque 允许指定缓冲区大小，默认值为0表示将使用512 bytes缓冲区
* 如果 n 为 0，则表示 buffer_size 使用默认值
*	若元素大小sz小于512，则buffer可以容纳 512 / sz
*	若元素大小sz大于512，则返回1
* 如果 n 不为 0，则表示用户自定义 buffer_size为 n
*/
inline size_t __deque_buf_size(size_t n, size_t sz)
{
	if (n == 0)
	{
		if (sz < 512)
			return size_t(512 / sz);
		else
			return size_t(1);
	}
	else
		return n;
}

template<class T, class Ref, class Ptr, size_t BufSize>
struct __deque_iterator
{
	typedef __deque_iterator<T, T&, T*, BufSize>	iterator;
	typedef __deque_iterator<T, const T&, const T*, BufSize> const_iterator;

	static size_t buffer_size() { return __deque_buf_size(BufSize, sizeof(T)); }

	typedef random_access_iterator_tag iterator_category;
	typedef Ptr pointer;
	typedef Ref reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T** map_pointer;

	typedef __deque_iterator self;

	T* cur;				// 当前迭代器所指缓冲区中的现行元素
	T* first;			// 当前迭代器所指缓冲区的头
	T* last;			// 当前迭代器所指缓冲区的尾
	map_pointer node;	// 指向 map 中指向当前缓冲区的node

	// 设置迭代器所指向的缓冲区
	void set_node(map_pointer new_node)
	{
		// new_node 是一个指向 map 元素 node 的指针
		// map 的元素都是指针，指向缓冲区
		node = new_node;
		first = *new_node;
		last = first + difference_type(buffer_size());
	}

	// 重载运算子
	reference operator * ()const { return *cur; }
	pointer operator -> () const { return &(operator * ()); }

	difference_type operator -(const self& x) const
	{
		return difference_type(buffer_size()) * (node - x.node - 1) +
			(cur - first) + (x.last - x.cur);
	}

	self& operator ++()	//++iter
	{
		++cur;	// 切换到下一个元素
		if (cur == last)	//如果已到达缓冲区末尾
		{
			set_node(node + 1);	// 切换至下一结点（缓冲区） 的第一个元素
			cur = first;
		}
		return *this;
	}

	self operator ++(int) // iter++;
	{
		self tmp = *this;
		++(*this);
		return tmp;
	}

	self& operator --() // -- iter;
	{
		if (cur == first)	// 如果当前已处于缓冲区首部
		{
			set_node(node - 1);	//切换至前一个结点的结尾
			cur = last;
		}
		--cur;	// 切换至前一个结点的最后一个结点
		return *this;
	}

	self operator --(int)	// iter --;
	{
		self tmp = *this;
		--(*this);
		return tmp;
	}

	self& operator+=(difference_type n)
	{
		difference_type offset = n + (cur - first);
		if (offset >= 0 && offset < difference_type(buffer_size()))
		{
			// 目标位置在同一缓冲区
			cur += n;
		}
		else
		{
			// 目标位置不在同一缓冲区
			difference_type node_offset;
			if (offset > 0)
				node_offset = offset / (buffer_size());
			else
				node_offset = -difference_type((-offset - 1) / buffer_size()) - 1;
			// 切换至目标缓冲区
			set_node(node + node_offset);
			// 切换至正确的元素
			cur = first + (offset - node_offset * difference_type(buffer_size()));
		}
		return *this;
	}

	self operator +(difference_type n)const
	{
		self tmp = *this;
		return tmp += n;
	}

	self& operator -=(difference_type n) { return *this += (-n); }
	self operator -(difference_type n) const
	{
		self tmp = *this;
		return tmp -= n;
	}

	reference operator[](difference_type n) const { return *(*this + n); }

	bool operator ==(const self& x)const { return cur == x.cur; }
	bool operator !=(const self& x)const { return !(*this == x); }
	bool operator < (const self& x)const
	{
		return (node == x.node) ? (cur < x.cur) : (node < x.node);
	}
};


template<class T, class Alloc = alloc, size_t BufSize = 0>
class blink_deque
{
public:
	typedef T	value_type;
	typedef T& reference;
	typedef T* pointer;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef const T& const_reference;
	typedef const T* const_pointer;

public:
	typedef __deque_iterator<T, T&, T*, BufSize> iterator;

protected:
	typedef pointer* map_pointer;
	typedef simple_alloc<value_type, Alloc>data_allocator;
	typedef simple_alloc<pointer, Alloc>map_allocator;

protected:
	// 队列头部地址 == 队列第一个元素地址
	// 队列尾部地址 == 队列最后一个元素地址（元素地址是元素首地址） + 1
	iterator start;	// 队列头部
	iterator finish;	//队列尾部（注意不是队列最后一个元素）

	// map 指针指向map，map是一个连续空间
	// map 的每一个元素都是一个指针（结点），每个指针指向一个缓冲区
	map_pointer map;
	size_type map_size; // map 大小
	enum { initial_map_size = 8 };

protected:
	void create_map_and_nodes(size_type numelements);
	void fill_initialize(size_type n, const value_type& value);
	void push_back_aux(const value_type& x);
	void push_front_aux(const value_type& x);
	void pop_back_aux();
	void pop_front_aux();
	iterator insert_aux(iterator pos, const value_type& x);
	pointer allocate_node() { return data_allocator::allocate(iterator::buffer_size()); }
	void deallocate_node(pointer p) { data_allocator::deallocate(p, iterator::buffer_size()); }
	void reallocate_map(size_type nodes_to_add, bool add_at_front);

	void reserve_map_at_back(size_type nodes_to_add = 1)
	{
		if (nodes_to_add + 1 > map_size - (finish.node - map))
			// 如果map尾端的节点备用空间不足时
			// 配置一个更大的map，拷贝原来的，释放原来的
			reallocate_map(nodes_to_add, false);
	}

	void reserve_map_at_front(size_type nodes_to_add = 1)
	{
		if (nodes_to_add > start.node - map)
			// 如果map头部的节点备用空间不足时
			// 配置一个更大的map，拷贝原来的，释放原来的
			reallocate_map(nodes_to_add, true);
	}

public:
	blink_deque() :start(), finish(), map(0), map_size(0) { create_map_and_nodes(0); }
	blink_deque(int n, const value_type& value)
		:start(), finish(), map(0), map_size(0)
	{
		fill_initialize(n, value);
	}
public:
	iterator begin() { return start; }
	iterator end() { return finish; }

	reference operator[](size_type n) { return start[difference_type(n)]; }

	reference front() { return *start; }
	reference back()
	{
		iterator tmp = finish;
		--tmp;
		return *tmp;
		// 此处不能写成return *(finish - 1);
		// 原因是 __deque_iterator<>没有为(finish - 1)定义运算子
	}

	size_type size()const { return finish - start; }
	size_type max_size()const { return size_type(-1); }
	bool empty()const { return finish == start; }

	void push_back(const value_type& x)
	{
		if (finish.cur != finish.last - 1)
		{
			construct(finish.cur, x);
			++finish.cur;
		}
		else
			push_back_aux(x);
	}

	void push_front(const value_type& x)
	{
		if (start.cur != start.first)
		{
			// 首部缓冲区仍有空闲空间
			construct(start.cur - 1, x);
			--start.cur;
		}
		else
			push_front_aux(x);
	}

	void pop_back()
	{
		if (finish.cur != finish.first)
		{
			--finish.cur;
			destroy(finish.cur);
		}
		else
		{
			pop_back_aux();
		}
	}

	void pop_front()
	{
		if (start.cur != start.last - 1)
		{
			destroy(start.cur);
			++start.cur;
		}
		else
			pop_front_aux();
	}

	void clear()
	{
		// clear()完成后回复初始状态
		// 初始状态下deque保留一个缓冲区
		for (map_pointer node = start.node + 1; node < finish.node; ++node)
		{
			destroy(*node, *node + iterator::buffer_size());
			data_allocator::deallocate(*node, iterator::buffer_size());
		}
		if (start.node != finish.node)
		{
			// 剩余两个缓冲区
			// 析构剩余元素
			destroy(start.cur, start.last);
			destroy(finish.first, finish.cur);
			// 释放尾部缓冲区
			data_allocator::deallocate(finish.first, iterator::buffer_size());

		}
		else
		{
			// 只有一个缓冲区
			// 析构剩余元素
			destroy(start.cur, finish.cur);
			// 注意初始状态下需要保留一个缓冲区
		}
	}

	// 清除单个元素
	iterator erase(iterator pos)
	{
		iterator next = pos;
		++next;
		difference_type index = pos - start;
		if (index < (size() >> 1))
		{	// 如果清除点之前的元素比较少
			// 将待移除元素拷贝到队列首部后移除
			copy_backward(start, pos, next);
			pop_back();
		}
		else
		{	// 如果清除点之后的元素比较少
			// 将待移除元素拷贝到队列尾部后移除
			copy(next, finish, pos);
			pop_back();
		}
		return start + index;
	}

	// 清除[first, last)区间内的所有元素
	iterator earse(iterator first, iterator last)
	{
		if (first == start && last == finish)
		{ // 清除区间是整个deque
			clear();
			return finish;
		}
		else
		{
			difference_type n = last - first;
			difference_type elems_before = first - start;
			if (elems_before < (size() - n) / 2)
			{
				// 清除区间前方的元素个数较少
				// 向后移动前方元素
				copy_backward(start, first, last);
				iterator new_start = start + n;
				destroy(start, new_start);
				for (map_pointer cur = start.node; cur < new_start.node; ++cur)
					data_allocator::deallocate(*cur, iterator::buffer_size());
				start = new_start;
			}
			else
			{
				// 清除区间后方的元素个数较少
				// 向前移动后方元素
				copy(last, finish, first);
				iterator new_finish = finish - n;
				destroy(new_finish, finish);
				for (map_pointer cur = new_finish.node + 1; cur <= finish.node; ++cur)
					data_allocator::destroy(*cur, iterator::buffer_size());
				finish = new_finish;
			}
			return start + elems_before;
		}
	}

	iterator insert(iterator position, const value_type& x)
	{
		if (position.cur == start.cur)
		{
			push_front(x);
			return start;
		}
		else if (position.cur == finish.cur)
		{
			push_back(x);
			iterator tmp = finish;
			--tmp;
			return tmp;
		}
		else
		{
			return insert_aux(position, x);
		}
	}
};

template<class T, class Alloc, size_t BufSize>
void blink_deque<T, Alloc, BufSize>::fill_initialize(size_type n, const value_type& value)
{
	create_map_and_nodes(n);
	map_pointer cur;
	// 为每个结点的缓冲区设定初值
	try
	{
		for (cur = start.node; cur < finish.node; ++cur)
			uninitialized_fill(*cur, *cur + iterator::buffer_size(), value);
		uninitialized_fill(finish.first, finish.cur, value);
	}
	catch (...)
	{

	}
}

template<class T, class Alloc, size_t BufSize>
void blink_deque<T, Alloc, BufSize>::create_map_and_nodes(size_type num_elements)
{
	// 需要节点数 = （元素个数 / 每个缓冲区可容纳的元素个数） + 1
	size_type num_nodes = num_elements / iterator::buffer_size() + 1;

	// map 管理节点数最少为8个，最多是所需节点数 + 2
	// 前后各预留一个备扩充时使用
	map_size = __BLINK::max(size_t(initial_map_size), num_nodes + 2);
	map = map_allocator::allocate(map_size);

	// 令 nstart 和 nfinish 指向 map 所拥有所有节点的最中央区段
	map_pointer nstart = map + (map_size - num_nodes) / 2;
	map_pointer nfinish = nstart + num_nodes - 1;

	map_pointer cur;

	try
	{
		// 为map内的每个节点配置缓冲区
		for (cur = nstart; cur <= nfinish; ++cur)
			*cur = allocate_node();
	}
	catch (...)
	{
		// commit or rollback
	}

	start.set_node(nstart);
	finish.set_node(nfinish);
	start.cur = start.first;
	finish.cur = finish.first + num_elements % iterator::buffer_size();
}

template<class T, class Alloc, size_t BufSize>
void blink_deque<T, Alloc, BufSize>::push_back_aux(const value_type& x)
{
	// 当最后一个缓冲区只剩下一个备用元素空间时才会被调用
	value_type x_copy = x;
	reserve_map_at_back(); // 必要时需要重新设置一个新的map
	*(finish.node + 1) = allocate_node(); // 配置一个新的节点
	try
	{
		construct(finish.cur, x_copy);
		finish.set_node(finish.node + 1);
		finish.cur = finish.first;
	}
	catch (...)
	{
		deallocate_node(*(finish.node + 1));
		throw;
	}
}

template<class T, class Alloc, size_t BufSize>
void blink_deque<T, Alloc, BufSize>::push_front_aux(const value_type& x)
{
	value_type x_copy = x;
	reserve_map_at_front();
	*(start.node - 1) = allocate_node();
	try
	{
		start.set_node(start.node - 1);
		start.cur = start.last - 1;
		construct(start.cur, x_copy);
	}
	catch (...)
	{
		start.set_node(start.node + 1);
		start.cur = start.first;
		deallocate_node(*(start.node - 1));
		throw;
	}
}

template<class T, class Alloc, size_t BufSize>
void blink_deque<T, Alloc, BufSize>::reallocate_map(size_type nodes_to_add, bool add_at_front)
{
	size_type old_num_nodes = finish.node - start.node + 1;
	size_type new_num_nodes = old_num_nodes + nodes_to_add;

	map_pointer new_nstart;
	if (map_size > 2 * new_num_nodes)
	{
		// 原来的map空间足够
		// 移动map中的节点
		new_nstart = map + (map_size - new_num_nodes) / 2
			+ (add_at_front ? nodes_to_add : 0);
		if (new_nstart < start.node)
			__BLINK::copy(start.node, finish.node + 1, new_nstart);
		else
			__BLINK::copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
	}
	else
	{
		// 配置一个新的map
		size_type new_map_size = map_size + __BLINK::max(map_size, nodes_to_add) + 2;

		map_pointer new_map = map_allocator::allocate(new_map_size);
		new_nstart = new_map + (new_map_size - new_num_nodes) / 2
			+ (add_at_front ? nodes_to_add : 0);
		// 拷贝原来的map
		__BLINK::copy(start.node, finish.node + 1, new_nstart);
		// 释放原来的map
		map_allocator::deallocate(map, map_size);
		map = new_map;
		map_size = new_map_size;
	}
	start.set_node(new_nstart);
	finish.set_node(new_nstart + old_num_nodes - 1);
}
template<class T, class Alloc, size_t BufSize>
void blink_deque<T, Alloc, BufSize>::pop_back_aux()
{
	deallocate_node(finish.first);
	finish.set_node(finish.node - 1);
	finish.cur = finish.last - 1;
	destroy(finish.cur);
}

template<class T, class Alloc, size_t BufSize>
void blink_deque<T, Alloc, BufSize>::pop_front_aux()
{
	destroy(start.cur);
	deallocate_node(start.first);
	start.set_node(start.node + 1);
	start.cur = start.first;
}

template<class T, class Alloc, size_t BufSize>
typename blink_deque<T, Alloc, BufSize>::iterator
blink_deque<T, Alloc, BufSize>::insert_aux(iterator pos, const value_type& x)
{
	difference_type index = pos - start;
	value_type x_copy = x;
	if (index < size() / 2)
	{
		// 注意iterator没有为(iter + 1)重载运算子
		// 插入点之前的元素个数较少
		push_front(front());	// 在队列首部插入一个元素
		iterator front1 = start;
		++front1;
		iterator front2 = front1;
		++front2;
		pos = start + index;
		iterator pos1 = pos;
		++pos1;
		copy(front2, pos1, front1);
	}
	else
	{
		// 插入点之后的元素个数较少
		push_back(back());
		iterator back1 = finish;
		--back1;
		iterator back2 = back1;
		--back2;
		pos = start + index;
		copy_backward(pos, back2, back1);
	}
	*pos = x_copy;
	return pos;
}

#endif // !BLINK_DEQUE
