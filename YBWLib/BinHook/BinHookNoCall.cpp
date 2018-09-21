#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "..\exception\Exception.h"
#include "BinHook.h"
#include "BinHookNoCall.h"

namespace BinHook {
#if defined (_M_IX86) || defined (__i386__)
	static_assert(sizeof(uintptr_t) == 4 && sizeof(uint32_t) == 4, L"The length of either uintptr_t or uint32_t is unexpected.");
	namespace Internal {
		static const uint8_t NoCallHookableProcedureBinHook_tmpl_hook_code[] = {
			// Stack:
			//			----------------
			//			NoCallRawParam.eax
			//			NoCallRawParam.ecx
			//			NoCallRawParam.edx
			//			NoCallRawParam.ebx
			//			NoCallRawParam.esp
			//			NoCallRawParam.ebp
			//			NoCallRawParam.esi
			//			NoCallRawParam.edi
			//			----------------
			0x60,										// pushad
			0x54,										// push esp
			0x68, 										// push dword imm32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x3: Placeholder for imm32
			0xE8, 										// call dword rel32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x8: Placeholder for rel32
			0x61,										// 0xC: popad
		};
		static INIT_ONCE initonce_NoCallHookableProcedureBinHook = INIT_ONCE_STATIC_INIT;

		void NoCallHookableProcedureBinHook_init() {
			if (!InitOnceExecuteOnce(&initonce_NoCallHookableProcedureBinHook,
				[](_Inout_ PINIT_ONCE, _Inout_opt_ PVOID, _Out_opt_ PVOID*)->BOOL {
					return TRUE;
				}, nullptr, nullptr)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "InitOnceExecuteOnce");
		}

		size_t NoCallHookableProcedureBinHook_get_len_hook_code() {
			return sizeof(NoCallHookableProcedureBinHook_tmpl_hook_code);
		}

		void NoCallHookableProcedureBinHook_make_hook_code(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
			UNREFERENCED_PARAMETER(suspend_other_threads);
			if (!hook_code) THROW_INVALID_PARAMETER_EXCEPTION(0);
			if (!cb_hook_code || cb_hook_code != sizeof(NoCallHookableProcedureBinHook_tmpl_hook_code)) THROW_INVALID_PARAMETER_EXCEPTION(1);
			Internal::NoCallHookableProcedureBinHook_ctx_t* ctx = reinterpret_cast<Internal::NoCallHookableProcedureBinHook_ctx_t*>(context);
			if (!ctx) THROW_INVALID_PARAMETER_EXCEPTION(2);
			memcpy(hook_code, NoCallHookableProcedureBinHook_tmpl_hook_code, sizeof(NoCallHookableProcedureBinHook_tmpl_hook_code));
			*reinterpret_cast<void**>(hook_code + 0x3) = ctx->pobj_hookable_proc;
			*reinterpret_cast<uintptr_t*>(hook_code + 0x8) = reinterpret_cast<uintptr_t>(ctx->p_invoke_procedure) - reinterpret_cast<uintptr_t>(hook_code + 0xC);
		}
	}
#endif
}
