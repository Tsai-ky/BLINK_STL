#ifndef __BLINK_DEQUE_H
#define __BLINK_DEQUE_H
#include"blink_alloc.h"
#include"blink_uninitialized.h"
#include"blink_algobase.h"

/*
* deque ����ָ����������С��Ĭ��ֵΪ0��ʾ��ʹ��512 bytes������
* ��� n Ϊ 0�����ʾ buffer_size ʹ��Ĭ��ֵ
*	��Ԫ�ش�СszС��512����buffer�������� 512 / sz
*	��Ԫ�ش�Сsz����512���򷵻�1
* ��� n ��Ϊ 0�����ʾ�û��Զ��� buffer_sizeΪ n
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

	T* cur;				// ��ǰ��������ָ�������е�����Ԫ��
	T* first;			// ��ǰ��������ָ��������ͷ
	T* last;			// ��ǰ��������ָ��������β
	map_pointer node;	// ָ�� map ��ָ��ǰ��������node

	// ���õ�������ָ��Ļ�����
	void set_node(map_pointer new_node)
	{
		// new_node ��һ��ָ�� map Ԫ�� node ��ָ��
		// map ��Ԫ�ض���ָ�룬ָ�򻺳���
		node = new_node;
		first = *new_node;
		last = first + difference_type(buffer_size());
	}

	// ����������
	reference operator * ()const { return *cur; }
	pointer operator -> () const { return &(operator * ()); }

	difference_type operator -(const self& x) const
	{
		return difference_type(buffer_size()) * (node - x.node - 1) +
			(cur - first) + (x.last - x.cur);
	}

	self& operator ++()	//++iter
	{
		++cur;	// �л�����һ��Ԫ��
		if (cur == last)	//����ѵ��ﻺ����ĩβ
		{
			set_node(node + 1);	// �л�����һ��㣨�������� �ĵ�һ��Ԫ��
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
		if (cur == first)	// �����ǰ�Ѵ��ڻ������ײ�
		{
			set_node(node - 1);	//�л���ǰһ�����Ľ�β
			cur = last;
		}
		--cur;	// �л���ǰһ���������һ�����
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
			// Ŀ��λ����ͬһ������
			cur += n;
		}
		else
		{
			// Ŀ��λ�ò���ͬһ������
			difference_type node_offset;
			if (offset > 0)
				node_offset = offset / (buffer_size());
			else
				node_offset = -difference_type((-offset - 1) / buffer_size()) - 1;
			// �л���Ŀ�껺����
			set_node(node + node_offset);
			// �л�����ȷ��Ԫ��
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
	// ����ͷ����ַ == ���е�һ��Ԫ�ص�ַ
	// ����β����ַ == �������һ��Ԫ�ص�ַ��Ԫ�ص�ַ��Ԫ���׵�ַ�� + 1
	iterator start;	// ����ͷ��
	iterator finish;	//����β����ע�ⲻ�Ƕ������һ��Ԫ�أ�

	// map ָ��ָ��map��map��һ�������ռ�
	// map ��ÿһ��Ԫ�ض���һ��ָ�루��㣩��ÿ��ָ��ָ��һ��������
	map_pointer map;
	size_type map_size; // map ��С
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
			// ���mapβ�˵Ľڵ㱸�ÿռ䲻��ʱ
			// ����һ�������map������ԭ���ģ��ͷ�ԭ����
			reallocate_map(nodes_to_add, false);
	}

	void reserve_map_at_front(size_type nodes_to_add = 1)
	{
		if (nodes_to_add > start.node - map)
			// ���mapͷ���Ľڵ㱸�ÿռ䲻��ʱ
			// ����һ�������map������ԭ���ģ��ͷ�ԭ����
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
		// �˴�����д��return *(finish - 1);
		// ԭ���� __deque_iterator<>û��Ϊ(finish - 1)����������
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
			// �ײ����������п��пռ�
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
		// clear()��ɺ�ظ���ʼ״̬
		// ��ʼ״̬��deque����һ��������
		for (map_pointer node = start.node + 1; node < finish.node; ++node)
		{
			destroy(*node, *node + iterator::buffer_size());
			data_allocator::deallocate(*node, iterator::buffer_size());
		}
		if (start.node != finish.node)
		{
			// ʣ������������
			// ����ʣ��Ԫ��
			destroy(start.cur, start.last);
			destroy(finish.first, finish.cur);
			// �ͷ�β��������
			data_allocator::deallocate(finish.first, iterator::buffer_size());

		}
		else
		{
			// ֻ��һ��������
			// ����ʣ��Ԫ��
			destroy(start.cur, finish.cur);
			// ע���ʼ״̬����Ҫ����һ��������
		}
	}

	// �������Ԫ��
	iterator erase(iterator pos)
	{
		iterator next = pos;
		++next;
		difference_type index = pos - start;
		if (index < (size() >> 1))
		{	// ��������֮ǰ��Ԫ�رȽ���
			// �����Ƴ�Ԫ�ؿ����������ײ����Ƴ�
			copy_backward(start, pos, next);
			pop_back();
		}
		else
		{	// ��������֮���Ԫ�رȽ���
			// �����Ƴ�Ԫ�ؿ���������β�����Ƴ�
			copy(next, finish, pos);
			pop_back();
		}
		return start + index;
	}

	// ���[first, last)�����ڵ�����Ԫ��
	iterator earse(iterator first, iterator last)
	{
		if (first == start && last == finish)
		{ // �������������deque
			clear();
			return finish;
		}
		else
		{
			difference_type n = last - first;
			difference_type elems_before = first - start;
			if (elems_before < (size() - n) / 2)
			{
				// �������ǰ����Ԫ�ظ�������
				// ����ƶ�ǰ��Ԫ��
				copy_backward(start, first, last);
				iterator new_start = start + n;
				destroy(start, new_start);
				for (map_pointer cur = start.node; cur < new_start.node; ++cur)
					data_allocator::deallocate(*cur, iterator::buffer_size());
				start = new_start;
			}
			else
			{
				// �������󷽵�Ԫ�ظ�������
				// ��ǰ�ƶ���Ԫ��
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
	// Ϊÿ�����Ļ������趨��ֵ
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
	// ��Ҫ�ڵ��� = ��Ԫ�ظ��� / ÿ�������������ɵ�Ԫ�ظ����� + 1
	size_type num_nodes = num_elements / iterator::buffer_size() + 1;

	// map ����ڵ�������Ϊ8�������������ڵ��� + 2
	// ǰ���Ԥ��һ��������ʱʹ��
	map_size = __BLINK::max(size_t(initial_map_size), num_nodes + 2);
	map = map_allocator::allocate(map_size);

	// �� nstart �� nfinish ָ�� map ��ӵ�����нڵ������������
	map_pointer nstart = map + (map_size - num_nodes) / 2;
	map_pointer nfinish = nstart + num_nodes - 1;

	map_pointer cur;

	try
	{
		// Ϊmap�ڵ�ÿ���ڵ����û�����
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
	// �����һ��������ֻʣ��һ������Ԫ�ؿռ�ʱ�Żᱻ����
	value_type x_copy = x;
	reserve_map_at_back(); // ��Ҫʱ��Ҫ��������һ���µ�map
	*(finish.node + 1) = allocate_node(); // ����һ���µĽڵ�
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
		// ԭ����map�ռ��㹻
		// �ƶ�map�еĽڵ�
		new_nstart = map + (map_size - new_num_nodes) / 2
			+ (add_at_front ? nodes_to_add : 0);
		if (new_nstart < start.node)
			__BLINK::copy(start.node, finish.node + 1, new_nstart);
		else
			__BLINK::copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
	}
	else
	{
		// ����һ���µ�map
		size_type new_map_size = map_size + __BLINK::max(map_size, nodes_to_add) + 2;

		map_pointer new_map = map_allocator::allocate(new_map_size);
		new_nstart = new_map + (new_map_size - new_num_nodes) / 2
			+ (add_at_front ? nodes_to_add : 0);
		// ����ԭ����map
		__BLINK::copy(start.node, finish.node + 1, new_nstart);
		// �ͷ�ԭ����map
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
		// ע��iteratorû��Ϊ(iter + 1)����������
		// �����֮ǰ��Ԫ�ظ�������
		push_front(front());	// �ڶ����ײ�����һ��Ԫ��
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
		// �����֮���Ԫ�ظ�������
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
