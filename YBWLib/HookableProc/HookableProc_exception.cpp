#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <guiddef.h>
#include <string>
#include "..\exception\Exception.h"
#include "HookableProc_exception.h"

#pragma comment(lib,"rpcrt4.lib")

namespace HookableProc {
	const wchar_t* HookableProcException::DescriptionPostfixFormat = L"\nHookable Procedure GUID: %2!s!.%1!s!";
	const wchar_t* HookableProcHookException::DescriptionPostfixFormat = L"\nHook GUID: %2!s!.%1!s!";
	const wchar_t* HookAlreadyExistException::DescriptionFormat = L"A hook with the specified GUID already exists.%1!s!";
	const wchar_t* HookNotExistException::DescriptionFormat = L"The hook with the specified GUID doesn't exist.%1!s!";
	const wchar_t* HookCyclicDependencyException::DescriptionFormat = L"A cyclic dependency is found on a hook:\n%2!s!%1!s!";
	const wchar_t* HookCyclicDependencyException::DescriptionFormat_DepChainFirst = L"%1!s! depends on\n";
	const wchar_t* HookCyclicDependencyException::DescriptionFormat_DepChainInc = L"%1!s!, which depends on\n";
	const wchar_t* HookCyclicDependencyException::DescriptionFormat_DepChainLast = L"%1!s!.";
	const wchar_t* InvokingNoModifyException::DescriptionFormat = L"The hookable procedure may not be modified, as its hook %2!s! is currently being invoked.%1!s!";
	const wchar_t* ProcedureAlreadyExistException::DescriptionFormat = L"A hookable procedure with the specified GUID already exists.%1!s!";
	const wchar_t* ProcedureNotExistException::DescriptionFormat = L"The hookable procedure with the specified GUID doesn't exist.%1!s!";

	HookableProcException::HookableProcException() {}

	HookableProcException::HookableProcException(const HookableProcException& t)
		: HasProcedureGuid(t.HasProcedureGuid), ProcedureGuid(t.ProcedureGuid), CountShouldPassThroughProcedure(t.CountShouldPassThroughProcedure), CountShouldPassThroughProcedureTable(t.CountShouldPassThroughProcedureTable) {}

	HookableProcException::HookableProcException(HookableProcException&& t)
		: HasProcedureGuid(t.HasProcedureGuid), ProcedureGuid(t.ProcedureGuid), CountShouldPassThroughProcedure(t.CountShouldPassThroughProcedure), CountShouldPassThroughProcedureTable(t.CountShouldPassThroughProcedureTable) {}

	HookableProcException::~HookableProcException() {
		this->HasProcedureGuid = false;
		this->ProcedureGuid = GUID_NULL;
	}

	void HookableProcException::SetProcedureGuid(const GUID* guid) {
		if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
		this->ProcedureGuid = *guid;
	}

	wchar_t* HookableProcException::GetDescriptionPostfix() const {
		wchar_t* ret = nullptr;
		wchar_t* str_postfix_parent = this->BaseException::GetDescriptionPostfix();
		if (this->HasProcedureGuid) {
			RPC_WSTR rpcstr_guid_procedure = nullptr;
			if (UuidToString(&this->ProcedureGuid, &rpcstr_guid_procedure) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "UuidToStringW");
			ret = Exception::format_string(this->DescriptionPostfixFormat, str_postfix_parent, rpcstr_guid_procedure);
			if (rpcstr_guid_procedure) {
				if (RpcStringFree(&rpcstr_guid_procedure) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "RpcStringFreeW");
				rpcstr_guid_procedure = nullptr;
			}
			if (str_postfix_parent) {
				delete[] str_postfix_parent;
				str_postfix_parent = nullptr;
			}
		} else {
			ret = str_postfix_parent;
		}
		return ret;
	}

	HookableProcHookException::HookableProcHookException() {}

	HookableProcHookException::HookableProcHookException(const HookableProcHookException& t)
		: HasHookGuid(t.HasHookGuid), HookGuid(t.HookGuid), CountShouldPassThroughHookTable(t.CountShouldPassThroughHookTable), CountShouldPassThroughProcedureTable(t.CountShouldPassThroughProcedureTable) {}

