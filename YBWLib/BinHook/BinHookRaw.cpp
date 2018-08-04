#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <cstdint>
#include "..\exception\Exception.h"
#include "LDasm.h"
#include "BinHookRaw.h"

namespace BinHook::Raw {
	namespace Internal {
#if defined (_M_IX86) || defined (__i386__)
		uint8_t* ApplyHookPrepareMakeHookCode(_Inout_ uint8_t* hook_target, _In_ size_t cb_hook_code) {
			size_t cb_orig_code_copy;
			ldasm_data ldasm_data_orig_code_copy;
			size_t cb_hook_code_total;
			uint8_t* p_hook_code;
			size_t i_orig_code_copy;
			size_t cb_orig_code_instr;
			DWORD old_protect;
			uint8_t* p_target_additional;
			// Calculate the size of original code to copy.
			for (cb_orig_code_copy = 0; cb_orig_code_copy < 5; cb_orig_code_copy += ldasm(hook_target + cb_orig_code_copy, &ldasm_data_orig_code_copy, 0));
			// Calculate total size of hook code.
			cb_hook_code_total = cb_hook_code + cb_orig_code_copy + 5;
			// Allocate memory for hook code.
			p_hook_code = (uint8_t*)VirtualAlloc(nullptr, cb_hook_code_total, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!p_hook_code) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "VirtualAlloc");
			// Copy original code to the end of hook code and fix relative code offset operands.
			i_orig_code_copy = 0;
			while (i_orig_code_copy < cb_orig_code_copy) {
				cb_orig_code_instr = ldasm(hook_target + i_orig_code_copy, &ldasm_data_orig_code_copy, 0);
				memcpy(p_hook_code + cb_hook_code + i_orig_code_copy, hook_target + i_orig_code_copy, cb_orig_code_instr);
				if (ldasm_data_orig_code_copy.flags & F_RELATIVE && ldasm_data_orig_code_copy.flags & F_IMM) {
					if (ldasm_data_orig_code_copy.imm_size != sizeof(ptrdiff_t)) THROW_UNEXPECTED_ERROR_EXCEPTION();
					// Fix relative code offset operand.
					*(ptrdiff_t*)(p_hook_code + cb_hook_code + i_orig_code_copy + ldasm_data_orig_code_copy.imm_offset) -=
						(ptrdiff_t)(p_hook_code + cb_hook_code + i_orig_code_copy + cb_orig_code_instr) - (ptrdiff_t)(hook_target + i_orig_code_copy + cb_orig_code_instr);
				}
				i_orig_code_copy += cb_orig_code_instr;
			}
			// Add instruction to jump back.
			// jmp rel32
			(p_hook_code + cb_hook_code + cb_orig_code_copy)[0] = 0xE9;
			*(ptrdiff_t*)((p_hook_code + cb_hook_code + cb_orig_code_copy) + 1) = (hook_target + cb_orig_code_copy) - ((p_hook_code + cb_hook_code + cb_orig_code_copy) + 5);
			// Change memory protection of hook target to allow writes.
			if (!VirtualProtect(hook_target, cb_orig_code_copy, PAGE_EXECUTE_READWRITE, &old_protect)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "VirtualProtect");
			// Add instruction to jump to hook code.
			// jmp rel32
			hook_target[0] = 0xE9;
			*(ptrdiff_t*)(hook_target + 1) = p_hook_code - (hook_target + 5);
			// Fill additional space in hook target with 0xCC bytes.
			for (p_target_additional = hook_target + 5; p_target_additional < hook_target + cb_orig_code_copy; ++p_target_additional) *p_target_additional = 0xCC;
			// Flush instruction cache for hook target.
			if (!FlushInstructionCache(GetCurrentProcess(), hook_target, cb_orig_code_copy)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "FlushInstructionCache");
			// Flush instruction cache for instructions appended to hook code.
			if (!FlushInstructionCache(GetCurrentProcess(), p_hook_code + cb_hook_code, cb_hook_code_total - cb_hook_code)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "FlushInstructionCache");
			return p_hook_code;
		}

		void ApplyHookFinishMakeHookCode(_Inout_ uint8_t* hook_code, _In_ size_t cb_hook_code) {
			// Flush instruction cache for hook code.
			if (!FlushInstructionCache(GetCurrentProcess(), hook_code, cb_hook_code)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "FlushInstructionCache");
		}
#endif
	}
}