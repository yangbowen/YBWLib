#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <mutex>
#include "pimpl_helper.h"
#include "sync_helper.h"

class _impl_mutex {
public:
	mutex* pdecl = nullptr;
	std::mutex mtx;
};

class _impl_recursive_mutex {
public:
	recursive_mutex* pdecl = nullptr;
	std::recursive_mutex mtx;
};

class _impl_lock_guard_mutex {
public:
	lock_guard_mutex* pdecl = nullptr;
	explicit _impl_lock_guard_mutex(_impl_mutex& m) : lock(m.mtx) {}
	_impl_lock_guard_mutex(_impl_mutex& m, adopt_lock_t) : lock(m.mtx, std::adopt_lock) {}
	std::lock_guard<std::mutex> lock;
};

class _impl_lock_guard_recursive_mutex {
public:
	lock_guard_recursive_mutex* pdecl = nullptr;
	explicit _impl_lock_guard_recursive_mutex(_impl_recursive_mutex& m) : lock(m.mtx) {}
	_impl_lock_guard_recursive_mutex(_impl_recursive_mutex& m, adopt_lock_t) : lock(m.mtx, std::adopt_lock) {}
	std::lock_guard<std::recursive_mutex> lock;
};

DEFINE_PIMPL_DEFAULT_CTOR(mutex);

DEFINE_PIMPL_DTOR(mutex);

void mutex::lock() {
	this->pimpl->mtx.lock();
}

bool mutex::try_lock() {
	return this->pimpl->mtx.try_lock();
}

void mutex::unlock() {
	this->pimpl->mtx.unlock();
}

DEFINE_PIMPL_DEFAULT_CTOR(recursive_mutex);

DEFINE_PIMPL_DTOR(recursive_mutex);

void recursive_mutex::lock() {
	this->pimpl->mtx.lock();
}

bool recursive_mutex::try_lock() {
	return this->pimpl->mtx.try_lock();
}

void recursive_mutex::unlock() {
	this->pimpl->mtx.unlock();
}

lock_guard_mutex::lock_guard_mutex(mutex& m) {
	this->pimpl = new _impl_lock_guard_mutex(*m.pimpl);
	this->pimpl->pdecl = this;
}

lock_guard_mutex::lock_guard_mutex(mutex& m, adopt_lock_t) {
	this->pimpl = new _impl_lock_guard_mutex(*m.pimpl, adopt_lock);
	this->pimpl->pdecl = this;
}

DEFINE_PIMPL_DTOR(lock_guard_mutex);

lock_guard_recursive_mutex::lock_guard_recursive_mutex(recursive_mutex& m) {
	this->pimpl = new _impl_lock_guard_recursive_mutex(*m.pimpl);
	this->pimpl->pdecl = this;
}

lock_guard_recursive_mutex::lock_guard_recursive_mutex(recursive_mutex& m, adopt_lock_t) {
	this->pimpl = new _impl_lock_guard_recursive_mutex(*m.pimpl, adopt_lock);
	this->pimpl->pdecl = this;
}

DEFINE_PIMPL_DTOR(lock_guard_recursive_mutex);