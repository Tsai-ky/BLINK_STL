#ifndef __BLINK_SET_H
#define __BLINK_SET_H
#include "blink_alloc.h"
#include "blink_iterator.h"
#include "blink_function.h"
#include "blink_tree.h"
#include "blink_config.h"

template<class Key, class Compare=less<Key>, class Alloc=alloc>
class blink_set
{
public:
	typedef Key key_type;
	typedef Key value_type;
	typedef Compare key_compare;
	typedef Compare value_compare;

private:
	typedef blink_rb_tree<key_type, value_type, identity<value_type>, key_compare, Alloc> rep_type;
	rep_type t;

public:
	typedef typename rep_type::const_pointer pointer;
	typedef typename rep_type::const_pointer const_pointer;
	typedef typename rep_type::const_reference reference;
	typedef typename rep_type::const_reference const_reference;
	typedef typename rep_type::const_iterator iterator;
	/* set 的迭代器无法执行写入操作
	* 这是因为set的元素有一定的次序排列
	* 不允许用户在任意处进行写入操作
	*/
	typedef typename rep_type::const_iterator const_iterator;
	typedef typename rep_type::size_type size_type;
	typedef typename rep_type::difference_type difference_type;

	blink_set():t(Compare()){}
	explicit blink_set(const Compare& comp):t(comp){}
	
	template<class InputIterator>
	blink_set(InputIterator first, InputIterator last):t(Compare())
	{
		t.insert_unique(first, last);
	}

	template<class InputIterator>
	blink_set(InputIterator first, InputIterator last, const Compare& comp) : t(comp)
	{
		t.insert_unique(first, last);
	}

	blink_set(const blink_set<Key, Compare, Alloc>& x):t(x.t){}

	blink_set<Key, Compare, Alloc>& operator=(const blink_set<Key, Compare, Alloc>& x)
	{
		t = x.t;
		return *this;
	}

	key_compare key_comp()const { return t.key_comp(); }
	value_compare value_comp()const { return t.key_comp(); }
	iterator begin()const { return t.begin(); }
	iterator end()const { return t.end(); }
	bool empty()const { return t.empty(); }
	size_type size()const { return t.size(); }
	size_type max_size()const { return t.max_size(); }
	void swap(blink_set<Key, Compare, Alloc>& x) { t.swap(x.t); }

	typedef blink_pair<iterator, bool> pair_iterator_bool;
	blink_pair<iterator, bool> insert(const value_type& x)
	{
		blink_pair<typename rep_type::iterator::iterator, bool>p = t.insert_unique(x);
		return blink_pair<iterator, bool>(p.first, p.second);
	}
	iterator insert(iterator position, const value_type& x)
	{
		typedef typename rep_type::iterator rep_iterator;
		return t.insert_unique((rep_iterator&)position, x);
	}
	
	template<class InputIterator>
	void insert(InputIterator first, InputIterator last)
	{
		t.insert_unique(first, last);
	}

	void erase(iterator position)
	{
		typedef typename rep_type::iterator rep_iterator;
		t.erase((rep_iterator&)position);
	}
	size_type erase(const key_type& x)
	{
		return t.erase(x);
	}
	void erase(iterator first, iterator last)
	{
		typedef typename rep_type::iterator rep_iterator;
		return t.erase((rep_iterator&)first, (rep_iterator&)last);
	}

	void clear() { t.clear(); }

	iterator find(const key_type& x)const { return t.find(x); }
	size_type count(const key_type& x)const { return t.count(x); }
};

template<class Key, class Compare, class Alloc>
inline bool operator ==(const blink_set<Key, Compare, Alloc>& x,
	const blink_set<Key, Compare, Alloc>& y)
{
	return x.t == y.count;
}

template<class Key, class Compare, class Alloc>
inline bool operator <(const blink_set<Key, Compare, Alloc>& x,
	const blink_set<Key, Compare, Alloc>& y)
{
	return x.t < y.t;
}
#endif