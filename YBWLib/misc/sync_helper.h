#pragma once
#include "..\YBWLib.h"

class mutex;
class recursive_mutex;
class lock_guard_mutex;
class lock_guard_recursive_mutex;
class _impl_mutex;
class _impl_recursive_mutex;
class _impl_lock_guard_mutex;
class _impl_lock_guard_recursive_mutex;

struct adopt_lock_t {};
constexpr adopt_lock_t adopt_lock {};

class YBWLIB_API mutex {
	friend lock_guard_mutex;
public:
	mutex();
	mutex(const mutex&) = delete;
	mutex(mutex&& t) = delete;
	~mutex();
	void lock();
	bool try_lock();
	void unlock();
protected:
	_impl_mutex* pimpl;
};


class YBWLIB_API recursive_mutex {
	friend lock_guard_recursive_mutex;
public:
	recursive_mutex();
	recursive_mutex(const recursive_mutex&) = delete;
	recursive_mutex(recursive_mutex&& t) = delete;
	~recursive_mutex();
	void lock();
	bool try_lock();
	void unlock();
protected:
	_impl_recursive_mutex* pimpl;
};

class YBWLIB_API lock_guard_mutex {
public:
	explicit lock_guard_mutex(mutex& m);
	lock_guard_mutex(mutex& m, adopt_lock_t);
	lock_guard_mutex(const lock_guard_mutex&) = delete;
	lock_guard_mutex(lock_guard_mutex&&) = delete;
	~lock_guard_mutex();
protected:
	_impl_lock_guard_mutex* pimpl;
};

class YBWLIB_API lock_guard_recursive_mutex {
public:
	explicit lock_guard_recursive_mutex(recursive_mutex& m);
	lock_guard_recursive_mutex(recursive_mutex& m, adopt_lock_t);
	lock_guard_recursive_mutex(const lock_guard_recursive_mutex&) = delete;
	lock_guard_recursive_mutex(lock_guard_recursive_mutex&&) = delete;
	~lock_guard_recursive_mutex();
protected:
	_impl_lock_guard_recursive_mutex* pimpl;
};