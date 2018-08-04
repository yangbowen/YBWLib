#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "HookableProc_exception.h"
#include "..\resource\resource_helper.h"
#include "..\resource\resource.h"
#include "HookableProc_res.h"

namespace HookableProc {
	static bool res_loaded = false;

	void res_load() {
		if (!res_loaded) {
			HookableProcException::DescriptionPostfixFormat = load_res_string(IDS_EXCEPTDESCPFIX_HOOKABLE_PROC);
			HookableProcHookException::DescriptionPostfixFormat = load_res_string(IDS_EXCEPTDESCPFIX_HOOKABLE_PROC_HOOK);
			HookAlreadyExistException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_HOOK_ALREADY_EXIST);
			HookNotExistException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_HOOK_NOT_EXIST);
			HookCyclicDependencyException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_HOOK_CYCLIC_DEPENDENCY);
			HookCyclicDependencyException::DescriptionFormat_DepChainFirst = load_res_string(IDS_EXCEPTDESC_HOOK_CYCLIC_DEPENDENCY_DEPCHAINFIRST);
			HookCyclicDependencyException::DescriptionFormat_DepChainInc = load_res_string(IDS_EXCEPTDESC_HOOK_CYCLIC_DEPENDENCY_DEPCHAININC);
			HookCyclicDependencyException::DescriptionFormat_DepChainLast = load_res_string(IDS_EXCEPTDESC_HOOK_CYCLIC_DEPENDENCY_DEPCHAINLAST);
			InvokingNoModifyException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_HOOK_INVOKING_NO_MODIFY);
			ProcedureAlreadyExistException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_HOOKABLE_PROCEDURE_ALREADY_EXIST);
			ProcedureNotExistException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_HOOKABLE_PROCEDURE_NOT_EXIST);
			res_loaded = true;
		}
	}
}