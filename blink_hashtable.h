#ifndef __BLINK_HASHTABLE_H
#define __BLINK_HASHTABLE_H

#include "blink_alloc.h"
#include "blink_algobase.h"
#include "blink_iterator.h"
#include "blink_construct.h"
#include "blink_vector.h"
#include "blink_pair.h"

template<class Value>
struct __hashtable_node
{
	__hashtable_node* next;
	Value val;
};

/* 模板参数
 * Value：节点的实值类型
 * Key：节点的键值类型
 * HashFcn：hash function的函数类型
 * ExtractKey：从节点中取出键值的方法
 * EqualKey：判断键值相同与否的方法
 * Alloc：空间配置器
 */
template<class Value, class Key, class HashFcn, 
	class ExtractKey, class EqualKey, class Alloc=alloc>
class hashtable;

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_iterator;

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
struct __hashtable_const_iterator;

template<class Value, class Key, class HashFcn, 
	class ExtractKey, class EqualKey, class Alloc>

struct __hashtable_iterator
{
	typedef hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> 
		hashtable;
	typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc> 
		iterator;
	typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
		const_iterator;
	typedef __hashtable_node<Value> node;

	typedef forward_iterator_tag iterator_category;
	typedef Value value_type;
	typedef ptrdiff_t difference_type;
	typedef size_t size_type;
	typedef Value& reference;
	typedef Value* pointer;

	node* cur;
	hashtable* ht;

	__hashtable_iterator(node* n, hashtable* tab):cur(n),ht(tab){}
	__hashtable_iterator(){}
	reference operator* ()const { return cur->val; }
	pointer operator->()const { return &(operator*()); }
	iterator& operator++();
	iterator operator++(int);

	bool operator==(const iterator& it)const { return cur == it.cur; }
	bool operator!=(const iterator& it)const { return cur != it.cur; }

};

static const int __blink_num_primes = 28;
static const unsigned long __blink_prime_list[__blink_num_primes] =
{
	53ul,         97ul,         193ul,       389ul,       769ul,
	1543ul,       3079ul,       6151ul,      12289ul,     24593ul,
	49157ul,      98317ul,      196613ul,    393241ul,    786433ul,
	1572869ul,    3145739ul,    6291469ul,   12582917ul,  25165843ul,
	50331653ul,   100663319ul,  201326611ul, 402653189ul, 805306457ul,
	1610612741ul, 3221225473ul, 4294967291ul
};

inline unsigned long __blink_next_prime(unsigned long n)
{
	const unsigned long* first = __blink_prime_list;
	const unsigned long* last = __blink_prime_list + __blink_num_primes;
	const unsigned long* pos = __BLINK::lower_bound(first, last, n);
	return pos == last ? *(last - 1) : *pos;
}

template<class Value, class Key, class HashFcn, 
	class ExtractKey, class EqualKey, class Alloc>
class hashtable
{
public:
	typedef Value value_type;
	typedef Key key_type;
	typedef HashFcn hasher;
	typedef EqualKey key_equal;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;

	hasher hash_funct()const { return hash; }
	key_equal key_eq()const { return equals; }

private:
	hasher hash;
	key_equal equals;
	ExtractKey get_key;

	typedef __hashtable_node<Value> node;
	typedef simple_alloc<node, Alloc> node_allocator;

	blink_vector<node*, Alloc> buckets;
	size_type num_elements;

public:
	typedef __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
		iterator;
	typedef __hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
		const_iterator;

	friend struct
		__hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>;

	friend struct
		__hashtable_const_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>;

public:
	hashtable(size_type n,
		const HashFcn& hf,
		const EqualKey& eql)
		:hash(hf), equals(eql), get_key(ExtractKey()), num_elements(0)
	{
		initialize_buckets(n);
	}

	~hashtable() { clear(); }

	size_type size()const { return num_elements; }
	size_type max_size()const { return size_type(-1); }
	bool empty()const { return size() == 0; }

	void swap(hashtable& ht)
	{
		__BLINK::swap(hash, ht.hash);
		__BLINK::swap(equals, ht.equals);
		__BLINK::swap(get_key, ht.get_key);
		buckets.swap(ht.buckets);
		__BLINK::swap(num_elements, ht.num_elements);
	}

	iterator begin()
	{
		for (size_type n = 0; n < buckets.size(); ++n)
		{
			if (buckets[n])
				return iterator(buckets[n], this);
		}
		return end();
	}

	iterator end() { return iterator(0, this); }

