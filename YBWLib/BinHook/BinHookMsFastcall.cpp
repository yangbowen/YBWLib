#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "..\exception\Exception.h"
#include "BinHook.h"
#include "BinHookMsFastcall.h"

namespace BinHook {
#if defined (_M_IX86) || defined (__i386__)
	static_assert(sizeof(uintptr_t) == 4 && sizeof(uint32_t) == 4, L"The length of either uintptr_t or uint32_t is unexpected.");
	namespace Internal {
		static const uint8_t MsFastcallHookableFunctionBinHook_tmpl_raw_call_target_original[] = {
			// Stack:
			// EBP+0x10	_Out_ MsFastcallRawReturn* raw_ret
			// EBP+0xC	_In_ const MsFastcallRawParam* raw_param
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
			0x8B, 0x48, 0x0C,							// mov ecx,[eax+0xc]
			0x8B, 0x50, 0x10,							// mov edx,[eax+0x10]
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
		static const uint8_t MsFastcallHookableFunctionBinHook_tmpl_hook_code[] = {
			// Stack:
			//			----------------
			//			Parameters...
			// EBP+0x8	----------------
			// EBP+0x4	Return Address
			// EBP		Stored EBP
			//			----------------
			// EBP-0x4	MsFastcallRawParam.edx
			// EBP-0x8	MsFastcallRawParam.ecx
			// EBP-0xC	MsFastcallRawParam.stack_param
			// EBP-0x10	MsFastcallRawParam.count_stack_param
			// EBP-0x14	MsFastcallRawParam.flags
			//			----------------
			//			----------------
			// EBP-0x18	MsFastcallRawReturn.edx
			// EBP-0x1C	MsFastcallRawReturn.eax
			// EBP-0x20	MsFastcallRawReturn.flags
			//			----------------
			0x55,										// push ebp
			0x89, 0xE5,									// mov ebp,esp
			0x83, 0xEC, 0x20,							// sub esp,byte +0x20
			0x50,										// push eax
			0x51,										// push ecx
			0x52,										// push edx
			0x89, 0x4D, 0xF8,							// mov [ebp-0x8],ecx
			0x89, 0x55, 0xFC,							// mov [ebp-0x4],edx
			0xC7, 0x45, 0xEC, 0x00, 0x00, 0x00, 0x00,	// mov dword [ebp-0x14],imm32
			0xC7, 0x45, 0xF0,							// mov dword [ebp-0x10],imm32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x19: Placeholder for imm32
			0x8D, 0x45, 0x08,							// lea eax,[ebp+0x8]
			0x89, 0x45, 0xF4,							// mov [ebp-0xc],eax
			0x8D, 0x45, 0xE0,							// lea eax,[ebp-0x20]
			0x50,										// push eax
			0x8D, 0x45, 0xEC,							// lea eax,[ebp-0x14]
			0x50,										// push eax
			0x68, 										// push dword imm32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x2C: Placeholder for imm32
			0xE8, 										// call dword rel32
			0xCC, 0xCC, 0xCC, 0xCC,						// 0x31: Placeholder for rel32
			0x5A,										// 0x35: pop edx
			0x59,										// pop ecx
			0x58,										// pop eax
			0xF7, 0x45, 0xE0, 0x01, 0x00, 0x00, 0x00,	// test dword [ebp-0x20],0x1
			0x74, 0x03,									// jz .+0x3
			0x8B, 0x45, 0xE4,							// mov eax,[ebp-0x1c]
			0xF7, 0x45, 0xE0, 0x02, 0x00, 0x00, 0x00,	// test dword [ebp-0x20],0x2
			0x74, 0x03,									// jz .+0x3
			0x8B, 0x55, 0xE8,							// mov edx,[ebp-0x18]
			0x89, 0xEC,									// mov esp,ebp
			0x5D,										// pop ebp
			0xC2,										// ret imm16
			0xCC, 0xCC									// 0x54: Placeholder for imm16
		};
		static INIT_ONCE initonce_MsFastcallHookableFunctionBinHook = INIT_ONCE_STATIC_INIT;
		void(__stdcall * MsFastcallHookableFunctionBinHook_raw_call_target_original)(_In_ void* target_original, _In_ const MsFastcallRawParam* raw_param, _Out_ MsFastcallRawReturn* raw_ret) = nullptr;

		void MsFastcallHookableFunctionBinHook_init() {
			if (!InitOnceExecuteOnce(&initonce_MsFastcallHookableFunctionBinHook,
				[](_Inout_ PINIT_ONCE, _Inout_opt_ PVOID, _Out_opt_ PVOID*)->BOOL {
					void* p_raw_call_target_original = (uint8_t*)VirtualAlloc(nullptr, sizeof(MsFastcallHookableFunctionBinHook_tmpl_raw_call_target_original), MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
					if (!p_raw_call_target_original) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "VirtualAlloc");
					memcpy(p_raw_call_target_original, MsFastcallHookableFunctionBinHook_tmpl_raw_call_target_original, sizeof(MsFastcallHookableFunctionBinHook_tmpl_raw_call_target_original));
					FlushInstructionCache(GetCurrentProcess(), p_raw_call_target_original, sizeof(MsFastcallHookableFunctionBinHook_tmpl_raw_call_target_original));
					MsFastcallHookableFunctionBinHook_raw_call_target_original = reinterpret_cast<void(__stdcall *)(_In_ void* target_original, _In_ const MsFastcallRawParam* raw_param, _Out_ MsFastcallRawReturn* raw_ret)>(p_raw_call_target_original);
					return TRUE;
				}, nullptr, nullptr)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "InitOnceExecuteOnce");
		}

		size_t MsFastcallHookableFunctionBinHook_get_len_hook_code(_In_opt_ size_t count_stack_param) {
			UNREFERENCED_PARAMETER(count_stack_param);
			return sizeof(MsFastcallHookableFunctionBinHook_tmpl_hook_code);
		}

		void MsFastcallHookableFunctionBinHook_make_hook_code(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code, _Inout_opt_ void* context, _Inout_opt_ SuspendOtherThreads* suspend_other_threads) {
			UNREFERENCED_PARAMETER(suspend_other_threads);
			if (!hook_code) THROW_INVALID_PARAMETER_EXCEPTION(0);
			if (!cb_hook_code || cb_hook_code != sizeof(MsFastcallHookableFunctionBinHook_tmpl_hook_code)) THROW_INVALID_PARAMETER_EXCEPTION(1);
			Internal::MsFastcallHookableFunctionBinHook_ctx_t* ctx = reinterpret_cast<Internal::MsFastcallHookableFunctionBinHook_ctx_t*>(context);
			if (!ctx) THROW_INVALID_PARAMETER_EXCEPTION(2);
			memcpy(hook_code, MsFastcallHookableFunctionBinHook_tmpl_hook_code, sizeof(MsFastcallHookableFunctionBinHook_tmpl_hook_code));
			*reinterpret_cast<size_t*>(hook_code + 0x19) = ctx->count_stack_param_raw;
			*reinterpret_cast<void**>(hook_code + 0x2C) = ctx->pobj_hookable_func;
			*reinterpret_cast<uintptr_t*>(hook_code + 0x31) = reinterpret_cast<uintptr_t>(ctx->p_invoke_function) - reinterpret_cast<uintptr_t>(hook_code + 0x35);
			if (ctx->count_stack_param_raw * sizeof(uintptr_t) > UINT16_MAX) THROW_UNEXPECTED_ERROR_EXCEPTION();
			*reinterpret_cast<uint16_t*>(hook_code + 0x54) = (ctx->count_stack_param_raw * sizeof(uintptr_t)) & (~(uint16_t)0);
			ctx->target_original = reinterpret_cast<void*>(hook_code + cb_hook_code);
		}
	}
#endif
}
