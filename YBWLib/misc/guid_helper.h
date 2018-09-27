#pragma once
#ifndef _INCLUDED_YBWLIB_GUID_HELPER_H_
#define _INCLUDED_YBWLIB_GUID_HELPER_H_
#include <minwindef.h>
#include <guiddef.h>
#include "../YBWLib.h"

YBWLIB_API size_t func_hash_guid(const GUID* t);
struct hash_guid {
	FORCEINLINE size_t operator()(const GUID& t) const {
		return func_hash_guid(&t);
	}
};

YBWLIB_API bool func_equal_to_guid(const GUID* l, const GUID* r);
struct equal_to_guid {
	FORCEINLINE bool operator()(const GUID& l, const GUID& r) const {
		return func_equal_to_guid(&l, &r);
	}
};

YBWLIB_API bool func_less_than_guid(const GUID* l, const GUID* r);
struct less_than_guid {
	FORCEINLINE bool operator()(const GUID& l, const GUID& r) const {
		return func_less_than_guid(&l, &r);
	}
};

constexpr hash_guid obj_hash_guid {};
constexpr equal_to_guid obj_equal_to_guid {};
constexpr less_than_guid obj_less_than_guid {};
#endif
