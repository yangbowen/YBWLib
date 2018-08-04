#pragma once
#include <memory>
#include <sal.h>
#include <guiddef.h>
#include "..\exception\Exception.h"
#include "..\misc\guid_helper.h"
#include "HookableProcCommon.h"

namespace HookableProc {
	class _impl_HookPosition final {
	public:
		HookPosition* pdecl = nullptr;
		GUID guid_ref = GUID_NULL;
		bool ref = false;
		bool side = false;
		std::unique_ptr<_impl_HookPosition> failsafe = nullptr;
		_impl_HookPosition(at_front_t)
			: ref(false), side(false) {}
		_impl_HookPosition(at_back_t)
			: ref(false), side(true) {}
		_impl_HookPosition(_In_ const GUID* guid_ref, before_t, _In_opt_ const _impl_HookPosition* failsafe)
			: ref(true), side(false) {
			if (!guid_ref) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			this->guid_ref = GUID(*guid_ref);
			if (failsafe) {
				this->failsafe.reset(new _impl_HookPosition(*failsafe));
			}
		}
		_impl_HookPosition(_In_ const GUID* guid_ref, after_t, _In_opt_ const _impl_HookPosition* failsafe)
			: ref(true), side(true) {
			if (!guid_ref) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			this->guid_ref = GUID(*guid_ref);
			if (failsafe) {
				this->failsafe.reset(new _impl_HookPosition(*failsafe));
			}
		}
		_impl_HookPosition(const _impl_HookPosition& t)
			:guid_ref(t.guid_ref), ref(t.ref), side(t.side), failsafe(t.failsafe ? (new _impl_HookPosition(*t.failsafe)) : nullptr) {}
		_impl_HookPosition(_impl_HookPosition&& t)
			:guid_ref(t.guid_ref), ref(t.ref), side(t.side) {
			this->failsafe.reset(t.failsafe.release());
		}
		~_impl_HookPosition() {}
	};
}