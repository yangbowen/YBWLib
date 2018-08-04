#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <guiddef.h>
#include <string>
#include "guid_helper.h"

size_t func_hash_guid(const GUID* t) {
	std::hash<std::string> hash;
	return hash(std::string((const char*)t, sizeof(GUID) / sizeof(char)));
}

bool func_equal_to_guid(const GUID* l, const GUID* r) {
	return IsEqualGUID(*l, *r);
}

bool func_less_than_guid(const GUID* l, const GUID* r) {
	static_assert(sizeof(GUID) == 0x10, L"The size of GUID is not 0x10.");
	static_assert(sizeof(uint32_t) == 0x4, L"The size of uint32_t is not 0x4.");
	for (int i = 0; i < 0x4; ++i) {
		if (reinterpret_cast<const uint32_t*>(l)[i] < reinterpret_cast<const uint32_t*>(r)[i]) return true;
		if (reinterpret_cast<const uint32_t*>(r)[i] != reinterpret_cast<const uint32_t*>(l)[i]) return false;
	}
	return false;
}