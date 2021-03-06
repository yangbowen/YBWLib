#pragma once
#ifndef _INCLUDED_YBWLIB_BINHOOKSTDCALL_H_
#define _INCLUDED_YBWLIB_BINHOOKSTDCALL_H_
#include <cstdint>
#include <sal.h>
#include <guiddef.h>
#include "..\exception\Exception.h"
#include "..\exception\exception_helper.h"
#include "..\HookableProc\HookableProc.h"
#include "BinHookRaw.h"
#include "BinHook.h"

namespace BinHook {
#if defined (_M_IX86) || defined (__i386__)
	static_assert(sizeof(uintptr_t) == 4 && sizeof(uint32_t) == 4, L"The length of either uintptr_t or uint32_t is unexpected.");

	/// <summary>The raw parameter of an <c>__stdcall</c> call.</summary>
	struct YBWLIB_API StdcallRawParam {
		__pragma(pack(push, 1));
		enum Flags {
			Flag_None = 0x0,
			Flag_OwnParam = 0x1
		} flags;
		static_assert(sizeof(Flags) == sizeof(uintptr_t), L"The length of StdcallRawParam::Flags is different from the length of uintptr_t.");
		size_t count_param = 0;
		_Field_size_(count_param) uintptr_t* param = nullptr;
		StdcallRawParam() {
			this->flags = Flag_OwnParam;
			this->count_param = 0;
			this->param = new uintptr_t[1];
		}
		StdcallRawParam(const StdcallRawParam& t)
			: flags(Flag_OwnParam), count_param(t.count_param), param(new uintptr_t[t.count_param ? t.count_param : 1]) {
			if (t.count_param) memcpy(this->param, t.param, t.count_param * sizeof(uintptr_t)); else this->param[0] = 0xCC;
		}
		StdcallRawParam(StdcallRawParam&& t)
			: flags(t.flags), count_param(t.count_param), param(t.param) {
			t.flags = Flag_None;
			t.count_param = 0;
			t.param = nullptr;
		}
		StdcallRawParam(const DummyParam&) : StdcallRawParam() {}
		~StdcallRawParam() {
			if ((this->flags & Flag_OwnParam) && this->param) {
				*reinterpret_cast<uintptr_t*>(&this->flags) &= ~Flag_OwnParam;
				delete[] this->param;
				this->param = nullptr;
			}
		}
		operator DummyParam() const {
			return DummyParam();
		}
		__pragma(pack(pop));
	};

	/// <summary>The raw return value of an <c>__stdcall</c> call.</summary>
	struct YBWLIB_API StdcallRawReturn {
		__pragma(pack(push, 1));
		enum Flags {
			Flag_None = 0x0,
			Flag_SetEax = 0x1,
			Flag_SetEdx = 0x2
		} flags;
		static_assert(sizeof(Flags) == sizeof(uintptr_t), L"The length of StdcallRawReturn::Flags is different from the length of uintptr_t.");
		uintptr_t eax;
		uintptr_t edx;
		StdcallRawReturn() {
			this->flags = Flag_None;
			this->eax = 0;
			this->edx = 0;
		}
		StdcallRawReturn(const StdcallRawReturn& t)
			: flags(t.flags), eax(t.eax), edx(t.edx) {}
		StdcallRawReturn(StdcallRawReturn&& t)
			: flags(t.flags), eax(t.eax), edx(t.edx) {}
		StdcallRawReturn(const DummyReturn&) : StdcallRawReturn() {}
		StdcallRawReturn(uint32_t val) {
			this->flags = Flag_SetEax;
			this->eax = val;
		}
		StdcallRawReturn(uint64_t val) {
			this->flags = (Flags)(Flag_SetEax | Flag_SetEdx);
			this->eax = val & ~(uintptr_t)0;
			this->edx = (val >> 0x20) & ~(uintptr_t)0;
		}
		StdcallRawReturn(int32_t val) : StdcallRawReturn((uint32_t)val) {}
		StdcallRawReturn(int64_t val) : StdcallRawReturn((uint64_t)val) {}
		StdcallRawReturn(void* val) : StdcallRawReturn(reinterpret_cast<uint32_t>(val)) {}
		~StdcallRawReturn() {
			this->flags = Flag_None;
			this->eax = 0;
			this->edx = 0;
		}
		operator DummyReturn() const {
			return DummyReturn();
		}
		operator uint32_t() const {
			if (!(this->flags & Flag_SetEax)) THROW_UNEXPECTED_ERROR_EXCEPTION();
			return this->eax;
		}
		operator uint64_t() const {
			if (!(this->flags & (Flag_SetEax | Flag_SetEdx))) THROW_UNEXPECTED_ERROR_EXCEPTION();
			return ((uint64_t)this->edx << 0x20) | this->eax;
		}
		operator int32_t() const { return (int32_t)static_cast<uint32_t>(*this); }
		operator int64_t() const { return (int64_t)static_cast<uint64_t>(*this); }
		operator void*() const { return reinterpret_cast<void*>(static_cast<uintptr_t>(*this)); }
		__pragma(pack(pop));
	};

