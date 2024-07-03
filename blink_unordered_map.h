#ifndef __BLINK_UNORDERED_MAP_H
#define __BLINK_UNORDERED_MAP_H

#include "blink_hashtable.h"
#include "blink_hash_func.h"
#include "blink_function.h"
#include "blink_pair.h"

template<class Key, class T, class HashFcn=hash<Key>, class EqualKey = equal_to<Key>, class Alloc=alloc>
class blink_unordered_map
{
private:
	typedef hashtable < blink_pair<const Key, T>, Key, HashFcn, 
		select1st<blink_pair<const Key, T>>, EqualKey,Alloc > ht;
	ht rep;

public:
	typedef typename ht::key_type key_type;
	typedef T data_type;
	typedef T mapped_type;
	typedef typename ht::value_type value_type;
	typedef typename ht::hasher hasher;
	typedef typename ht::key_equal key_equal;

	typedef typename ht::size_type size_type;
	typedef typename ht::difference_type difference_type;
	typedef typename ht::pointer pointer;
	typedef typename ht::const_pointer const_pointer;
	typedef typename ht::reference reference;
	typedef typename ht::const_reference const_reference;

	typedef typename ht::iterator iterator;
	typedef typename ht::const_iterator const_iterator;

	hasher hash_funct() const { return rep.hash_funct(); }
	key_equal key_eq()const { return rep.key_eq(); }

public:
	blink_unordered_map() : rep(100, hasher(), key_equal()) {}
	explicit blink_unordered_map(size_type n):rep(n, hasher(), key_equal()){}
	blink_unordered_map(size_type n, const hasher& hf):rep(n, hf, key_equal()){}
	blink_unordered_map(size_type n, const hasher& hf, const key_equal& eql)
		:rep(n, hf, eql){}

	template<class InputIterator>
	blink_unordered_map(InputIterator f, InputIterator l)
		: rep(100, hasher(), key_equal())
	{
		rep.insert_unique(f, l);
	}

	template<class InputIterator>
	blink_unordered_map(InputIterator f, InputIterator l, size_type n)
		: rep(n, hasher(), key_equal())
	{
		rep.insert_unique(f, l);
	}
	template<class InputIterator>
	blink_unordered_map(InputIterator f, InputIterator l, size_type n,
		const hasher& hf)
		: rep(n, hf, key_equal())
	{
		rep.insert_unique(f, l);
	}

	template<class InputIterator>
	blink_unordered_map(InputIterator f, InputIterator l, size_type n,
		const hasher& hf, const key_equal& eql)
		: rep(100, hf, eql)
	{
		rep.insert_unique(f, l);
	}

public:
	size_type size()const { return rep.size(); }
	size_type max_size()const { return rep.max_size(); }
	bool empty()const { return rep.empty(); }
	void swap(blink_unordered_map& hs) { rep.swap(hs.rep); }
	friend bool
		operator == (const blink_unordered_map&, const blink_unordered_map&);
	
	iterator begin() { return rep.begin(); }
	iterator end() { return rep.end(); }
	const_iterator begin()const { return rep.begin(); }
	const_iterator end()const { return rep.end(); }

public:
	blink_pair<iterator, bool>insert(const value_type& obj)
	{
		return rep.insert_unique(obj);
	}

	template<class InputIterator>
	void insert(InputIterator f, InputIterator l)
	{
		rep.insert_unique(f, l);
	}

	blink_pair<iterator, bool>insert_noresize(const value_type& obj)
	{
		return rep.insert_unique_noresize(obj);
	}

	iterator find(const key_type& key) { return rep.find(key); }
	const_iterator find(const key_type& key)const { return rep.find(key); }

	T& operator[](const key_type& key) 
	{
		return rep.find_or_insert(value_type(key, T())).second;
	}

	size_type count(const key_type& key) const { return rep.count(key); }
	blink_pair<iterator, iterator>equal_range(const key_type& key)
	{
		return rep.equal_range(key);
	}

	blink_pair<const_iterator, const_iterator>equal_range(const key_type& key)const
	{
		return rep.equal_range(key);
	}

	size_type erase(const key_type& key) { return rep.erase(key); }
	void erase(iterator it) { rep.erase(it); }
	void erase(iterator f, iterator l) { rep.erase(f, l); }

	void clear() { rep.clear(); }

public:
	void resize(size_type hint) { rep.resize(hint); }
	size_type bucket_count()const { return rep.bucket_count(); }
	size_type max_bucket_count()const { return rep.max_bucket_count(); }
	size_type elems_in_bucket(size_type n)const
	{
		return rep.elems_in_bucket(n);
	}
};

template<class Key, class T, class HashFcn, class EqualKey, class Alloc>
inline bool operator==(const blink_unordered_map<Key, T, HashFcn, EqualKey, Alloc>& hm1,
	const blink_unordered_map<Key, T, HashFcn, EqualKey, Alloc>& hm2)
{
	return hm1.rep == hm2.rep;
}

#endif