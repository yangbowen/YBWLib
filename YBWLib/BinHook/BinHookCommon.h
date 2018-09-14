#pragma once
#ifndef _INCLUDED_YBWLIB_BINHOOKCOMMON_H_
#define _INCLUDED_YBWLIB_BINHOOKCOMMON_H_
#include <minwindef.h>
#include "..\YBWLib.h"

namespace BinHook {
	class SuspendOtherThreads;
	class _impl_SuspendOtherThreads;

	/// <summary>An object that can suspend other threads in the process to prevent data race when applying binary hooks.</summary>
	class YBWLIB_API SuspendOtherThreads {
	public:
		explicit SuspendOtherThreads();
		SuspendOtherThreads(const SuspendOtherThreads&) = delete;
		SuspendOtherThreads(SuspendOtherThreads&& t);
		virtual ~SuspendOtherThreads();
		virtual bool GetHasSuspended() const;
		/// <summary>Add a thread that should be exempted from suspending.</summary>
		/// <param name="id_thread">The thread ID of the thread.</param>
		virtual void AddExemptedThread(_In_ DWORD id_thread);
		virtual void SuspendThreads();
		virtual void ResumeThreads();
	protected:
		_impl_SuspendOtherThreads* pimpl = nullptr;
	};
}
#endif