	const_iterator begin()const
	{
		for (size_type n = 0; n < buckets.size(); ++n)
		{
			if (buckets[n])
				return const_iterator(buckets[n], this);
		}
		return end();
	}

	const_iterator end() const { return const_iterator(0, this); }


public:
	size_type bucket_count() const { return buckets.size(); }

	size_type max_bucket_count()const
	{
		return __blink_prime_list[(int)__blink_num_primes - 1];
	}

	size_type elems_in_bucket(size_type bucket)const
	{
		size_type result = 0;
		for (node* cur = buckets[bucket]; cur; cur = cur->next)
			result += 1;
		return result;
	}

	blink_pair<iterator, bool>insert_unique(const value_type& obj)
	{
		resize(num_elements + 1);
		return insert_unique_noresize(obj);
	}

	blink_pair<iterator, bool> insert_unique_noresize(const value_type& obj);
	
	reference find_or_insert(const value_type& obj);

	iterator find(const key_type& key)
	{
		size_type n = bkt_num_key(key);
		node* first;
		for(first = buckets[n];
			first && !equals(get_key(first->val), key);
			first = first->next)
		{ }
		return iterator(first, this);
	}

	const_iterator find(const key_type& key)const
	{
		size_type n = bkt_num_key(key);
		node* first;
		for (first = buckets[n];
			first && !equals(get_key(first->val), key);
			first = first->next)
		{
		}
		return const_iterator(first, this);
	}

	size_type count(const key_type& key)const
	{
		const size_type n = bkt_num_key(key);
		size_type result = 0;

		for(const node* cur = buckets[n]; cur;cur=cur->next)
		{
			if (equals(get_key(cur->val), key))
				++result;
		}
		return result;
	}

	blink_pair<iterator, iterator>
		equal_range(const key_type& key);
	blink_pair<const_iterator, const_iterator>
		equal_range(const key_type& key)const;

	size_type erase(const key_type& key);
	void erase(const iterator& it);
	void erase(iterator first, iterator last);

	void erase(const const_iterator& it);
	void erase(const_iterator first, const_iterator last);

	void resize(size_type num_elements_hint);
	void clear();

private:
	size_type next_size(size_type n)const
	{
		return __blink_next_prime(n);
	}

	void initialize_buckets(size_type n)
	{
		const size_type n_buckets = next_size(n);
		buckets.reserve(n_buckets);
		buckets.insert(buckets.end(), n_buckets, (node*)0);
		num_elements = 0;
	}

	size_type bkt_num_key(const key_type& key)const
	{
		return bkt_num_key(key, buckets.size());
	}

	size_type bkt_num(const value_type& obj)const
	{
		return bkt_num_key(get_key(obj));
	}

	size_type bkt_num_key(const key_type& key, size_t n)const
	{
		return hash(key) % n;
	}

	size_type bkt_num(const value_type& obj, size_t n)const
	{
		return bkt_num_key(get_key(obj), n);
	}

	node* new_node(const value_type& obj)
	{
		node* n = node_allocator::allocate();
		n->next = 0;
		try
		{
			construct(&n->val, obj);
			return n;
		}
		catch (...)
		{
			node_allocator::deallocate(n);
			throw;
		}
	}

	void delete_node(node* n)
	{
		destroy(&n->val);
		node_allocator::deallocate(n);
	}

	void erase_bucket(const size_type n, node* first, node* last);
	void erast_bucket(const size_type n, node* last);
	void copy_from(const hashtable& ht);
};