	HookableProcHookException::HookableProcHookException(HookableProcHookException&& t)
		: HasHookGuid(t.HasHookGuid), HookGuid(t.HookGuid), CountShouldPassThroughHookTable(t.CountShouldPassThroughHookTable), CountShouldPassThroughProcedureTable(t.CountShouldPassThroughProcedureTable) {}

	HookableProcHookException::~HookableProcHookException() {
		this->HasHookGuid = false;
		this->HookGuid = GUID_NULL;
	}

	void HookableProcHookException::SetHookGuid(const GUID* guid) {
		if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
		this->HookGuid = *guid;
	}

	wchar_t* HookableProcHookException::GetDescriptionPostfix() const {
		wchar_t* ret = nullptr;
		wchar_t* str_postfix_parent = this->BaseException::GetDescriptionPostfix();
		if (this->HasHookGuid) {
			RPC_WSTR rpcstr_guid_hook = nullptr;
			if (UuidToString(&this->HookGuid, &rpcstr_guid_hook) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "UuidToStringW");
			ret = Exception::format_string(this->DescriptionPostfixFormat, str_postfix_parent, rpcstr_guid_hook);
			if (rpcstr_guid_hook) {
				if (RpcStringFree(&rpcstr_guid_hook) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "RpcStringFreeW");
				rpcstr_guid_hook = nullptr;
			}
			if (str_postfix_parent) {
				delete[] str_postfix_parent;
				str_postfix_parent = nullptr;
			}
		} else {
			ret = str_postfix_parent;
		}
		return ret;
	}

	HookAlreadyExistException::HookAlreadyExistException()
		: HookableProcHookException() {}

	HookAlreadyExistException::HookAlreadyExistException(const HookAlreadyExistException& t)
		: HookableProcHookException(t) {}

	HookAlreadyExistException::HookAlreadyExistException(HookAlreadyExistException&& t)
		: HookableProcHookException(t) {}

	wchar_t* HookAlreadyExistException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = Exception::format_string(this->DescriptionFormat, this->GetDescriptionPostfix());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	HookNotExistException::HookNotExistException()
		: HookableProcHookException() {}

	HookNotExistException::HookNotExistException(const HookNotExistException& t)
		: HookableProcHookException(t) {}

	HookNotExistException::HookNotExistException(HookNotExistException&& t)
		: HookableProcHookException(t) {}

	wchar_t* HookNotExistException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = Exception::format_string(this->DescriptionFormat, this->GetDescriptionPostfix());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	HookCyclicDependencyException::HookCyclicDependencyException(const GUID* const * dependency_chain)
		: HookableProcHookException() {
		if (!dependency_chain) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
		for (this->DependencyChainLength = 0; dependency_chain[this->DependencyChainLength]; ++this->DependencyChainLength);
		if (this->DependencyChainLength < 2) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
	}

	HookCyclicDependencyException::HookCyclicDependencyException(const HookCyclicDependencyException& t)
		: HookableProcHookException(t), DependencyChainLength(t.DependencyChainLength) {
		if (t.DependencyChain) {
			this->DependencyChain = new GUID[this->DependencyChainLength];
			for (intptr_t i = 0; i < this->DependencyChainLength; ++i) this->DependencyChain[i] = t.DependencyChain[i];
		} else {
			this->DependencyChain = nullptr;
		}
	}

	HookCyclicDependencyException::HookCyclicDependencyException(HookCyclicDependencyException&& t)
		: HookableProcHookException(t), DependencyChain(t.DependencyChain), DependencyChainLength(t.DependencyChainLength) {}

	HookCyclicDependencyException::~HookCyclicDependencyException() {
		if (this->DependencyChain) {
			delete[] this->DependencyChain;
			this->DependencyChain = nullptr;
		}
		this->DependencyChainLength = -1;
	}

