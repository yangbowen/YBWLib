#pragma once
#ifndef _INCLUDED_YBWLIB_BINHOOK_H_
#define _INCLUDED_YBWLIB_BINHOOK_H_
#include <cstdint>
#include <sal.h>
#include <guiddef.h>
#include "..\exception\Exception.h"
#include "..\HookableProc\HookableProc.h"
#include "BinHookCommon.h"

namespace BinHook {
	struct DummyParam {};

	struct DummyReturn {};

	/// <summary>An object for binary hooking a function and wrapping it in a hookable function.</summary>
	/// <typeparam name="_Paramty">The type of the parameter of the function.</typeparam>
	/// <typeparam name="_Returnty">
	/// The type of the return value of the function.
	/// The destructor of the type passed to this parameter, if any, must be accessible to <c>HookableFunction</c>
	/// </typeparam>
	template <typename _Paramty, typename _Returnty>
	class HookableFunctionBinHook abstract {
	public:
		using ParamType = _Paramty;
		using ReturnType = _Returnty;
		typedef HookableProc::HookableFunction<_Paramty, _Returnty> HookableFunctionType;
		typedef HookableProc::HookableFunction<ParamType, ReturnType>::HookEntry<HookableFunctionBinHook<ParamType, ReturnType> > MainHookEntryType;
		/// <summary>Create a hookable function binary hook.</summary>
		/// <param name="guid_function">
		/// The GUID of the hookable function to be created.
		/// No other hookable functions or hookable procedures may have an identical GUID.
		/// </param>
		/// <param name="context_func">An optional context value associated with the function.</param>
		/// <param name="guid_hook_main">
		/// The GUID of the main hook entry.
		/// No other hook entries in the same hookable function may have an identical GUID.
		/// </param>
		explicit HookableFunctionBinHook(_In_ const GUID* guid_function, _Inout_opt_ void* context_func, _In_ const GUID* guid_hook_main)
			: guid_hook_main(*guid_hook_main) {
			if (!guid_function) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!guid_hook_main) THROW_INVALID_PARAMETER_EXCEPTION_THIS(2);
			MainHookEntryType entry_main(guid_hook_main, &HookableFunctionBinHook::HookFnMain, nullptr, this, nullptr);
			this->hookable_func = HookableFunctionType::CreateFunction(guid_function, context_func, &entry_main);
			MemoryBarrier();
			this->constructed = true;
		}
		/// <summary>Apply the binary hook.</summary>
		/// <param name="hook_target">
		/// The executable location at which to apply the binary hook.
		/// The caller must ensure that no code that jumps to or calls some location just after <paramref name="hook_target" />. (Jumps or calls exactly to <paramref name="hook_target" /> are fine)
		/// </param>
		/// <param name="suspend_other_threads">
		/// An optional pointer to a <c>SuspendOtherThreads</c> object that suspends other threads in order to avoid data races.
		/// If this pointer is not empty, this function will not call <c>ResumeThreads</c> or delete the object. It's the caller's responsiblity to resume the threads.
		/// If this pointer is empty, this function creates a <c>SuspendOtherThreads</c> object internally and will delete the object when it's done.
		/// Note that if there're multiple binary hooks to apply, it's recommended to create a common <c>SuspendOtherThreads</c> object and use it.
		/// </param>
		virtual void ApplyBinHook(_Inout_ void* hook_target, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
			if (this->has_applied) THROW_UNEXPECTED_ERROR_EXCEPTION();
			MemoryBarrier();
			this->ApplyRawBinHook(hook_target, suspend_other_threads);
			MemoryBarrier();
			this->has_applied = true;
		}
		virtual ~HookableFunctionBinHook() {
			HookableFunctionType::DeleteHook(this->hookable_func->GetGuid(), &guid_hook_main);
		}
	protected:
		bool constructed = false;
		bool has_applied = false;
		/// <summary>
		/// A pointer to the hookable function.
		/// This object does not own the object pointed to by this member variable.
		/// </summary>
		HookableFunctionType* hookable_func = nullptr;
		/// <summary>The GUID of the main hook.</summary>
		const GUID guid_hook_main = GUID_NULL;
		/// <summary>Apply the raw binary hook.</summary>
		/// <param name="hook_target">
		/// The executable location at which to apply the binary hook.
		/// The caller must ensure that no code that jumps to or calls some location just after <paramref name="hook_target" />. (Jumps or calls exactly to <paramref name="hook_target" /> are fine)
		/// </param>
		/// <param name="suspend_other_threads">
		/// An optional pointer to a <c>SuspendOtherThreads</c> object that suspends other threads in order to avoid data races.
		/// If this pointer is not empty, this function will not call <c>ResumeThreads</c> or delete the object. It's the caller's responsiblity to resume the threads.
		/// If this pointer is empty, this function creates a <c>SuspendOtherThreads</c> object internally and will delete the object when it's done.
		/// Note that if there're multiple binary hooks to apply, it's recommended to create a common <c>SuspendOtherThreads</c> object and use it.
		/// </param>
		virtual void ApplyRawBinHook(_Inout_ void* hook_target, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) = 0;
		/// <summary>Call the original function which served as the binary hook target.</summary>
		/// <param name="param">
		/// A pointer to the parameter of the target original function.
		/// </param>
		/// <returns>
		/// A pointer to the return of the target original function.
		/// The caller will delete the returned object when it's no longer useful.
		/// </returns>
		virtual ReturnType* CallTargetOriginal(const ParamType* param) const = 0;
		HookableProc::HookRet __thiscall HookFnMain(_In_ typename HookableFunctionType::InvocationPacket* packet, _Inout_opt_ void*, _Inout_ HookableFunctionType*, _Inout_ MainHookEntryType*) {
			if (!packet) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!this->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
			if (this->has_applied) {
				ParamType* param = packet->GetParam();
				if (!param) THROW_UNEXPECTED_ERROR_EXCEPTION();
				ReturnType* ret = this->CallTargetOriginal(param);
				packet->SetReturn(ret);
			}
			return HookableProc::HookRet_Continue;
		}
	};
}
#endif
