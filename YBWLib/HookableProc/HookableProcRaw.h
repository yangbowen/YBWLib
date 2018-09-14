#pragma once
#ifndef _INCLUDED_YBWLIB_HOOKABLEPROCRAW_H_
#define _INCLUDED_YBWLIB_HOOKABLEPROCRAW_H_
#include <sal.h>
#include <guiddef.h>
#include "..\YBWLib.h"
#include "HookableProcCommon.h"

namespace HookableProc::Raw {
	class RawHookEntry;
	class RawProcedure;
	class _impl_RawHookEntry;
	class RawHookTable;
	class _impl_RawProcedure;
	class RawProcedureTable;

	typedef HookRet(__stdcall * RawHookFnptrType)(_In_opt_ void* param, _Inout_opt_ void* context, _Inout_ RawProcedure* procedure, _Inout_ RawHookEntry* hook_entry);
	typedef void(__stdcall * RawHookCleanupFnptrType)(_Inout_opt_ void* context, _In_ const GUID* guid_hook);

	class YBWLIB_API RawHookEntry {
		friend RawHookTable;
	public:
		/// <summary>Create a new hook entry.</summary>
		/// <param name="guid">
		/// The GUID of this hook entry.
		/// No other hook entries in the same hookable procedure may have an identical GUID.
		/// </param>
		/// <param name="hook_fnptr">A pointer to the function that is called when the hook is invoked.</param>
		/// <param name="cleanup_fnptr">
		/// A pointer to the function that is called when the hook entry is being deleted.
		/// If this hook entry is copied, the function is called only when the last instance of the hook entry is being deleted.
		/// </param>
		/// <param name="context">An optional context value associated with this hook entry.</param>
		RawHookEntry(_In_ const GUID* guid, _In_opt_ RawHookFnptrType hook_fnptr, _In_opt_ RawHookCleanupFnptrType cleanup_fnptr, _Inout_opt_ void* context);
		RawHookEntry(const RawHookEntry& t);
		RawHookEntry(RawHookEntry&& t);
		virtual ~RawHookEntry();
		/// <summary>Get the GUID of this hook entry.</summary>
		/// <returns>
		/// A pointer to the GUID of this hook entry.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		const GUID* GetGuid() const;
	protected:
		_impl_RawHookEntry* pimpl = nullptr;
		/// <summary>Invoke this hook entry.</summary>
		/// <param name="param">An optional parameter from the caller.</param>
		/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
		/// <returns>A <c>HookRet</c> value to tell the caller what further actions it should take after calling this function.</returns>
		virtual HookRet __thiscall InvokeHook(_In_opt_ void* param, _Inout_ RawProcedure* procedure) const;
	};

	/// <summary>A raw hookable procedure.</summary>
	/// <remarks>Objects of this class are thread-safe. Callers don't need to synchronize the calls.</remarks>
	class YBWLIB_API RawProcedure {
		friend RawHookTable;
		friend RawProcedureTable;
	public:
		/// <summary>Create a hookable procedure.</summary>
		/// <param name="guid_procedure">
		/// The GUID of the hookable procedure to be created.
		/// No other hookable procedures may have an identical GUID.
		/// </param>
		/// <param name="context_procedure">An optional context value associated with the procedure.</param>
		/// <returns>
		/// A pointer to the newly-created hookable procedure.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		static RawProcedure* CreateProcedure(_In_ const GUID* guid_procedure, _Inout_opt_ void* context_procedure);
		/// <summary>Get the pointer to an existing hookable procedure.</summary>
		/// <param name="guid_procedure">The GUID of the hookable procedure.</param>
		/// <returns>
		/// A pointer to the hookable procedure.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		static RawProcedure* GetProcedure(_In_ const GUID* guid_procedure);
		/// <summary>Create a hook.</summary>
		/// <param name="guid_procedure">The GUID of the hookable procedure.</param>
		/// <param name="hook_entry">
		/// The hook entry.
		/// This function copies the hook entry and keeps an internal storage of it until the hook is deleted.
		/// </param>
		/// <param name="hook_position">The hook position at which to create the hook.</param>
		static void CreateHook(_In_ const GUID* guid_procedure, _In_ const RawHookEntry* hook_entry, _In_ const HookPosition* hook_position);
		/// <summary>Delete a hook.</summary>
		/// <param name="guid_procedure">The GUID of the hookable procedure.</param>
		/// <param name="guid_hook">The GUID of the hook to be deleted.</param>
		static void DeleteHook(_In_ const GUID* guid_procedure, _In_ const GUID* guid_hook);
		RawProcedure(_In_ const GUID* guid, _Inout_opt_ void* context_procedure);
		RawProcedure(const RawProcedure&) = delete;
		RawProcedure(RawProcedure&& t) = delete;
		virtual ~RawProcedure();
		/// <summary>Get the GUID of this hookable procedure.</summary>
		/// <returns>
		/// A pointer to the GUID of this hookable procedure.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		const GUID* GetGuid() const;
		/// <summary>Get the context value associated with this hookable procedure.</summary>
		/// <returns>The context value associated with this hookable procedure.</returns>
		void* GetProcedureContext() const;
		virtual void CreateHook(_In_ const RawHookEntry* hook_entry, _In_ const HookPosition* hook_position);
		virtual void DeleteHook(_In_ const GUID* guid_hook);
		/// <summary>Invoke all inserted hooks associated with this procedure.</summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		virtual void __thiscall Invoke(_In_opt_ void* param);
		/// <summary>
		/// Invoke some inserted hooks associated with this procedure.
		/// The hooks that are after (not including) the one identified by <paramref name="guid_hook_ref" /> are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		virtual void __thiscall InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_not_including_t);
		/// <summary>
		/// Invoke some inserted hooks associated with this procedure.
		/// The hook identified by <paramref name="guid_hook_ref" /> and the hooks that are after it are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		virtual void __thiscall InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_including_t);
	protected:
		_impl_RawProcedure* pimpl = nullptr;
		intptr_t* GetPCountIsInvoking();
		const GUID** GetPGuidHookInvoking();
	};
}
#endif
