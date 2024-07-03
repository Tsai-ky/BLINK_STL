#ifndef __BLINK_VECTOR_H
#define __BLINK_VECTOR_H

#include"blink_alloc.h"
#include"blink_uninitialized.h"
#include"blink_construct.h"
#include"blink_algobase.h"
#include "type_traits.h"

template<class T, class Alloc = alloc>
class blink_vector
{
public:
	typedef T value_type;
	typedef value_type*	 pointer;
	typedef const value_type* const_pointer;
	typedef value_type*	 iterator;	//vector以普通指针为迭代器
	typedef const value_type* const_iterator;
	typedef value_type&	 reference;
	typedef const value_type& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

protected:
	// simple_alloc是一个工具类，其所有成员函数都为静态的！
	typedef simple_alloc<value_type, Alloc> data_allocator;
	iterator start; // 目前使用空间的头
	iterator finish; // 目前使用空间的尾
	iterator end_of_storage; // 目前可用空间的尾

	void insert_aux(iterator position, const T& x);
	void deallocate()
	{
		if (start)
			data_allocator::deallocate(start, end_of_storage - start);
	}

	void deallocate(T* p, size_t n)
	{
		if (p)
			data_allocator::deallocate(p, n);
	}
	void fill_initialize(size_type n, const T& value)
	{
		start = allocate_and_fill(n, value);
		finish = start + n;
		end_of_storage = finish;
	}

public:
	iterator begin()const { return start; }
	iterator end()const { return finish; }
	size_type size() const { return size_type(end() - begin()); }
	size_type capacity() const { return size_type(end_of_storage - begin()); }

	bool empty() const { return begin() == end(); }

	reference operator[](size_type n) { return *(begin() + n); }

	blink_vector() :start(0), finish(0), end_of_storage(0) {}
	blink_vector(size_type n, const T& value) { fill_initialize(n, value); }
	blink_vector(int n, const T& value) { fill_initialize(n, value); }
	blink_vector(long n, const T& value) { fill_initialize(n, value); }
	explicit blink_vector(size_type n) { fill_initialize(n, T()); }

	template<class InputIterator>
	blink_vector(InputIterator first, InputIterator last)
	{
		typedef typename Is_integer<InputIterator>::_Integral _Integral;
		initialize_aux(first, last, _Integral());
	}

	template<class Integer>
	void initialize_aux(Integer n, Integer value, __true_type)
	{
		start = data_allocator::allocate(n);
		end_of_storage = start + n;
		finish = uninitialized_fill_n(start, n, value);
	}

	template<class InputIterator>
	void initialize_aux(InputIterator first, InputIterator last, __false_type)
	{
		range_initialize(first, last, iterator_category(first));
	}

	~blink_vector()
	{
		destroy(start, finish);
		deallocate();
	}

	void reserve(size_type n)
	{
		if (capacity() < n)
		{
			const size_type old_size = size();
			iterator tmp = allocate_and_copy(n, start, finish);
			destroy(start, finish);
			deallocate();
			start = tmp;
			finish = tmp + old_size;
			end_of_storage = start + n;
		}
	}

	reference front() { return *begin(); }
	const_reference front()const { return *begin(); }
	reference back() { return *end(); }
	const_reference back()const { return *end(); }
	void push_back(const T& x)
	{
		if (finish != end_of_storage)
		{
			construct(finish, x);
			++finish;
		}
		else
		{
			insert_aux(end(), x);
		}
	}

	void swap(blink_vector<T, Alloc>& x)
	{
		__BLINK::swap(start, x.start);
		__BLINK::swap(finish, x.finish);
		__BLINK::swap(end_of_storage, x.end_of_storage);
	}

	void pop_back()
	{
		--finish;
		destroy(finish);
	}

	iterator erase(iterator position)
	{
		if (position + 1 != end())
			copy(position + 1, finish, position);
		--finish;
		destroy(finish);
		return position;
	}

	iterator erase(iterator first, iterator last)
	{
		iterator i = __BLINK::copy(last, finish, first);
		destroy(i, finish);
		finish = finish - (last - first);
		return first;
	}
	void resize(size_type new_sz, const T& x)
	{
		if (new_sz < size())
			erase(begin() + new_sz, end());
		else
			insert(end(), new_sz - size(), x);
	}

	void resize(size_type new_sz) { resize(new_sz, T()); }

	void clear() { erase(begin(), end()); }