// 在不需要重建表格时插入新节点，键值不允许重复
template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
blink_pair<typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::iterator, bool> 
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
::insert_unique_noresize(const value_type& obj)
{
	const size_type n = bkt_num(obj);
	node* first = buckets[n];

	for (node* cur = first; cur; cur = cur->next)
	{
		if (equals(get_key(cur->val), get_key(obj)))
			return blink_pair<iterator, bool>(iterator(cur, this), false);
	}
	node* tmp = new_node(obj);
	tmp->next = first;
	buckets[n] = tmp;
	++num_elements;
	return blink_pair<iterator, bool>(iterator(tmp, this), true);
}

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::reference
hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
	::find_or_insert(const value_type& obj)
{
	resize(num_elements + 1);
	size_type n = bkt_num(obj);
	node* first = buckets[n];
	for (node* cur = first; cur; cur = cur->next)
	{
		if(equals(get_key(cur->val), get_key(obj)))
			return cur->val;
	}
	node* tmp = new_node(obj);
	tmp->next = first;
	buckets[n] = tmp;
	++num_elements;
	return tmp->val;
}

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
blink_pair<typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::iterator,
	typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::iterator>
	hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::equal_range(const key_type& key)
{
	typedef blink_pair<iterator, iterator> Pii;
	const size_type n = bkt_num_key(key);
	
	for (node* first = buckets[n]; first; first = first->next)
	{
		if (equals(get_key(first->val), key))
		{
			for (node* cur = first->next; cur; cur = cur->next)
				if (!equals(get_key(cur->val), key))
					return Pii(iterator(first, this), iterator(cur, this));
			for (size_type m = n + 1; m < buckets.size(); ++m)
				if (buckets[m])
					return Pii(iterator(first, this), iterator * buckets[m], this);
			return Pii(iterator(first, this), end());
		}
	}
	return Pii(end(), end());
}

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
blink_pair<typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::const_iterator,
	typename hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::const_iterator>
	hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::equal_range(const key_type& key)const
{
	typedef blink_pair<const_iterator, const_iterator> Pii;
	const size_type n = bkt_num_key(key);

	for (node* first = buckets[n]; first; first = first->next)
	{
		if (equals(get_key(first->val), key))
		{
			for (node* cur = first->next; cur; cur = cur->next)
				if (!equals(get_key(cur->val), key))
					return Pii(const_iterator(first, this), const_iterator(cur, this));
			for (size_type m = n + 1; m < buckets.size(); ++m)
				if (buckets[m])
					return Pii(const_iterator(first, this), const_iterator * buckets[m], this);
			return Pii(const_iterator(first, this), end());
		}
	}
	return Pii(end(), end());
}

template<class Value, class Key, class HashFcn, 
	class ExtractKey, class EqualKey, class Alloc>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
	::resize(size_type num_elements_hint)
{
	/* 表格是否重建：新增元素计入后的元素个数与buckets vector的大小作比较
	* 每个bucket的最大容量和buckets vector的大小相同
	*/
	const size_type old_n = buckets.size();
	if (num_elements_hint > old_n)
	{
		const size_type n = next_size(num_elements_hint);
		if (n > old_n)
		{
			blink_vector < node*, Alloc> tmp(n, (node*)0);
			try
			{
				for (size_type bucket = 0; bucket < old_n; ++bucket)
				{
					// 处理每一个旧的bucket
					node* first = buckets[bucket];
					while (first)
					{
						size_type new_bucket = bkt_num(first->val, n);
						buckets[bucket] = first->next;
						first->next = tmp[new_bucket];
						tmp[new_bucket] = first;
						first = buckets[bucket];
					}
				}
				buckets.swap(tmp);
			}
			catch (...)
			{

			}
		}
	}
}

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::clear()
{
	for (size_type i = 0; i < buckets.size(); ++i)
	{
		node* cur = buckets[i];
		while (cur != 0)
		{
			node* next = cur->next;
			delete_node(cur);
			cur = next;
		}
		buckets[i] = 0;
	}
	num_elements = 0;
}

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
void hashtable<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::copy_from(const hashtable& ht) 
{
	buckets.clear();
	// 为己方的buckets vector保留空间，使与对方相同
	// 如果己方空间大于对方，就不动，如果己方空间小于对方，就会增大
	buckets.reserve(ht.buckets.size());
	buckets.insert(buckets.end(), ht.buckets.size(), (node*)0);
	try
	{
		for (size_type i = 0; i < ht.buckets.size(); ++i)
		{
			if (const node* cur = ht.buckets[i])
			{
				node* copy = new_node(cur->val);
				buckets[i] = copy;
				for (node* next = cur->next; next; cur = next, next = cur->next)
				{
					copy->next = new_node(next->val);
					copy = copy->next;
				}
			}
		}
		num_elements = ht.num_elements;
	}
	catch (...)
	{
		clear();
		throw;
	}
}

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
__hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>&
__hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::operator++()
{
	const node* old = cur;
	cur = cur->next;
	if (!cur)
	{
		size_type bucket = ht->bkt_num(old->val);
		while (!cur && ++bucket < ht->buckets.size())
			cur = ht->buckets[bucket];
	}
	return *this;
}

template<class Value, class Key, class HashFcn,
	class ExtractKey, class EqualKey, class Alloc>
inline __hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>
__hashtable_iterator<Value, Key, HashFcn, ExtractKey, EqualKey, Alloc>::operator++(int)
{
	iterator tmp = *this;
	++* this;
	return tmp;
}

#endif