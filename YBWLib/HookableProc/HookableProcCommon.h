#pragma once
#ifndef _INCLUDED_YBWLIB_HOOKABLEPROCCOMMON_H_
#define _INCLUDED_YBWLIB_HOOKABLEPROCCOMMON_H_
#include <sal.h>
#include <guiddef.h>
#include "..\YBWLib.h"

namespace HookableProc {
	class HookPosition;
	class _impl_HookPosition;
	namespace Raw {
		class RawProcedure;
		class RawProcedureTable;
	}

	enum HookRet {
		HookRet_Continue = 0,
		HookRet_Yield = 1
	};

	// Hook Insert Position Properties
	struct at_front_t {};
	struct at_back_t {};
	struct before_t {};
	struct after_t {};

	// Partial Invoke Properties
	struct after_not_including_t {};
	struct after_including_t {};

	// Hook Insert Position Properties
	constexpr at_front_t at_front {};
	constexpr at_back_t at_back {};
	constexpr before_t before {};
	constexpr after_t after {};

	// Partial Invoke Properties
	constexpr after_not_including_t after_not_including {};
	constexpr after_including_t after_including {};

	class YBWLIB_API HookPosition {
		friend Raw::RawProcedure;
		friend Raw::RawProcedureTable;
	public:
		HookPosition(at_front_t);
		HookPosition(at_back_t);
		HookPosition(_In_ const GUID* guid_ref, before_t, _In_opt_ const HookPosition* failsafe);
		HookPosition(_In_ const GUID* guid_ref, after_t, _In_opt_ const HookPosition* failsafe);
		FORCEINLINE HookPosition(_In_ const GUID& guid_ref, before_t, _In_opt_ const HookPosition* failsafe)
			: HookPosition(&guid_ref, before, failsafe) {}
		FORCEINLINE HookPosition(_In_ const GUID& guid_ref, after_t, _In_opt_ const HookPosition* failsafe)
			: HookPosition(&guid_ref, after, failsafe) {}
		HookPosition(const HookPosition& t);
		HookPosition(HookPosition&& t);
		virtual ~HookPosition();
	protected:
		_impl_HookPosition* pimpl = nullptr;
	};
}
#endif