	void insert(iterator position, size_type n, const T& x);

protected:
	iterator allocate_and_copy(size_type n, const_iterator first, const_iterator last)
	{
		iterator result = data_allocator::allocate(n);
		try
		{
			uninitialized_copy(first, last, result);
			return result;
		}
		catch (...)
		{
			deallocate(result, n);
			throw;
		}
	}

	iterator allocate_and_fill(size_type n, const T& x)
	{
		iterator result = data_allocator::allocate(n);
		uninitialized_fill_n(result, n, x);
		return result;
	}

	template<class InputIterator>
	void range_initialize(InputIterator first, 
		InputIterator last, input_iterator_tag)
	{
		for (; first != last; ++first)
			push_back(*first);
	}

	template<class ForwardIterator>
	void range_initialize(ForwardIterator first,
		ForwardIterator last, forward_iterator_tag)
	{
		size_type n = 0;
		distance(first, last, n);
		start = data_allocator::allocate(n);
		end_of_storage = start + n;
		finish = uninitialized_copy(first, last, start);
	}
};


template<class T, class Alloc>
void blink_vector<T, Alloc>::insert_aux(iterator position, const T& x)
{
	if (finish != end_of_storage)
	{
		// 在备用空间起始处以vector的最后一个元素为模板拷贝构造一个元素
		construct(finish, *(finish - 1));
		++finish;
		T x_copy = x;
		copy_backward(position, finish - 2, finish - 1);
		*position = x_copy;
	}
	else
	{
		// 备用空间不足
		const size_type old_size = size();
		const size_type len = old_size != 0 ? 2 * old_size : 1;
		// 如果原大小为0， 则配置1
		// 如果原大小不为0， 则配置原大小的两倍

		iterator new_start = data_allocator::allocate(len);
		// len 是pass by reference
		iterator new_finish = new_start;
		try
		{
			new_finish = uninitialized_copy(start, position, new_start);
			construct(new_finish, x);
			new_finish++;
			new_finish = uninitialized_copy(position, finish, new_finish);
		}
		catch (...)
		{
			destroy(new_start, new_finish);
			data_allocator::deallocate(new_start, len);
			throw;
		}

		// 回收原vector
		destroy(begin(), end());
		deallocate();

		//调整迭代器指向新的vector
		start = new_start;
		finish = new_finish;
		end_of_storage = new_start + len;
	}
}

template<class T, class Alloc>
void blink_vector<T, Alloc>::insert(iterator position, size_type n, const T& x)
{
	if (n != 0)
	{
		if (size_type(end_of_storage - finish) >= n)
		{
			// 备用空间充足
			T x_copy = x;
			// 计算插入点之后的现有元素个数
			const size_type elems_after = finish - position;
			iterator old_finish = finish;
			if (elems_after > n)
			{
				// 插入点之后的现有元素个数 > 新增元素个数
				// 将当前vector尾部n个元素向后移动
				uninitialized_copy(finish - n, finish, finish);
				finish += n;
				// 往后移动[position, old_finish - n)
				__BLINK::copy_backward(position, old_finish - n, old_finish);
				// 填充新元素
				__BLINK::fill(position, position + n, x_copy);
			}
			else
			{
				// 插入点之后的现有元素个数 < 新增元素的个数
				uninitialized_fill_n(finish, n - elems_after, x_copy);
				finish += (n - elems_after);
				uninitialized_copy(position, old_finish, finish);
				finish += elems_after;
				__BLINK::fill(position, old_finish, x_copy);
			}
		}
		else
		{
			// 备用空间不足时
			// 新长度 = max(旧长度的两倍，旧长度 + 新增元素个数)
			const size_type old_size = size();
			const size_type len = old_size + __BLINK::max(old_size, n);
			iterator new_start = data_allocator::allocate(len);
			iterator new_finish = new_start;

			try
			{
				// 将原vector 插入点之前的元素复制到新空间
				new_finish = uninitialized_copy(start, position, new_finish);
				// 将n个新增元素填充到新空间
				new_finish = uninitialized_fill_n(new_finish, n, x);
				// 将原vector插入点之后的元素复制到新空间
				new_finish = uninitialized_copy(position, finish, new_finish);
			}
			catch (...)
			{
				// commit or rollback
				destroy(new_start, new_finish);
				data_allocator::deallocate(new_start, len);
				throw;
			}
			// 回收旧vector
			destroy(start, finish);
			deallocate();

			// 调整迭代器指向新的vector
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + len;
		}
	}
}

template<class T, class Alloc>
inline void swap(blink_vector<T, Alloc>& x, blink_vector<T, Alloc>& y)
{
	x.swap(y);
}
#endif // !BLINK_VECTOR

