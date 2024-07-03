#ifndef __BLINK_HASH_FUNC_H
#define __BLINK_HASH_FUNC_H

#include "blink_config.h"

template<class Key> struct hash {};

inline size_t __blink_hash_string(const char* s)
{
	unsigned long h = 0;
	for (; *s; ++s)
	{
		h = 5 * h + *s;
	}
	return size_t(h);
}

__BLINK_TEMPLATE_NULL struct hash<char*>
{
	size_t operator()(const char* s)const { return __blink_hash_string(s); }
};

__BLINK_TEMPLATE_NULL struct hash<const char*>
{
	size_t operator()(const char* s)const { return __blink_hash_string(s); }
};

__BLINK_TEMPLATE_NULL struct hash<char>
{
	size_t operator()(char x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<unsigned char>
{
	size_t operator()(unsigned char x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<signed char>
{
	size_t operator()(unsigned char x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<short>
{
	size_t operator()(short x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<unsigned short>
{
	size_t operator()(unsigned short x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<int>
{
	size_t operator()(int x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<unsigned int>
{
	size_t operator()(unsigned int x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<long>
{
	size_t operator()(long x)const { return x; }
};

__BLINK_TEMPLATE_NULL struct hash<unsigned long>
{
	size_t operator()(unsigned long x)const { return x; }
};
#endif