	wchar_t* HookCyclicDependencyException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		std::wstring str_dep_chain;
		if (this->DependencyChainLength < 2) THROW_UNEXPECTED_ERROR_EXCEPTION();
		if (!this->DependencyChain) THROW_UNEXPECTED_ERROR_EXCEPTION();
		{
			RPC_WSTR rpcstr_guid_node = nullptr;
			if (UuidToString(&this->DependencyChain[0], &rpcstr_guid_node) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "UuidToStringW");
			wchar_t* str_dep_chain_node = Exception::format_string(this->DescriptionFormat_DepChainFirst, rpcstr_guid_node);
			if (rpcstr_guid_node) {
				if (RpcStringFree(&rpcstr_guid_node) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "RpcStringFreeW");
				rpcstr_guid_node = nullptr;
			}
			str_dep_chain += str_dep_chain_node;
			if (str_dep_chain_node) {
				delete[] str_dep_chain_node;
				str_dep_chain_node = nullptr;
			}
		}
		for (intptr_t i = 1; i < this->DependencyChainLength - 1; ++i) {
			RPC_WSTR rpcstr_guid_node = nullptr;
			if (UuidToString(&this->DependencyChain[i], &rpcstr_guid_node) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "UuidToStringW");
			wchar_t* str_dep_chain_node = Exception::format_string(this->DescriptionFormat_DepChainInc, rpcstr_guid_node);
			if (rpcstr_guid_node) {
				if (RpcStringFree(&rpcstr_guid_node) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "RpcStringFreeW");
				rpcstr_guid_node = nullptr;
			}
			str_dep_chain += str_dep_chain_node;
			if (str_dep_chain_node) {
				delete[] str_dep_chain_node;
				str_dep_chain_node = nullptr;
			}
		}
		{
			RPC_WSTR rpcstr_guid_node = nullptr;
			if (UuidToString(&this->DependencyChain[this->DependencyChainLength - 1], &rpcstr_guid_node) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "UuidToStringW");
			wchar_t* str_dep_chain_node = Exception::format_string(this->DescriptionFormat_DepChainLast, rpcstr_guid_node);
			if (rpcstr_guid_node) {
				if (RpcStringFree(&rpcstr_guid_node) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "RpcStringFreeW");
				rpcstr_guid_node = nullptr;
			}
			str_dep_chain += str_dep_chain_node;
			if (str_dep_chain_node) {
				delete[] str_dep_chain_node;
				str_dep_chain_node = nullptr;
			}
		}
		wchar_t* ret = Exception::format_string(this->DescriptionFormat, this->GetDescriptionPostfix(), str_dep_chain.data());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	InvokingNoModifyException::InvokingNoModifyException(const GUID* guid_hook_invoking) {
		if (!guid_hook_invoking) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
		this->InvokingHookGuid = *guid_hook_invoking;
	}

	InvokingNoModifyException::InvokingNoModifyException(const InvokingNoModifyException& t)
		: InvokingHookGuid(t.InvokingHookGuid) {}

	InvokingNoModifyException::InvokingNoModifyException(InvokingNoModifyException&& t)
		: InvokingHookGuid(t.InvokingHookGuid) {}

	InvokingNoModifyException::~InvokingNoModifyException() {
		this->InvokingHookGuid = GUID_NULL;
	}

	wchar_t* InvokingNoModifyException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		RPC_WSTR rpcstr_guid_hook_invoking = nullptr;
		if (UuidToString(&this->InvokingHookGuid, &rpcstr_guid_hook_invoking) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "UuidToStringW");
		wchar_t* ret = Exception::format_string(this->DescriptionFormat, this->GetDescriptionPostfix(), rpcstr_guid_hook_invoking);
		if (rpcstr_guid_hook_invoking) {
			if (RpcStringFree(&rpcstr_guid_hook_invoking) != RPC_S_OK) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("rpcrt4", "RpcStringFreeW");
			rpcstr_guid_hook_invoking = nullptr;
		}
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	ProcedureAlreadyExistException::ProcedureAlreadyExistException()
		: HookableProcException() {}

	ProcedureAlreadyExistException::ProcedureAlreadyExistException(const ProcedureAlreadyExistException& t)
		: HookableProcException(t) {}

	ProcedureAlreadyExistException::ProcedureAlreadyExistException(ProcedureAlreadyExistException&& t)
		: HookableProcException(t) {}

	wchar_t* ProcedureAlreadyExistException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = Exception::format_string(this->DescriptionFormat, this->GetDescriptionPostfix());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	ProcedureNotExistException::ProcedureNotExistException()
		: HookableProcException() {}

	ProcedureNotExistException::ProcedureNotExistException(const ProcedureNotExistException& t)
		: HookableProcException(t) {}

	ProcedureNotExistException::ProcedureNotExistException(ProcedureNotExistException&& t)
		: HookableProcException(t) {}

	wchar_t* ProcedureNotExistException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = Exception::format_string(this->DescriptionFormat, this->GetDescriptionPostfix());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}
}