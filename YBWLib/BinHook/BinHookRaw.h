#pragma once
#include <minwindef.h>
#include "..\YBWLib.h"
#include "..\exception\Exception.h"
#include "BinHookCommon.h"

namespace BinHook::Raw {
	namespace Internal {
		YBWLIB_API uint8_t* ApplyHookPrepareMakeHookCode(_Inout_ uint8_t* hook_target, _In_ size_t cb_hook_code);
		YBWLIB_API void ApplyHookFinishMakeHookCode(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code);
	}

	/// <summary>Apply the binary hook.</summary>
	/// <typeparam name="_FnMakeHookCode">
	/// The type of the callback function that makes the hook code.
	/// Call signature: <c>void fn_make_hook_code(_Inout_ void* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads);</c>
	/// </typeparam>
	/// <param name="hook_target">
	/// The executable location at which to apply the binary hook.
	/// The caller must ensure that no code that jumps to or calls some location just after <paramref name="hook_target" />. (Jumps or calls exactly to <paramref name="hook_target" /> are fine)
	/// </param>
	/// <param name="cb_hook_code">The length, in bytes, of the hook code.</param>
	/// <param name="fn_make_hook_code">
	/// A callback function supplied by the caller that makes the hook code.
	/// Call signature: <c>void fn_make_hook_code(_Inout_ void* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads);</c>
	/// </param>
	/// <param name="context">An optional context value passed to the hook code making function.</param>
	/// <param name="suspend_other_threads">
	/// An optional pointer to a <c>SuspendOtherThreads</c> object that suspends other threads in order to avoid data races.
	/// If this pointer is not empty, this function will not call <c>ResumeThreads</c> or delete the object. It's the caller's responsiblity to resume the threads.
	/// If this pointer is empty, this function creates a <c>SuspendOtherThreads</c> object internally and will delete the object when it's done.
	/// Note that if there're multiple binary hooks to apply, it's recommended to create a common <c>SuspendOtherThreads</c> object and use it.
	/// </param>
	/// <returns>A pointer to the hook code.</returns>
	template <typename _FnMakeHookCode>
	FORCEINLINE void* ApplyHook(_Inout_ void* hook_target, _In_ size_t cb_hook_code, _In_ _FnMakeHookCode fn_make_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
		if (!hook_target) THROW_INVALID_PARAMETER_EXCEPTION(0);
		if (!cb_hook_code) THROW_INVALID_PARAMETER_EXCEPTION(1);
		SuspendOtherThreads suspend_other_threads_internal;
		if (!suspend_other_threads) {
			suspend_other_threads = &suspend_other_threads_internal;
			suspend_other_threads->SuspendThreads();
		}
		void* hook_code = reinterpret_cast<void*>(Internal::ApplyHookPrepareMakeHookCode(reinterpret_cast<uint8_t*>(hook_target), cb_hook_code));
		if (!hook_code) THROW_UNEXPECTED_ERROR_EXCEPTION();
		fn_make_hook_code(reinterpret_cast<uint8_t*>(hook_code), cb_hook_code, context, suspend_other_threads);
		Internal::ApplyHookFinishMakeHookCode(reinterpret_cast<uint8_t*>(hook_code), cb_hook_code);
		return hook_code;
	}

	/// <summary>Apply the binary hook.</summary>
	/// <param name="hook_target">
	/// The executable location at which to apply the binary hook.
	/// The caller must ensure that no code that jumps to or calls some location just after <paramref name="hook_target" />. (Jumps or calls exactly to <paramref name="hook_target" /> are fine)
	/// </param>
	/// <param name="cb_hook_code">The length, in bytes, of the hook code.</param>
	/// <param name="hook_code_src">The hook code to be copied from.</param>
	/// <param name="suspend_other_threads">
	/// An optional pointer to a <c>SuspendOtherThreads</c> object that suspends other threads in order to avoid data races.
	/// If this pointer is not empty, this function will not call <c>ResumeThreads</c> or delete the object. It's the caller's responsiblity to resume the threads.
	/// If this pointer is empty, this function creates a <c>SuspendOtherThreads</c> object internally and will delete the object when it's done.
	/// Note that if there're multiple binary hooks to apply, it's recommended to create a common <c>SuspendOtherThreads</c> object and use it.
	/// </param>
	/// <returns>A pointer to the hook code.</returns>
	FORCEINLINE void* ApplyHook(_Inout_ void* hook_target, _In_ size_t cb_hook_code, const void* hook_code_src, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
		if (!hook_target) THROW_INVALID_PARAMETER_EXCEPTION(0);
		if (!cb_hook_code) THROW_INVALID_PARAMETER_EXCEPTION(1);
		if (!hook_code_src) THROW_INVALID_PARAMETER_EXCEPTION(2);
		SuspendOtherThreads suspend_other_threads_internal;
		if (!suspend_other_threads) {
			suspend_other_threads = &suspend_other_threads_internal;
			suspend_other_threads->SuspendThreads();
		}
		void* hook_code = reinterpret_cast<void*>(Internal::ApplyHookPrepareMakeHookCode(reinterpret_cast<uint8_t*>(hook_target), cb_hook_code));
		if (!hook_code) THROW_UNEXPECTED_ERROR_EXCEPTION();
		memcpy(hook_code, hook_code_src, cb_hook_code);
		Internal::ApplyHookFinishMakeHookCode(reinterpret_cast<uint8_t*>(hook_code), cb_hook_code);
		return hook_code;
	}
}