#pragma once
#include <minwindef.h>
#include <Ole2.h>
#include "..\YBWLib.h"

class YBWLIB_API unique_handle {
public:
	unique_handle() {}
	unique_handle(nullptr_t) {}
	unique_handle(HANDLE t) {
		this->reset(t);
	}
	unique_handle(const unique_handle& t) = delete;
	unique_handle(unique_handle&& t) {
		this->reset(t.release());
	}
	~unique_handle() {
		this->reset();
	}
	HANDLE get() const {
		return this->handle;
	}
	HANDLE release() {
		HANDLE t = this->handle;
		this->handle = nullptr;
		return t;
	}
	void reset(HANDLE t = nullptr);
	void swap(unique_handle& x) {
		HANDLE t;
		t = x.handle;
		x.handle = this->handle;
		this->handle = t;
	}
	operator bool() const {
		return this->get();
	}
	operator HANDLE() const {
		return this->get();
	}
	unique_handle& operator=(nullptr_t) {
		this->reset();
		return *this;
	}
	unique_handle& operator=(HANDLE t) {
		this->reset(t);
		return *this;
	}
	unique_handle& operator=(const unique_handle& t) = delete;
	unique_handle& operator=(unique_handle&& t) {
		this->reset(t.release());
		return *this;
	}
	bool operator==(const unique_handle& r) const {
		return this->get() == r.get();
	}
	bool operator!=(const unique_handle& r) const {
		return this->get() != r.get();
	}
protected:
	HANDLE handle = nullptr;
};

FORCEINLINE bool operator==(const unique_handle& l, nullptr_t) {
	return !l.get();
}

FORCEINLINE bool operator==(nullptr_t, const unique_handle& r) {
	return !r.get();
}

FORCEINLINE bool operator!=(const unique_handle& l, nullptr_t) {
	return l.get();
}

FORCEINLINE bool operator!=(nullptr_t, const unique_handle& r) {
	return r.get();
}

BOOL CloseHandle(const unique_handle&) = delete;

class YBWLIB_API unique_hdc {
public:
	unique_hdc() {}
	unique_hdc(nullptr_t) {}
	unique_hdc(HDC t) {
		this->reset(t);
	}
	unique_hdc(const unique_hdc& t) = delete;
	unique_hdc(unique_hdc&& t) {
		this->reset(t.release());
	}
	~unique_hdc() {
		this->reset();
	}
	HDC get() const {
		return this->hdc;
	}
	HDC release() {
		HDC t = this->hdc;
		this->hdc = nullptr;
		return t;
	}
	void reset(HDC t = nullptr);
	void swap(unique_hdc& x) {
		HDC t;
		t = x.hdc;
		x.hdc = this->hdc;
		this->hdc = t;
	}
	operator bool() const {
		return this->get();
	}
	operator HDC() const {
		return this->get();
	}
	unique_hdc& operator=(nullptr_t) {
		this->reset();
		return *this;
	}
	unique_hdc& operator=(HDC t) {
		this->reset(t);
		return *this;
	}
	unique_hdc& operator=(const unique_hdc& t) = delete;
	unique_hdc& operator=(unique_hdc&& t) {
		this->reset(t.release());
		return *this;
	}
	bool operator==(const unique_hdc& r) const {
		return this->get() == r.get();
	}
	bool operator!=(const unique_hdc& r) const {
		return this->get() != r.get();
	}
protected:
	HDC hdc = nullptr;
};

FORCEINLINE bool operator==(const unique_hdc& l, nullptr_t) {
	return !l.get();
}

FORCEINLINE bool operator==(nullptr_t, const unique_hdc& r) {
	return !r.get();
}

FORCEINLINE bool operator!=(const unique_hdc& l, nullptr_t) {
	return l.get();
}

FORCEINLINE bool operator!=(nullptr_t, const unique_hdc& r) {
	return r.get();
}

BOOL DeleteDC(const unique_hdc&) = delete;

class YBWLIB_API olestr {
public:
	olestr() {}
	olestr(nullptr_t) {}
	// Do NOT pass a normal wide-char string to this constructor.
	explicit olestr(BSTR t) {
		this->reset(t);
	}
	olestr(const olestr& t) : olestr() {
		*this = t;
	}
	olestr(olestr&& t) {
		this->reset(t.release());
	}
	~olestr() {
		this->reset();
	}
	BSTR get() const {
		return this->bstr;
	}
	BSTR release() {
		BSTR t = this->bstr;
		this->bstr = nullptr;
		return t;
	}
	void reset(BSTR t = nullptr);
	void swap(olestr& x) {
		BSTR t;
		t = x.bstr;
		x.bstr = this->bstr;
		this->bstr = t;
	}
	operator bool() const {
		return this->get();
	}
	operator BSTR() const {
		return this->get();
	}
	olestr& operator=(nullptr_t) {
		this->reset();
		return *this;
	}
	olestr& operator=(const olestr& t);
	olestr& operator=(olestr&& t) {
		this->reset(t.release());
		return *this;
	}
	bool operator==(const olestr& r) const {
		return this->get() == r.get();
	}
	bool operator!=(const olestr& r) const {
		return this->get() != r.get();
	}
protected:
	BSTR bstr = nullptr;
};

void SysFreeString(const olestr&) = delete;