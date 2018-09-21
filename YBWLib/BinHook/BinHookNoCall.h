#pragma once
#ifndef _INCLUDED_YBWLIB_BINHOOKNOCALL_H_
#define _INCLUDED_YBWLIB_BINHOOKNOCALL_H_
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

	/// <summary>The raw parameter.</summary>
	struct YBWLIB_API NoCallRawParam {
		__pragma(pack(push, 1));
		uintptr_t edi;
		uintptr_t esi;
		uintptr_t ebp;
		uintptr_t esp;
		uintptr_t ebx;
		uintptr_t edx;
		uintptr_t ecx;
		uintptr_t eax;
		NoCallRawParam() {
			this->edi = 0;
			this->esi = 0;
			this->ebp = 0;
			this->esp = 0;
			this->ebx = 0;
			this->edx = 0;
			this->ecx = 0;
			this->eax = 0;
		}
		NoCallRawParam(const NoCallRawParam& t)
			: edi(t.edi),
			esi(t.esi),
			ebp(t.ebp),
			esp(t.esp),
			ebx(t.ebx),
			edx(t.edx),
			ecx(t.ecx),
			eax(t.eax) {}
		NoCallRawParam(NoCallRawParam&& t)
			: edi(t.edi),
			esi(t.esi),
			ebp(t.ebp),
			esp(t.esp),
			ebx(t.ebx),
			edx(t.edx),
			ecx(t.ecx),
			eax(t.eax) {}
		NoCallRawParam(const DummyParam&) : NoCallRawParam() {}
		~NoCallRawParam() {}
		operator DummyParam() const {
			return DummyParam();
		}
		__pragma(pack(pop));
	};

	namespace Internal {
		struct NoCallHookableProcedureBinHook_ctx_t {
			void* pobj_hookable_proc = nullptr;
			void(__stdcall* p_invoke_procedure)(_Inout_ void* pobj_hookable_proc, _In_ NoCallRawParam* raw_param);
		};
		YBWLIB_API void NoCallHookableProcedureBinHook_init();
		YBWLIB_API size_t NoCallHookableProcedureBinHook_get_len_hook_code();
		YBWLIB_API void NoCallHookableProcedureBinHook_make_hook_code(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads);
	}

	/// <summary>An object for binary hooking some code and calling a hookable procedure when the code is executed.</summary>
	/// <typeparam name="_Paramty">
	/// The type of the parameter of the procedure.
	/// Must be implicitly convertible from and to <c>NoCallRawParam</c>.
	/// </typeparam>
	template <typename _Paramty>
	class NoCallHookableProcedureBinHook : public HookableProcedureBinHook<_Paramty> {
	public:
		using ParamType = _Paramty;
		typedef HookableProc::PureHookableProcedure<ParamType> HookableProcedureType;
		/// <summary>Create a hookable procedure binary hook.</summary>
		/// <param name="guid_procedure">
		/// The GUID of the hookable procedure to be created.
		/// No other hookable functions or hookable procedures may have an identical GUID.
		/// </param>
		/// <param name="context_proc">An optional context value associated with the procedure.</param>
		explicit NoCallHookableProcedureBinHook(_In_ const GUID* guid_procedure, _Inout_opt_ void* context_proc)
			: HookableProcedureBinHook(guid_procedure, context_proc), ctx() {}
		virtual ~NoCallHookableProcedureBinHook() {}
	protected:
		virtual void ApplyRawBinHook(_Inout_ void* hook_target, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
			this->ctx.pobj_hookable_proc = reinterpret_cast<void*>(this->hookable_proc);
			this->ctx.p_invoke_procedure = &this->RawInvokeProcedure;
			Raw::ApplyHook(
				hook_target,
				Internal::NoCallHookableProcedureBinHook_get_len_hook_code(),
				Internal::NoCallHookableProcedureBinHook_make_hook_code,
				reinterpret_cast<void*>(&this->ctx),
				suspend_other_threads
			);
		}
	private:
		static void __stdcall RawInvokeProcedure(_Inout_ void* pobj_hookable_proc, _In_ NoCallRawParam* raw_param) {
			try {
				HookableProcedureType* hookable_proc = reinterpret_cast<HookableProcedureType*>(pobj_hookable_proc);
				if (!hookable_proc) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(NoCallHookableProcedureBinHook, 0);
				if (!raw_param) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(NoCallHookableProcedureBinHook, 1);
				ParamType param(*raw_param);
				hookable_proc->Invoke(&param);
				raw_param.~NoCallRawParam();
				new(raw_param) NoCallRawParam(param);
			} catch (Exception::BaseException& err) {
				display_exception(&err);
				throw;
			}
		}
		Internal::NoCallHookableProcedureBinHook_ctx_t ctx;
	};
#endif
}
#endif
