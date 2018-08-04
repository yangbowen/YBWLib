#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "..\exception\Exception.h"
#include "ptr_helper.h"

void unique_handle::reset(HANDLE t) {
	if (this->handle) {
		if (!CloseHandle(this->handle)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "CloseHandle");
		this->handle = nullptr;
	}
	if (!t || t == INVALID_HANDLE_VALUE)
		this->handle = nullptr;
	else
		this->handle = t;
}

void unique_hdc::reset(HDC t) {
	if (this->hdc) {
		if (!DeleteDC(this->hdc)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "CloseHandle");
		this->hdc = nullptr;
	}
	this->hdc = t;
}

void olestr::reset(BSTR t) {
	if (this->bstr) {
		SysFreeString(this->bstr);
		this->bstr = nullptr;
	}
	this->bstr = t;
}

olestr& olestr::operator=(const olestr& t) {
	if (t.get()) {
		this->reset(SysAllocStringLen(t.get(), SysStringLen(t.get())));
	} else {
		this->reset();
	}
	return *this;
}