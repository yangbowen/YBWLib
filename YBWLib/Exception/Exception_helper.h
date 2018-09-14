#pragma once
#ifndef _INCLUDED_YBWLIB_EXCEPTION_HELPER_H_
#define _INCLUDED_YBWLIB_EXCEPTION_HELPER_H_
#include <minwindef.h>
#include <exception>
#include "..\YBWLib.h"
#include "..\exception\Exception.h"

namespace Exception {
	void YBWLIB_API display_error_string(_In_ const wchar_t* error_string);
	void YBWLIB_API display_exception(_In_ const Exception::BaseException* err);
	void YBWLIB_API display_exception(_In_ const std::exception* err);
	void YBWLIB_API display_exception();
}
#endif
