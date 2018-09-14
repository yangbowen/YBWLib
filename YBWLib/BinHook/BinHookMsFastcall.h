#pragma once
#ifndef _INCLUDED_YBWLIB_BINHOOKFASTCALL_H_
#define _INCLUDED_YBWLIB_BINHOOKFASTCALL_H_
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

	/// <summary>The raw parameter of a Microsoft <c>__fastcall</c> call.</summary>
	struct YBWLIB_API MsFastcallRawParam {
		__pragma(pack(push, 1));
		enum Flags {
			Flag_None = 0x0,
			Flag_OwnParam = 0x1
		} flags;
		static_assert(sizeof(Flags) == sizeof(uintptr_t), L"The length of MsFastcallRawParam::Flags is different from the length of uintptr_t.");
		size_t count_stack_param = 0;
		_Field_size_(count_stack_param) uintptr_t* stack_param = nullptr;
		uintptr_t ecx;
		uintptr_t edx;
		MsFastcallRawParam() {
			this->flags = Flag_OwnParam;
			this->count_stack_param = 0;
			this->stack_param = new uintptr_t[1];
			this->ecx = 0;
			this->edx = 0;
		}
		MsFastcallRawParam(const MsFastcallRawParam& t)
			: flags(Flag_OwnParam), count_stack_param(t.count_stack_param), stack_param(new uintptr_t[t.count_stack_param ? t.count_stack_param : 1]), ecx(t.ecx), edx(t.edx) {
			if (t.count_stack_param) memcpy(this->stack_param, t.stack_param, t.count_stack_param * sizeof(uintptr_t)); else this->stack_param[0] = 0xCC;
		}
		MsFastcallRawParam(MsFastcallRawParam&& t)
			: flags(t.flags), count_stack_param(t.count_stack_param), stack_param(t.stack_param), ecx(t.ecx), edx(t.edx) {
			t.flags = Flag_None;
			t.count_stack_param = 0;
			t.stack_param = nullptr;
			t.ecx = 0;
			t.edx = 0;
		}
		MsFastcallRawParam(const DummyParam&) : MsFastcallRawParam() {}
		~MsFastcallRawParam() {
			if ((this->flags & Flag_OwnParam) && this->stack_param) {
				*reinterpret_cast<uintptr_t*>(&this->flags) &= ~Flag_OwnParam;
				delete[] this->stack_param;
				this->stack_param = nullptr;
			}
		}
		operator DummyParam() const {
			return DummyParam();
		}
		__pragma(pack(pop));
	};

	/// <summary>The raw return value of a Microsoft <c>__fastcall</c> call.</summary>
	struct YBWLIB_API MsFastcallRawReturn {
		__pragma(pack(push, 1));
		enum Flags {
			Flag_None = 0x0,
			Flag_SetEax = 0x1,
			Flag_SetEdx = 0x2
		} flags;
		static_assert(sizeof(Flags) == sizeof(uintptr_t), L"The length of MsFastcallRawReturn::Flags is different from the length of uintptr_t.");
		uintptr_t eax;
		uintptr_t edx;
		MsFastcallRawReturn() {
			this->flags = Flag_None;
			this->eax = 0;
			this->edx = 0;
		}
		MsFastcallRawReturn(const MsFastcallRawReturn& t)
			: flags(t.flags), eax(t.eax), edx(t.edx) {}
		MsFastcallRawReturn(MsFastcallRawReturn&& t)
			: flags(t.flags), eax(t.eax), edx(t.edx) {}
		MsFastcallRawReturn(const DummyReturn&) : MsFastcallRawReturn() {}
		MsFastcallRawReturn(uint32_t val) {
			this->flags = Flag_SetEax;
			this->eax = val;
		}
		MsFastcallRawReturn(uint64_t val) {
			this->flags = (Flags)(Flag_SetEax | Flag_SetEdx);
			this->eax = val & ~(uintptr_t)0;
			this->edx = (val >> 0x20) & ~(uintptr_t)0;
		}
		MsFastcallRawReturn(int32_t val) : MsFastcallRawReturn((uint32_t)val) {}
		MsFastcallRawReturn(int64_t val) : MsFastcallRawReturn((uint64_t)val) {}
		MsFastcallRawReturn(void* val) : MsFastcallRawReturn(reinterpret_cast<uint32_t>(val)) {}
		~MsFastcallRawReturn() {
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
		struct MsFastcallHookableFunctionBinHook_ctx_t {
			size_t count_stack_param_raw = 0;
			void* pobj_hookable_func = nullptr;
			void(__stdcall* p_invoke_function)(_Inout_ void* pobj_hookable_func, _In_ const MsFastcallRawParam* raw_param, _Out_ MsFastcallRawReturn* raw_ret);
			void* target_original = nullptr;
		};
		YBWLIB_API extern void(__stdcall * MsFastcallHookableFunctionBinHook_raw_call_target_original)(_In_ void* target_original, _In_ const MsFastcallRawParam* raw_param, _Out_ MsFastcallRawReturn* raw_ret);
		YBWLIB_API void MsFastcallHookableFunctionBinHook_init();
		YBWLIB_API size_t MsFastcallHookableFunctionBinHook_get_len_hook_code(_In_opt_ size_t count_stack_param);
		YBWLIB_API void MsFastcallHookableFunctionBinHook_make_hook_code(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads);
	}

	/// <summary>An object for binary hooking a function whose calling convention is Microsoft <c>__fastcall</c> and wrapping it in a hookable function.</summary>
	/// <typeparam name="_Paramty">
	/// The type of the parameter of the function.
	/// Must be implicitly convertible from and to <c>MsFastcallRawParam</c>.
	/// </typeparam>
	/// <typeparam name="_Returnty">
	/// The type of the return value of the function.
	/// The destructor of the type passed to this parameter, if any, must be accessible to <c>HookableFunction</c>
	/// Must be implicitly convertible from and to <c>MsFastcallRawReturn</c>.
	/// </typeparam>
	template <typename _Paramty, typename _Returnty>
	class MsFastcallHookableFunctionBinHook : public HookableFunctionBinHook<_Paramty, _Returnty> {
	public:
		using ParamType = _Paramty;
		using ReturnType = _Returnty;
		typedef HookableProc::HookableFunction<ParamType, ReturnType> HookableFunctionType;
		/// <summary>Create a hookable function binary hook that hooks a Microsoft <c>__fastcall</c> function.</summary>
		/// <param name="guid_function">
		/// The GUID of the hookable function to be created.
		/// No other hookable functions or hookable procedures may have an identical GUID.
		/// </param>
		/// <param name="context_func">An optional context value associated with the function.</param>
		/// <param name="guid_hook_main">
		/// The GUID of the main hook entry.
		/// No other hook entries in the same hookable function may have an identical GUID.
		/// </param>
		/// <param name="count_stack_param_raw">
		/// The count of the raw stack parameters.
		/// Each raw parameter is treated as a <c>uintptr_t</c>.
		/// </param>
		explicit MsFastcallHookableFunctionBinHook(_In_ const GUID* guid_function, _Inout_opt_ void* context_func, _In_ const GUID* guid_hook_main, _In_opt_ size_t count_stack_param_raw)
			: count_stack_param_raw(count_stack_param_raw), HookableFunctionBinHook(guid_function, context_func, guid_hook_main), ctx() {}
		virtual ~MsFastcallHookableFunctionBinHook() {}
	protected:
		virtual void ApplyRawBinHook(_Inout_ void* hook_target, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
			this->ctx.count_stack_param_raw = this->count_stack_param_raw;
			this->ctx.pobj_hookable_func = reinterpret_cast<void*>(this->hookable_func);
			this->ctx.p_invoke_function = &this->RawInvokeFunction;
			Raw::ApplyHook(
				hook_target,
				Internal::MsFastcallHookableFunctionBinHook_get_len_hook_code(this->count_stack_param_raw),
				Internal::MsFastcallHookableFunctionBinHook_make_hook_code,
				reinterpret_cast<void*>(&this->ctx),
				suspend_other_threads
			);
		}
		virtual ReturnType* CallTargetOriginal(const ParamType* param) const {
			if (!param) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			MsFastcallRawParam raw_param(*param);
			if (raw_param.count_stack_param != this->count_stack_param_raw) THROW_UNEXPECTED_ERROR_EXCEPTION();
			Internal::MsFastcallHookableFunctionBinHook_init();
			MsFastcallRawReturn raw_ret;
			Internal::MsFastcallHookableFunctionBinHook_raw_call_target_original(this->ctx.target_original, &raw_param, &raw_ret);
			ReturnType* ret = new ReturnType(raw_ret);
			return ret;
		}
	private:
		static void __stdcall RawInvokeFunction(_Inout_ void* pobj_hookable_func, _In_ const MsFastcallRawParam* raw_param, _Out_ MsFastcallRawReturn* raw_ret) {
			try {
				HookableFunctionType* hookable_func = reinterpret_cast<HookableFunctionType*>(pobj_hookable_func);
				if (!hookable_func) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(MsFastcallHookableFunctionBinHook, 0);
				if (!raw_param) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(MsFastcallHookableFunctionBinHook, 1);
				if (!raw_ret) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(MsFastcallHookableFunctionBinHook, 2);
				ParamType param(*raw_param);
				ReturnType* ret = hookable_func->Invoke(&param);
				if (!ret) THROW_UNEXPECTED_ERROR_EXCEPTION();
				new(raw_ret) MsFastcallRawReturn(*ret);
			} catch (Exception::BaseException& err) {
				display_exception(&err);
				throw;
			}
		}
		const size_t count_stack_param_raw;
		Internal::MsFastcallHookableFunctionBinHook_ctx_t ctx;
	};
#endif
}
#endif
