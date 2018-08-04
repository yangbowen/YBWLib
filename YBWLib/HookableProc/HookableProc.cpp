#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <unordered_map>
#include <guiddef.h>
#include "..\misc\pimpl_helper.h"
#include "..\exception\Exception.h"
#include "..\misc\guid_helper.h"
#include "HookableProc_exception.h"
#include "HookableProcCommon.h"
#include "HookableProcCommon_internal.h"
#include "HookableProc.h"

namespace HookableProc {
	namespace Internal {
		class _impl_InvocationPacketAdditionalDataHelper {
		public:
			typedef std::unordered_map<GUID, uintptr_t, hash_guid, equal_to_guid> map_data_t;
			InvocationPacketAdditionalDataHelper* pdecl = nullptr;
			map_data_t map_data;
			_impl_InvocationPacketAdditionalDataHelper()
				: map_data() {}
			_impl_InvocationPacketAdditionalDataHelper(const _impl_InvocationPacketAdditionalDataHelper& t)
				: map_data(t.map_data) {}
			_impl_InvocationPacketAdditionalDataHelper(_impl_InvocationPacketAdditionalDataHelper&& t)
				: map_data(t.map_data) {}
			inline bool IsEntryPresent(_In_ const GUID* guid) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				return this->map_data.count(*guid);
			}
			inline uintptr_t GetEntryValue(_In_ const GUID* guid) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				uintptr_t ret = 0;
				try {
					ret = this->map_data.at(*guid);
				} catch (std::out_of_range&) {}
				return ret;
			}
			inline void SetEntryValue(_In_ const GUID* guid, _In_opt_ uintptr_t value) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				this->map_data[*guid] = value;
			}
			inline void DeleteEntry(_In_ const GUID* guid) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				this->map_data.erase(*guid);
			}
		};

		DEFINE_PIMPL_DEFAULT_CTOR(InvocationPacketAdditionalDataHelper);

		DEFINE_PIMPL_FUNCS(InvocationPacketAdditionalDataHelper);

		bool InvocationPacketAdditionalDataHelper::IsEntryPresent(_In_ const GUID* guid) { return this->pimpl->IsEntryPresent(guid); }

		uintptr_t InvocationPacketAdditionalDataHelper::GetEntryValue(_In_ const GUID* guid) { return this->pimpl->GetEntryValue(guid); }

		void InvocationPacketAdditionalDataHelper::SetEntryValue(_In_ const GUID* guid, _In_opt_ uintptr_t value) { return this->pimpl->SetEntryValue(guid, value); }

		void InvocationPacketAdditionalDataHelper::DeleteEntry(_In_ const GUID* guid) { return this->pimpl->DeleteEntry(guid); }
	}
}