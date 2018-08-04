#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "..\misc\pimpl_helper.h"
#include "HookableProcCommon.h"
#include "HookableProcCommon_internal.h"

namespace HookableProc {
	HookPosition::HookPosition(at_front_t tag)
		: pimpl(new _impl_HookPosition(tag)) {}

	HookPosition::HookPosition(at_back_t tag)
		: pimpl(new _impl_HookPosition(tag)) {}

	HookPosition::HookPosition(_In_ const GUID* guid_ref, before_t tag, _In_opt_ const HookPosition* failsafe)
		: pimpl(new _impl_HookPosition(guid_ref, tag, failsafe ? failsafe->pimpl : nullptr)) {}

	HookPosition::HookPosition(_In_ const GUID* guid_ref, after_t tag, _In_opt_ const HookPosition* failsafe)
		: pimpl(new _impl_HookPosition(guid_ref, tag, failsafe ? failsafe->pimpl : nullptr)) {}

	DEFINE_PIMPL_FUNCS(HookPosition);
}