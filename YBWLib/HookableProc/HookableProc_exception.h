#pragma once

#include <cstdint>
#include <guiddef.h>
#include "..\YBWLib.h"
#include "..\exception\Exception.h"

namespace HookableProc {
	// A HookableProc specific exception.
	class YBWLIB_API HookableProcException abstract :public Exception::BaseException {
	public:
		static const wchar_t* DescriptionPostfixFormat;
		bool HasProcedureGuid = false;
		GUID ProcedureGuid = GUID_NULL;
		// This exception should pass through this number of layers of hookable procedures unchanged.
		// i.e. these layers of hookable procedures shouldn't set its procedure GUID.
		intptr_t CountShouldPassThroughProcedure = 0;
		// This exception should pass through this number of layers of hookable procedure tables unchanged.
		// i.e. these layers of hookable procedure tables shouldn't set its procedure GUID.
		intptr_t CountShouldPassThroughProcedureTable = 0;
		HookableProcException();
		HookableProcException(const HookableProcException& t);
		HookableProcException(HookableProcException&& t);
		virtual ~HookableProcException();
		virtual wchar_t* GetDescriptionPostfix() const;
		void SetProcedureGuid(const GUID* guid);
	};

	// A HookableProc specific exception that's related to a hook.
	class YBWLIB_API HookableProcHookException abstract :public HookableProcException {
	public:
		static const wchar_t* DescriptionPostfixFormat;
		bool HasHookGuid = false;
		GUID HookGuid = GUID_NULL;
		// This exception should pass through this number of layers of hooks unchanged.
		// i.e. these layers of hooks shouldn't set its hook GUID.
		intptr_t CountShouldPassThroughHookTable = 0;
		// This exception should pass through this number of layers of hookable procedure tables unchanged.
		// i.e. these layers of hookable procedure tables shouldn't set its hook GUID.
		intptr_t CountShouldPassThroughProcedureTable = 0;
		HookableProcHookException();
		HookableProcHookException(const HookableProcHookException& t);
		HookableProcHookException(HookableProcHookException&& t);
		virtual ~HookableProcHookException();
		virtual wchar_t* GetDescriptionPostfix() const;
		void SetHookGuid(const GUID* guid);
	};

	// A hook with the specified GUID already exists.
	class YBWLIB_API HookAlreadyExistException :public HookableProcHookException {
	public:
		static const wchar_t* DescriptionFormat;
		HookAlreadyExistException();
		HookAlreadyExistException(const HookAlreadyExistException& t);
		HookAlreadyExistException(HookAlreadyExistException&& t);
		virtual wchar_t* GetDescription() const;
	};

	// The hook with the specified GUID doesn't exist.
	class YBWLIB_API HookNotExistException :public HookableProcHookException {
	public:
		static const wchar_t* DescriptionFormat;
		HookNotExistException();
		HookNotExistException(const HookNotExistException& t);
		HookNotExistException(HookNotExistException&& t);
		virtual wchar_t* GetDescription() const;
	};

	// A cyclic dependency is found on a hook.
	class YBWLIB_API HookCyclicDependencyException :public HookableProcHookException {
	public:
		static const wchar_t* DescriptionFormat;
		static const wchar_t* DescriptionFormat_DepChainFirst;
		static const wchar_t* DescriptionFormat_DepChainInc;
		static const wchar_t* DescriptionFormat_DepChainLast;
		GUID* DependencyChain = nullptr;
		intptr_t DependencyChainLength = -1;
		HookCyclicDependencyException(const GUID* const * dependency_chain);
		HookCyclicDependencyException(const HookCyclicDependencyException& t);
		HookCyclicDependencyException(HookCyclicDependencyException&& t);
		virtual ~HookCyclicDependencyException();
		virtual wchar_t* GetDescription() const;
	};

	// The hookable procedure may not be modified, as it is currently being invoked.
	class YBWLIB_API InvokingNoModifyException :public HookableProcException {
	public:
		static const wchar_t* DescriptionFormat;
		GUID InvokingHookGuid = GUID_NULL;
		InvokingNoModifyException(const GUID* guid_hook_invoking);
		InvokingNoModifyException(const InvokingNoModifyException& t);
		InvokingNoModifyException(InvokingNoModifyException&& t);
		virtual ~InvokingNoModifyException();
		virtual wchar_t* GetDescription() const;
	};

	// A procedure with the specified GUID already exists.
	class YBWLIB_API ProcedureAlreadyExistException :public HookableProcException {
	public:
		static const wchar_t* DescriptionFormat;
		ProcedureAlreadyExistException();
		ProcedureAlreadyExistException(const ProcedureAlreadyExistException& t);
		ProcedureAlreadyExistException(ProcedureAlreadyExistException&& t);
		virtual wchar_t* GetDescription() const;
	};

	// The procedure with the specified GUID doesn't exist.
	class YBWLIB_API ProcedureNotExistException :public HookableProcException {
	public:
		static const wchar_t* DescriptionFormat;
		ProcedureNotExistException();
		ProcedureNotExistException(const ProcedureNotExistException& t);
		ProcedureNotExistException(ProcedureNotExistException&& t);
		virtual wchar_t* GetDescription() const;
	};
}