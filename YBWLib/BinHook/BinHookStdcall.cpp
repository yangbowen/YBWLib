#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "..\exception\Exception.h"
#include "BinHook.h"
#include "BinHookStdcall.h"

namespace BinHook {
#if defined (_M_IX86) || defined (__i386__)
	static_assert(sizeof(uintptr_t) == 4 && sizeof(uint32_t) == 4, L"The length of either uintptr_t or uint32_t is unexpected.");
	namespace Internal {
		static const uint8_t StdcallHookableFunctionBinHook_tmpl_raw_call_target_original[] = {
			// Stack:
			// EBP+0x10	_Out_ StdcallRawReturn* raw_ret
			// EBP+0xC	_In_ const StdcallRawParam* raw_param
			// EBP+0x8	_In_ void* target_original
			// EBP+0x4	Return Address
			// EBP		Stored EBP
			0x55,										// push ebp
			0x89, 0xE5,									// mov ebp,esp
			0x53,										// push ebx
			0x56,										// push esi
			0x57,										// push edi
			0x9C,										// pushfd
			0x8B, 0x45, 0x0C,							// mov eax,[ebp+0xc]
			0x8B, 0x70, 0x08,							// mov esi,[eax+0x8]
			0x8B, 0x48, 0x04,							// mov ecx,[eax+0x4]
			0xF7, 0xC1, 0x00, 0x00, 0x00, 0xC0,			// test ecx,0xc0000000
			0x74, 0x01,									// jz .+1
			0xCC,										// int3
			0xC1, 0xE1, 0x02,							// shl ecx,byte 0x2
			0x29, 0xCC,									// sub esp,ecx
			0x89, 0xE7,									// mov edi,esp
			0x8B, 0x48, 0x04,							// mov ecx,[eax+0x4]
			0xFC,										// cld
			0xF3, 0xA5,									// rep movsd
			0xFF, 0x55, 0x08,							// call dword [ebp+0x8]
			0x8B, 0x75, 0x10,							// mov esi,[ebp+0x10]
			0xC7, 0x06, 0x03, 0x00, 0x00, 0x00,			// mov dword [esi],0x3
			0x89, 0x46, 0x04,							// mov [esi+0x4],eax
			0x89, 0x56, 0x08,							// mov [esi+0x8],edx
			0x9D,										// popfd
			0x5F,										// pop edi
			0x5E,										// pop esi
			0x5B,										// pop ebx
			0x89, 0xEC,									// mov esp,ebp
			0x5D,										// pop ebp
			0xC2, 0x0C, 0x00							// ret 0xc
		};
		static const uint8_t StdcallHookableFunctionBinHook_tmpl_hook_code[] = {
			// Stack:
			//			----------------
			//			Parameters...
			// EBP+0x8	----------------
			// EBP+0x4	Return Address
			// EBP		Stored EBP
			//			----------------
			// EBP-0x4	StdcallRawParam.param
			// EBP-0x8	StdcallRawParam.count_param
			// EBP-0xC	StdcallRawParam.flags
			//			----------------
			//			----------------
			// EBP-0x10	StdcallRawReturn.edx
			// EBP-0x14	StdcallRawReturn.eax
			// EBP-0x18	StdcallRawReturn.flags
			//			----------------
			0x55,										// push ebp
			0x89, 0xE5,									// mov ebp,esp
			0x83, 0xEC, 0x18,							// sub esp,byte +0x18
			0x50,										// push eax
			0x51,										// push ecx
			0x52,										// push edx
			0xC7, 0x45, 0xF4, 0x00, 0x00, 0x00, 0x00,	// mov dword [ebp-0xc],imm32
			0xC7, 0x45, 0xF8,							// mov dword [ebp-0x8],imm32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x13: Placeholder for imm32
			0x8D, 0x45, 0x08,							// lea eax,[ebp+0x8]
			0x89, 0x45, 0xFC,							// mov [ebp-0x4],eax
			0x8D, 0x45, 0xE8,							// lea eax,[ebp-0x18]
			0x50,										// push eax
			0x8D, 0x45, 0xF4,							// lea eax,[ebp-0xc]
			0x50,										// push eax
			0x68, 										// push dword imm32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x26: Placeholder for imm32
			0xE8, 										// call dword rel32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x2B: Placeholder for rel32
			0x5A,										// 0x2F: pop edx
			0x59,										// pop ecx
			0x58,										// pop eax
			0xF7, 0x45, 0xE8, 0x01, 0x00, 0x00, 0x00,	// test dword [ebp-0x18],0x1
			0x74, 0x03,									// jz .+0x3
			0x8B, 0x45, 0xEC,							// mov eax,[ebp-0x14]
			0xF7, 0x45, 0xE8, 0x02, 0x00, 0x00, 0x00,	// test dword [ebp-0x18],0x2
			0x74, 0x03,									// jz .+0x3
			0x8B, 0x55, 0xF0,							// mov edx,[ebp-0x10]
			0x89, 0xEC,									// mov esp,ebp
			0x5D,										// pop ebp
			0xC2,										// ret imm16
			0xCC, 0xCC									// 0x4E: Placeholder for imm16
		};
		static INIT_ONCE initonce_StdcallHookableFunctionBinHook = INIT_ONCE_STATIC_INIT;
		void(__stdcall * StdcallHookableFunctionBinHook_raw_call_target_original)(_In_ void* target_original, _In_ const StdcallRawParam* raw_param, _Out_ StdcallRawReturn* raw_ret) = nullptr;

