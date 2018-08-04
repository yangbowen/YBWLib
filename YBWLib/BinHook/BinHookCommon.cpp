#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <memory>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <TlHelp32.h>
#include "..\misc\pimpl_helper.h"
#include "..\exception\Exception.h"
#include "..\misc\ptr_helper.h"
#include "BinHookCommon.h"

namespace BinHook {
	class _impl_SuspendOtherThreads final {
	public:
		SuspendOtherThreads* pdecl = nullptr;
		explicit _impl_SuspendOtherThreads() {}
		_impl_SuspendOtherThreads(const _impl_SuspendOtherThreads&) = delete;
		_impl_SuspendOtherThreads(_impl_SuspendOtherThreads&&) = delete;
		~_impl_SuspendOtherThreads() {
			if (this->has_suspended) this->ResumeThreads();
		}
		bool GetHasSuspended() const { return this->has_suspended; }
		void AddExemptedThread(DWORD id_thread) { this->set_thread_exempted.insert(id_thread); }
		void SuspendThreads() {
			this->AddExemptedThread(GetCurrentThreadId());
			while (!this->SuspendThreadsSinglePass());
			this->has_suspended = true;
		}
		void ResumeThreads() {
			if (this->has_suspended) {
				map_thread_t::iterator it_map_thread_suspended = this->map_thread_suspended.begin();
				while (it_map_thread_suspended != this->map_thread_suspended.end()) {
					map_thread_t::iterator it_map_thread_suspended_current = it_map_thread_suspended++;
					unique_handle hthread(OpenThread(THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME | READ_CONTROL, FALSE, (*it_map_thread_suspended_current).first));
					if (!hthread) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "OpenThread");
					if (ResumeThread(hthread) == -1) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "ResumeThread");
					this->map_thread_suspended.erase(it_map_thread_suspended_current);
				}
				this->has_suspended = false;
			}
		}
	protected:
		typedef std::unordered_set<DWORD> set_thread_t;
		typedef std::unordered_map<DWORD, CONTEXT> map_thread_t;
		bool has_suspended = false;
		set_thread_t set_thread_exempted;
		map_thread_t map_thread_suspended;
		/// <summary>Enumerate the threads once, and suspend the threads that has been found and has not been suspended by previous passes yet.</summary>
		/// <returns>Returns <c>true</c> if no additional threads are found in this pass. Returns <c>false</c> otherwise.</returns>
		bool SuspendThreadsSinglePass() {
			std::unique_ptr<std::vector<THREADENTRY32> > vector_te(this->EnumThreads());
			bool no_new_thread_found = true;
			std::for_each(vector_te->begin(), vector_te->end(),
				[this, &no_new_thread_found](THREADENTRY32& val_te) {
					if (!this->map_thread_suspended.count(val_te.th32ThreadID)) {
						no_new_thread_found = false;
						unique_handle hthread(OpenThread(THREAD_QUERY_INFORMATION | THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | READ_CONTROL, FALSE, val_te.th32ThreadID));
						if (hthread && SuspendThread(hthread) != -1) {
							if (!GetThreadContext(hthread, &this->map_thread_suspended[val_te.th32ThreadID])) {
								this->map_thread_suspended.erase(val_te.th32ThreadID);
								this->set_thread_exempted.insert(val_te.th32ThreadID);
							}
						} else {
							this->set_thread_exempted.insert(val_te.th32ThreadID);
						}
					}
				});
			return no_new_thread_found;
		}
		std::vector<THREADENTRY32>* EnumThreads() {
			unique_handle hsnap(CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0));
			if (!hsnap) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "CreateToolhelp32Snapshot");
			std::unique_ptr<std::vector<THREADENTRY32> > vector_te(new std::vector<THREADENTRY32>());
			bool has_finished = false;
			if (!has_finished) {
				THREADENTRY32 te;
				memset(&te, 0, sizeof(THREADENTRY32));
				te.dwSize = sizeof(THREADENTRY32);
				if (Thread32First(hsnap, &te)) {
					if (te.th32OwnerProcessID == GetCurrentProcessId() && !this->set_thread_exempted.count(te.th32ThreadID)) vector_te->push_back(te);
				} else {
					if (GetLastError() == ERROR_NO_MORE_FILES)
						has_finished = true;
					else
						THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "Thread32First");
				}
			}
			while (!has_finished) {
				THREADENTRY32 te;
				memset(&te, 0, sizeof(THREADENTRY32));
				te.dwSize = sizeof(THREADENTRY32);
				if (Thread32Next(hsnap, &te)) {
					if (te.th32OwnerProcessID == GetCurrentProcessId() && !this->set_thread_exempted.count(te.th32ThreadID)) vector_te->push_back(te);
				} else {
					if (GetLastError() == ERROR_NO_MORE_FILES)
						has_finished = true;
					else
						THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "Thread32First");
				}
			}
			return vector_te.release();
		}
	};

	DEFINE_PIMPL_DEFAULT_CTOR(SuspendOtherThreads);

	DEFINE_PIMPL_MOVE_CTOR(SuspendOtherThreads);

	DEFINE_PIMPL_DTOR(SuspendOtherThreads);

	bool SuspendOtherThreads::GetHasSuspended() const { return this->pimpl->GetHasSuspended(); }

	void SuspendOtherThreads::AddExemptedThread(_In_ DWORD id_thread) { return this->pimpl->AddExemptedThread(id_thread); }

	void SuspendOtherThreads::SuspendThreads() { return this->pimpl->SuspendThreads(); }

	void SuspendOtherThreads::ResumeThreads() { return this->pimpl->ResumeThreads(); }
}