	namespace Internal {
		struct StdcallHookableFunctionBinHook_ctx_t {
			size_t count_param_raw = 0;
			void* pobj_hookable_func = nullptr;
			void(__stdcall* p_invoke_function)(_Inout_ void* pobj_hookable_func, _In_ const StdcallRawParam* raw_param, _Out_ StdcallRawReturn* raw_ret);
			void* target_original = nullptr;
		};
		YBWLIB_API extern void(__stdcall * StdcallHookableFunctionBinHook_raw_call_target_original)(_In_ void* target_original, _In_ const StdcallRawParam* raw_param, _Out_ StdcallRawReturn* raw_ret);
		YBWLIB_API void StdcallHookableFunctionBinHook_init();
		YBWLIB_API size_t StdcallHookableFunctionBinHook_get_len_hook_code(_In_opt_ size_t count_param);
		YBWLIB_API void StdcallHookableFunctionBinHook_make_hook_code(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads);
	}

	/// <summary>An object for binary hooking a function whose calling convention is <c>__stdcall</c> and wrapping it in a hookable function.</summary>
	/// <typeparam name="_Paramty">
	/// The type of the parameter of the function.
	/// Must be implicitly convertible from and to <c>StdcallRawParam</c>.
	/// </typeparam>
	/// <typeparam name="_Returnty">
	/// The type of the return value of the function.
	/// The destructor of the type passed to this parameter, if any, must be accessible to <c>HookableFunction</c>
	/// Must be implicitly convertible from and to <c>StdcallRawReturn</c>.
	/// </typeparam>
	template <typename _Paramty, typename _Returnty>
	class StdcallHookableFunctionBinHook : public HookableFunctionBinHook<_Paramty, _Returnty> {
	public:
		using ParamType = _Paramty;
		using ReturnType = _Returnty;
		typedef HookableProc::HookableFunction<ParamType, ReturnType> HookableFunctionType;
		/// <summary>Create a hookable function binary hook that hooks a <c>__stdcall</c> function.</summary>
		/// <param name="guid_function">
		/// The GUID of the hookable function to be created.
		/// No other hookable functions or hookable procedures may have an identical GUID.
		/// </param>
		/// <param name="context_func">An optional context value associated with the function.</param>
		/// <param name="guid_hook_main">
		/// The GUID of the main hook entry.
		/// No other hook entries in the same hookable function may have an identical GUID.
		/// </param>
		/// <param name="count_param_raw">
		/// The count of the raw parameters.
		/// Each raw parameter is treated as a <c>uintptr_t</c>.
		/// </param>
		explicit StdcallHookableFunctionBinHook(_In_ const GUID* guid_function, _Inout_opt_ void* context_func, _In_ const GUID* guid_hook_main, _In_opt_ size_t count_param_raw)
			: count_param_raw(count_param_raw), HookableFunctionBinHook(guid_function, context_func, guid_hook_main), ctx() {}
		virtual ~StdcallHookableFunctionBinHook() {}
	protected:
		virtual void ApplyRawBinHook(_Inout_ void* hook_target, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
			this->ctx.count_param_raw = this->count_param_raw;
			this->ctx.pobj_hookable_func = reinterpret_cast<void*>(this->hookable_func);
			this->ctx.p_invoke_function = &this->RawInvokeFunction;
			Raw::ApplyHook(
				hook_target,
				Internal::StdcallHookableFunctionBinHook_get_len_hook_code(this->count_param_raw),
				Internal::StdcallHookableFunctionBinHook_make_hook_code,
				reinterpret_cast<void*>(&this->ctx),
				suspend_other_threads
			);
		}
		virtual ReturnType* CallTargetOriginal(const ParamType* param) const {
			if (!param) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			StdcallRawParam raw_param(*param);
			if (raw_param.count_param != this->count_param_raw) THROW_UNEXPECTED_ERROR_EXCEPTION();
			Internal::StdcallHookableFunctionBinHook_init();
			StdcallRawReturn raw_ret;
			Internal::StdcallHookableFunctionBinHook_raw_call_target_original(this->ctx.target_original, &raw_param, &raw_ret);
			ReturnType* ret = new ReturnType(raw_ret);
			return ret;
		}
	private:
		static void __stdcall RawInvokeFunction(_Inout_ void* pobj_hookable_func, _In_ const StdcallRawParam* raw_param, _Out_ StdcallRawReturn* raw_ret) {
			try {
				HookableFunctionType* hookable_func = reinterpret_cast<HookableFunctionType*>(pobj_hookable_func);
				if (!hookable_func) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(StdcallHookableFunctionBinHook, 0);
				if (!raw_param) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(StdcallHookableFunctionBinHook, 1);
				if (!raw_ret) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(StdcallHookableFunctionBinHook, 2);
				ParamType param(*raw_param);
				ReturnType* ret = hookable_func->Invoke(&param);
				if (!ret) THROW_UNEXPECTED_ERROR_EXCEPTION();
				new(raw_ret) StdcallRawReturn(*ret);
			} catch (Exception::BaseException& err) {
				display_exception(&err);
				throw;
			}
		}
		const size_t count_param_raw;
		Internal::StdcallHookableFunctionBinHook_ctx_t ctx;
	};
#endif
}
#endif