		void StdcallHookableFunctionBinHook_init() {
			if (!InitOnceExecuteOnce(&initonce_StdcallHookableFunctionBinHook,
				[](_Inout_ PINIT_ONCE, _Inout_opt_ PVOID, _Out_opt_ PVOID*)->BOOL {
					void* p_raw_call_target_original = (uint8_t*)VirtualAlloc(nullptr, sizeof(StdcallHookableFunctionBinHook_tmpl_raw_call_target_original), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
					if (!p_raw_call_target_original) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "VirtualAlloc");
					memcpy(p_raw_call_target_original, StdcallHookableFunctionBinHook_tmpl_raw_call_target_original, sizeof(StdcallHookableFunctionBinHook_tmpl_raw_call_target_original));
					FlushInstructionCache(GetCurrentProcess(), p_raw_call_target_original, sizeof(StdcallHookableFunctionBinHook_tmpl_raw_call_target_original));
					StdcallHookableFunctionBinHook_raw_call_target_original = reinterpret_cast<void(__stdcall *)(_In_ void* target_original, _In_ const StdcallRawParam* raw_param, _Out_ StdcallRawReturn* raw_ret)>(p_raw_call_target_original);
					return TRUE;
				}, nullptr, nullptr)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "InitOnceExecuteOnce");
		}

		size_t StdcallHookableFunctionBinHook_get_len_hook_code(_In_opt_ size_t count_param) {
			UNREFERENCED_PARAMETER(count_param);
			return sizeof(StdcallHookableFunctionBinHook_tmpl_hook_code);
		}

		void StdcallHookableFunctionBinHook_make_hook_code(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
			UNREFERENCED_PARAMETER(suspend_other_threads);
			if (!hook_code) THROW_INVALID_PARAMETER_EXCEPTION(0);
			if (!cb_hook_code || cb_hook_code != sizeof(StdcallHookableFunctionBinHook_tmpl_hook_code)) THROW_INVALID_PARAMETER_EXCEPTION(1);
			Internal::StdcallHookableFunctionBinHook_ctx_t* ctx = reinterpret_cast<Internal::StdcallHookableFunctionBinHook_ctx_t*>(context);
			if (!ctx) THROW_INVALID_PARAMETER_EXCEPTION(2);
			memcpy(hook_code, StdcallHookableFunctionBinHook_tmpl_hook_code, sizeof(StdcallHookableFunctionBinHook_tmpl_hook_code));
			*reinterpret_cast<size_t*>(hook_code + 0x13) = ctx->count_param_raw;
			*reinterpret_cast<void**>(hook_code + 0x26) = ctx->pobj_hookable_func;
			*reinterpret_cast<uintptr_t*>(hook_code + 0x2B) = reinterpret_cast<uintptr_t>(ctx->p_invoke_function) - reinterpret_cast<uintptr_t>(hook_code + 0x2F);
			if (ctx->count_param_raw * sizeof(uintptr_t) > UINT16_MAX) THROW_UNEXPECTED_ERROR_EXCEPTION();
			*reinterpret_cast<uint16_t*>(hook_code + 0x4E) = (ctx->count_param_raw * sizeof(uintptr_t)) & (~(uint16_t)0);
			ctx->target_original = reinterpret_cast<void*>(hook_code + cb_hook_code);
		}
	}
#endif
}
