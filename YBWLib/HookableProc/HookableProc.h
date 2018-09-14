#pragma once
#ifndef _INCLUDED_YBWLIB_HOOKABLEPROC_H_
#define _INCLUDED_YBWLIB_HOOKABLEPROC_H_
#include <new>
#include <sal.h>
#include <guiddef.h>
#include "..\YBWLib.h"
#include "..\exception\Exception.h"
#include "HookableProcCommon.h"
#include "HookableProcRaw.h"

namespace HookableProc {
	namespace Internal {
		class InvocationPacketAdditionalDataHelper;
		class _impl_InvocationPacketAdditionalDataHelper;

		class YBWLIB_API InvocationPacketAdditionalDataHelper final {
		public:
			InvocationPacketAdditionalDataHelper();
			InvocationPacketAdditionalDataHelper(const InvocationPacketAdditionalDataHelper& t);
			InvocationPacketAdditionalDataHelper(InvocationPacketAdditionalDataHelper&& t);
			~InvocationPacketAdditionalDataHelper();
			bool IsEntryPresent(_In_ const GUID* guid);
			uintptr_t GetEntryValue(_In_ const GUID* guid);
			void SetEntryValue(_In_ const GUID* guid, _In_opt_ uintptr_t value);
			void DeleteEntry(_In_ const GUID* guid);
		protected:
			_impl_InvocationPacketAdditionalDataHelper * pimpl = nullptr;
		};
		class _impl_InvocationPacketAdditionalDataHelper;
	}

	/// <summary>A hookable function that has a main hook when it's just created and may return something.</summary>
	/// <remarks>Objects of this class are thread-safe. Callers don't need to synchronize the calls.</remarks>
	/// <typeparam name="_Paramty">The type of the parameter of the function.</typeparam>
	/// <typeparam name="_Returnty">
	/// The type of the return value of the function.
	/// The destructor of the type passed to this parameter, if any, must be accessible to <c>HookableFunction</c>
	/// </typeparam>
	template <typename _Paramty, typename _Returnty>
	class HookableFunction final {
	public:
		using ParamType = _Paramty;
		using ReturnType = _Returnty;
		class InvocationPacket final {
		public:
			using ParamType = ParamType;
			using ReturnType = ReturnType;
			explicit InvocationPacket(_Inout_opt_ ParamType* param)
				: param(param) {}
			InvocationPacket(const InvocationPacket&) = delete;
			InvocationPacket(InvocationPacket&&) = delete;
			~InvocationPacket() {
				if (this->ret) {
					delete this->ret;
					this->ret = nullptr;
				}
			}
			/// <summary>Get the parameter passed from the invoker.</summary>
			/// <returns>
			/// A pointer to the parameter.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			ParamType* GetParam() const { return this->param; }
			/// <summary>Get the current return value set by a previous hook.</summary>
			/// <returns>
			/// A pointer to the return value.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			ReturnType* GetReturn() const { return this->ret; }
			/// <summary>
			/// Make this object releases ownership of the current return value set by a previous hook.
			/// After calling <c>ReleaseReturn</c>, the return value pointer stored in this object becomes empty.
			/// </summary>
			/// <returns>
			/// A pointer to the return value.
			/// The caller should delete the returned object when it's no longer useful.
			/// The caller may pass it to a managed pointer object.
			/// </returns>
			ReturnType* ReleaseReturn() {
				ReturnType* ret_t = this->ret;
				this->ret = nullptr;
				return ret_t;
			}
			/// <summary>
			/// Set the return value.
			/// If a non-empty return value pointer is already stored in this object, the value pointed by it will be deleted.
			/// </summary>
			/// <param name="ret_new">
			/// The new return value pointer.
			/// If it's not empty, it must point to some dynamically allocated memory.
			/// After this function is called, this object owns the pointer passed to this parameter and will delete the value pointed to by it when it's no longer useful.
			/// The caller should NOT delete the value pointed to by it, NOR should the caller pass it to a managed pointer object.
			/// </param>
			void SetReturn(_Inout_opt_ ReturnType* ret_new) {
				if (this->ret) {
					delete this->ret;
					this->ret = nullptr;
				}
				this->ret = ret_new;
			}
			inline bool IsAdditionalDataEntryPresent(_In_ const GUID* guid) { return this->add_data_helper.IsEntryPresent(guid); }
			FORCEINLINE bool IsAdditionalDataEntryPresent(_In_ const GUID& guid) { return this->IsAdditionalDataEntryPresent(&guid); }
			inline uintptr_t GetAdditionalDataValue(_In_ const GUID* guid) { return this->add_data_helper.GetEntryValue(guid); }
			FORCEINLINE uintptr_t GetAdditionalDataValue(_In_ const GUID& guid) { return this->GetAdditionalDataValue(&guid); }
			inline void SetAdditionalDataValue(_In_ const GUID* guid, _In_opt_ uintptr_t value) { return this->add_data_helper.SetEntryValue(guid, value); }
			FORCEINLINE void SetAdditionalDataValue(_In_ const GUID& guid, _In_opt_ uintptr_t value) { return this->SetAdditionalDataValue(&guid, value); }
			inline void DeleteAdditionalDataEntry(_In_ const GUID* guid) { return this->add_data_helper.DeleteEntry(guid); }
			FORCEINLINE void DeleteAdditionalDataEntry(_In_ const GUID& guid) { return this->DeleteAdditionalDataEntry(&guid); }
		protected:
			ParamType * param = nullptr;
			ReturnType* ret = nullptr;
			Internal::InvocationPacketAdditionalDataHelper add_data_helper;
		};
		/// <summary>A hook entry of which the hook function and the cleanup function are non-static member functions of a class.</summary>
		/// <typeparam name="_Classty">The class that contains the hook function and the cleanup function.</typeparam>
		template <typename _Classty>
		class HookEntry final {
		public:
			using ParamType = ParamType;
			using ReturnType = ReturnType;
			using ClassType = _Classty;
			typedef HookRet(__thiscall ClassType:: * HookFnptrType)(_In_ InvocationPacket* packet, _Inout_opt_ void* context, _Inout_ HookableFunction* func, _Inout_ HookEntry* hook_entry);
			typedef void(__thiscall ClassType:: * HookCleanupFnptrType)(_Inout_opt_ void* context, _In_ const GUID* guid_hook);
			typedef HookRet(__thiscall ClassType::*LiteHookFnptrType)(_In_ InvocationPacket* packet);
			/// <summary>Create a hook entry.</summary>
			/// <param name="guid">
			/// The GUID of this hook entry.
			/// No other hook entries in the same hookable function may have an identical GUID.
			/// </param>
			/// <param name="hook_fnptr">A pointer to the member function that is called when the hook is invoked.</param>
			/// <param name="cleanup_fnptr">
			/// A pointer to the member function that is called when the hook entry is being deleted.
			/// If this hook entry is copied, the function is called only when the last instance of the hook entry is being deleted.
			/// </param>
			/// <param name="obj">
			/// A pointer to the object that's used as the <c>this</c> pointer when calling <paramref name="hook_fnptr" /> and <paramref name="cleanup_fnptr" />.
			/// The hook entry itself will not delete the object pointed to by it.
			/// However, the member function pointed by <paramref name="cleanup_fnptr" /> may delete it using <c>delete this</c>.
			/// </param>
			/// <param name="context">An optional context value associated with this hook entry.</param>
			HookEntry(_In_ const GUID* guid, _In_opt_ HookFnptrType hook_fnptr, _In_opt_ HookCleanupFnptrType cleanup_fnptr, _In_ ClassType* obj, _Inout_opt_ void* context) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				if (!obj) THROW_INVALID_PARAMETER_EXCEPTION_THIS(3);
				ctx_t* ctx = new ctx_t();
				ctx->hook_fnptr = hook_fnptr;
				ctx->cleanup_fnptr = cleanup_fnptr;
				ctx->obj = obj;
				ctx->context = context;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->obj || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						HookableFunction* func = reinterpret_cast<HookableFunction*>(rawproc->GetProcedureContext());
						if (!func || !func->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->hook_fnptr)
							return (ctx->obj->*(ctx->hook_fnptr))(packet, ctx->context, func, ctx->hook_entry);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx || !ctx->obj || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->cleanup_fnptr)
							(ctx->obj->*(ctx->cleanup_fnptr))(ctx->context, guid_hook);
						delete ctx->hook_entry;
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			/// <summary>Create a hook entry.</summary>
			/// <param name="guid">
			/// The GUID of this hook entry.
			/// No other hook entries in the same hookable function may have an identical GUID.
			/// </param>
			/// <param name="lite_hook_fnptr">A pointer to the member function that is called when the hook is invoked.</param>
			/// <param name="obj">
			/// A pointer to the object that's used as the <c>this</c> pointer when calling <paramref name="hook_fnptr" /> and <paramref name="cleanup_fnptr" />.
			/// The hook entry itself will not delete the object pointed to by it.
			/// However, the member function pointed by <paramref name="cleanup_fnptr" /> may delete it using <c>delete this</c>.
			/// </param>
			HookEntry(_In_ const GUID* guid, _In_opt_ LiteHookFnptrType lite_hook_fnptr, _In_ ClassType* obj) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				if (!obj) THROW_INVALID_PARAMETER_EXCEPTION_THIS(2);
				ctx_t* ctx = new ctx_t();
				ctx->lite_hook_fnptr = lite_hook_fnptr;
				ctx->obj = obj;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->obj || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						HookableFunction* func = reinterpret_cast<HookableFunction*>(rawproc->GetProcedureContext());
						if (!func || !func->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->lite_hook_fnptr)
							return (ctx->obj->*(ctx->lite_hook_fnptr))(packet);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx || !ctx->obj) THROW_UNEXPECTED_ERROR_EXCEPTION();
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			HookEntry(const HookEntry& t)
				: raw_entry(t.raw_entry ? new Raw::RawHookEntry(*t.raw_entry) : nullptr) {}
			HookEntry(HookEntry&& t)
				: raw_entry(t.raw_entry) {
				t.raw_entry = nullptr;
			}
			~HookEntry() {
				if (raw_entry) {
					delete raw_entry;
					raw_entry = nullptr;
				}
			}
			/// <summary>Get the GUID of this hook entry.</summary>
			/// <returns>
			/// A pointer to the GUID of this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const GUID* GetGuid() const { return this->raw_entry.GetGuid(); }
			/// <summary>Get a pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry of this hook entry.</summary>
			/// <returns>
			/// A pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const Raw::RawHookEntry* GetRawEntry() const { return this->raw_entry; }
		protected:
			struct ctx_t final {
				union {
					HookFnptrType hook_fnptr = nullptr;
					LiteHookFnptrType lite_hook_fnptr;
				};
				HookCleanupFnptrType cleanup_fnptr = nullptr;
				ClassType* obj = nullptr;
				void* context = nullptr;
				HookEntry* hook_entry = nullptr;
				explicit ctx_t() {}
				ctx_t(const ctx_t&) = delete;
				ctx_t(ctx_t&&) = delete;
			};
			Raw::RawHookEntry* raw_entry = nullptr;
		};
		/// <summary>A hook entry of which the hook function and the cleanup function are non-member functions or static member functions.</summary>
		template <>
		class HookEntry<void> final {
		public:
			using ParamType = ParamType;
			using ReturnType = ReturnType;
			typedef HookRet(__stdcall * HookFnptrType)(_In_ InvocationPacket* packet, _Inout_opt_ void* context, _Inout_ HookableFunction* func, _Inout_ HookEntry* hook_entry);
			typedef void(__stdcall * HookCleanupFnptrType)(_Inout_opt_ void* context, _In_ const GUID* guid_hook);
			typedef HookRet(__stdcall *LiteHookFnptrType)(_In_ InvocationPacket* packet);
			/// <summary>Create a hook entry.</summary>
			/// <param name="guid">
			/// The GUID of this hook entry.
			/// No other hook entries in the same hookable function may have an identical GUID.
			/// </param>
			/// <param name="hook_fnptr">A pointer to the function that is called when the hook is invoked.</param>
			/// <param name="cleanup_fnptr">
			/// A pointer to the function that is called when the hook entry is being deleted.
			/// If this hook entry is copied, the function is called only when the last instance of the hook entry is being deleted.
			/// </param>
			/// <param name="context">An optional context value associated with this hook entry.</param>
			HookEntry(_In_ const GUID* guid, _In_opt_ HookFnptrType hook_fnptr, _In_opt_ HookCleanupFnptrType cleanup_fnptr, _Inout_opt_ void* context) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				ctx_t* ctx = new ctx_t();
				ctx->hook_fnptr = hook_fnptr;
				ctx->cleanup_fnptr = cleanup_fnptr;
				ctx->context = context;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						HookableFunction* func = reinterpret_cast<HookableFunction*>(rawproc->GetProcedureContext());
						if (!func || !func->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->hook_fnptr)
							return (*ctx->hook_fnptr)(packet, ctx->context, func, ctx->hook_entry);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->cleanup_fnptr)
							(*ctx->cleanup_fnptr)(ctx->context, guid_hook);
						delete ctx->hook_entry;
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			/// <summary>Create a hook entry.</summary>
			/// <param name="guid">
			/// The GUID of this hook entry.
			/// No other hook entries in the same hookable function may have an identical GUID.
			/// </param>
			/// <param name="lite_hook_fnptr">A pointer to the function that is called when the hook is invoked.</param>
			HookEntry(_In_ const GUID* guid, _In_opt_ LiteHookFnptrType lite_hook_fnptr) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				ctx_t* ctx = new ctx_t();
				ctx->lite_hook_fnptr = lite_hook_fnptr;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						HookableFunction* func = reinterpret_cast<HookableFunction*>(rawproc->GetProcedureContext());
						if (!func || !func->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->lite_hook_fnptr)
							return (*ctx->lite_hook_fnptr)(packet);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						UNREFERENCED_PARAMETER(guid_hook);
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx) THROW_UNEXPECTED_ERROR_EXCEPTION();
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			HookEntry(const HookEntry& t)
				: raw_entry(t.raw_entry ? new Raw::RawHookEntry(*t.raw_entry) : nullptr) {}
			HookEntry(HookEntry&& t)
				: raw_entry(t.raw_entry) {
				t.raw_entry = nullptr;
			}
			~HookEntry() {
				if (raw_entry) {
					delete raw_entry;
					raw_entry = nullptr;
				}
			}
			/// <summary>Get the GUID of this hook entry.</summary>
			/// <returns>
			/// A pointer to the GUID of this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const GUID* GetGuid() const { return this->raw_entry.GetGuid(); }
			/// <summary>Get a pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry of this hook entry.</summary>
			/// <returns>
			/// A pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const Raw::RawHookEntry* GetRawEntry() const { return this->raw_entry; }
		protected:
			struct ctx_t final {
				union {
					HookFnptrType hook_fnptr = nullptr;
					LiteHookFnptrType lite_hook_fnptr;
				};
				HookCleanupFnptrType cleanup_fnptr = nullptr;
				void* context = nullptr;
				HookEntry* hook_entry = nullptr;
				explicit ctx_t() {}
				ctx_t(const ctx_t&) = delete;
				ctx_t(ctx_t&&) = delete;
			};
			Raw::RawHookEntry* raw_entry = nullptr;
		};
		/// <summary>Create a hookable function.</summary>
		/// <param name="guid_function">
		/// The GUID of the hookable function to be created.
		/// No other hookable functions or hookable procedures may have an identical GUID.
		/// </param>
		/// <param name="context_func">An optional context value associated with the function.</param>
		/// <param name="hook_entry_main">The main hook entry.</param>
		/// <returns>
		/// A pointer to the newly-created hookable function.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		template <typename _Classty>
		static HookableFunction* CreateFunction(_In_ const GUID* guid_function, _Inout_opt_ void* context_func, _In_ const HookEntry<_Classty>* hook_entry_main) {
			if (!guid_function) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 0);
			if (!hook_entry_main) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 2);
			void* mem_func = malloc(sizeof(HookableFunction));
			if (!mem_func) THROW_EXTERNAL_API_ERROR_EXCEPTION_NOLIB("malloc");
			memset(mem_func, 0, sizeof(HookableFunction));
			reinterpret_cast<HookableFunction*>(mem_func)->constructed = false;
			MemoryBarrier();
			Raw::RawProcedure* rawproc = Raw::RawProcedure::CreateProcedure(guid_function, reinterpret_cast<void*>(reinterpret_cast<HookableFunction*>(mem_func)));
			HookPosition hook_position_main(at_front);
			rawproc->CreateHook(hook_entry_main->GetRawEntry(), &hook_position_main);
			return new(mem_func) HookableFunction(rawproc, context_func);
		}
		/// <summary>Get the pointer to an existing hookable function.</summary>
		/// <param name="guid_function">The GUID of the hookable function.</param>
		/// <returns>
		/// A pointer to the hookable function.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		static HookableFunction* GetFunction(_In_ const GUID* guid_function) {
			if (!guid_function) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 0);
			Raw::RawProcedure* rawproc = Raw::RawProcedure::GetProcedure(guid_function);
			HookableFunction* func = reinterpret_cast<HookableFunction*>(rawproc->GetProcedureContext());
			if (!func || !func->constructed) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 0);
			return func;
		}
		/// <summary>Create a hook.</summary>
		/// <param name="guid_function">The GUID of the hookable function.</param>
		/// <param name="hook_entry">
		/// The hook entry.
		/// This function copies the hook entry and keeps an internal storage of it until the hook is deleted.
		/// </param>
		/// <param name="hook_position">The hook position at which to create the hook.</param>
		template <typename _Classty>
		static void CreateHook(_In_ const GUID* guid_function, _In_ const HookEntry<_Classty>* hook_entry, _In_ const HookPosition* hook_position) {
			if (!guid_function) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 0);
			if (!hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 1);
			if (!hook_position) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 2);
			return Raw::RawProcedure::CreateHook(guid_function, hook_entry->GetRawEntry(), hook_position);
		}
		/// <summary>Create a hook.</summary>
		/// <param name="guid_function">The GUID of the hookable function.</param>
		/// <param name="hook_entry">
		/// The hook entry.
		/// This function copies the hook entry and keeps an internal storage of it until the hook is deleted.
		/// </param>
		/// <param name="hook_position">The hook position at which to create the hook.</param>
		template <typename _Classty>
		FORCEINLINE static void CreateHook(_In_ const GUID& guid_function, _In_ const HookEntry<_Classty>& hook_entry, _In_ const HookPosition& hook_position) {
			return CreateHook(&guid_function, &hook_entry, &hook_position);
		}
		/// <summary>Delete a hook.</summary>
		/// <param name="guid_function">The GUID of the hookable function.</param>
		/// <param name="guid_hook">The GUID of the hook to be deleted.</param>
		static void DeleteHook(_In_ const GUID* guid_function, _In_ const GUID* guid_hook) {
			if (!guid_function) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 0);
			if (!guid_hook) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(HookableFunction, 1);
			return Raw::RawProcedure::DeleteHook(guid_function, guid_hook);
		}
		template <typename _Classty>
		HookableFunction(_In_ const GUID* guid, _Inout_opt_ void* context_func, _In_ const HookEntry<_Classty>* hook_entry_main)
			: own_rawproc(true), rawproc(new Raw::RawProcedure(guid, reinterpret_cast<void*>(this))), context_func(context_func) {
			HookPosition hook_position_main(at_front);
			this->rawproc->CreateHook(hook_entry_main->GetRawEntry(), &hook_position_main);
			MemoryBarrier();
			this->constructed = true;
		}
		HookableFunction(const HookableFunction&) = delete;
		HookableFunction(HookableFunction&&) = delete;
		~HookableFunction() {
			if (own_rawproc) delete rawproc;
		}
		/// <summary>Get the GUID of this hookable function.</summary>
		/// <returns>
		/// A pointer to the GUID of this hookable function.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		const GUID* GetGuid() { return this->rawproc->GetGuid(); }
		/// <summary>Get the context value associated with this hookable procedure.</summary>
		/// <returns>The context value associated with this hookable procedure.</returns>
		void* GetProcedureContext() const { return this->context_proc; }
		template <typename _Classty>
		void CreateHook(_In_ const HookEntry<_Classty>* hook_entry, _In_ const HookPosition* hook_position) {
			if (!hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!hook_position) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			return this->rawproc->CreateHook(hook_entry->GetRawEntry(), hook_position);
		}
		template <typename _Classty>
		FORCEINLINE void CreateHook(_In_ const HookEntry<_Classty>& hook_entry, _In_ const HookPosition& hook_position) {
			return this->CreateHook(&hook_entry, &hook_position);
		}
		void DeleteHook(_In_ const GUID* guid_hook) {
			if (!guid_hook) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			return this->rawproc->DeleteHook(guid_hook);
		}
		/// <summary>Invoke all inserted hooks associated with this function.</summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <returns>
		/// The final return value set by an invoked hook.
		/// The caller should delete the returned object when it's no longer useful.
		/// The caller may pass it to a managed pointer object.
		/// </returns>
		ReturnType* __thiscall Invoke(_Inout_opt_ ParamType* param) {
			InvocationPacket packet(param);
			this->rawproc->Invoke(reinterpret_cast<void*>(&packet));
			return packet.ReleaseReturn();
		}
		/// <summary>Invoke all inserted hooks associated with this function.</summary>
		/// <param name="packet">The invocation packet to be passed to the hooks.</param>
		void __thiscall Invoke(_Inout_ InvocationPacket* packet) {
			this->rawproc->Invoke(reinterpret_cast<void*>(packet));
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this function.
		/// The hooks that are after (not including) the one identified by <paramref name="guid_hook_ref" /> are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		/// <returns>
		/// The final return value set by an invoked hook.
		/// The caller should delete the returned object when it's no longer useful.
		/// The caller may pass it to a managed pointer object.
		/// </returns>
		ReturnType* __thiscall InvokePartial(_Inout_opt_ ParamType* param, _In_opt_ const GUID* guid_hook_ref, after_not_including_t) {
			InvocationPacket packet(param);
			this->rawproc->InvokePartial(reinterpret_cast<void*>(&packet), guid_hook_ref, after_not_including);
			return packet.ReleaseReturn();
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this function.
		/// The hooks that are after (not including) the one identified by <paramref name="guid_hook_ref" /> are invoked.
		/// </summary>
		/// <param name="packet">The invocation packet to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		void __thiscall InvokePartial(_Inout_ InvocationPacket* packet, _In_opt_ const GUID* guid_hook_ref, after_not_including_t) {
			this->rawproc->InvokePartial(reinterpret_cast<void*>(packet), guid_hook_ref, after_not_including);
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this function.
		/// The hook identified by <paramref name="guid_hook_ref" /> and the hooks that are after it are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		/// <returns>
		/// The final return value set by an invoked hook.
		/// The caller should delete the returned object when it's no longer useful.
		/// The caller may pass it to a managed pointer object.
		/// </returns>
		ReturnType* __thiscall InvokePartial(_Inout_opt_ ParamType* param, _In_opt_ const GUID* guid_hook_ref, after_including_t) {
			InvocationPacket packet(param);
			this->rawproc->InvokePartial(reinterpret_cast<void*>(&packet), guid_hook_ref, after_including);
			return packet.ReleaseReturn();
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this function.
		/// The hook identified by <paramref name="guid_hook_ref" /> and the hooks that are after it are invoked.
		/// </summary>
		/// <param name="packet">The invocation packet to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		void __thiscall InvokePartial(_Inout_ InvocationPacket* packet, _In_opt_ const GUID* guid_hook_ref, after_including_t) {
			this->rawproc->InvokePartial(reinterpret_cast<void*>(packet), guid_hook_ref, after_including);
		}
	protected:
		bool constructed = false;
		const bool own_rawproc = false;
		Raw::RawProcedure* const rawproc = nullptr;
		void* const context_func = nullptr;
		HookableFunction(_In_ Raw::RawProcedure* rawproc, _Inout_opt_ void* context_func)
			: own_rawproc(false), rawproc(rawproc), context_func(context_func) {
			MemoryBarrier();
			this->constructed = true;
		}
	};

	/// <summary>A hookable procedure that does not have any hooks when it is just created.</summary>
	/// <remarks>Objects of this class are thread-safe. Callers don't need to synchronize the calls.</remarks>
	/// <typeparam name="_Paramty">The type of the parameter of the procedure.</typeparam>
	template <typename _Paramty>
	class PureHookableProcedure final {
	public:
		using ParamType = _Paramty;
		class InvocationPacket final {
		public:
			using ParamType = ParamType;
			explicit InvocationPacket(_Inout_opt_ ParamType* param)
				: param(param) {}
			InvocationPacket(const InvocationPacket&) = delete;
			InvocationPacket(InvocationPacket&&) = delete;
			~InvocationPacket() {
				if (this->param) {
					delete this->param;
					this->param = nullptr;
				}
			}
			/// <summary>Get the parameter passed from the invoker.</summary>
			/// <returns>
			/// A pointer to the parameter.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			ParamType* GetParam() const { return this->param; }
			inline bool IsAdditionalDataEntryPresent(_In_ const GUID* guid) { return this->add_data_helper.IsEntryPresent(guid); }
			FORCEINLINE bool IsAdditionalDataEntryPresent(_In_ const GUID& guid) { return this->IsAdditionalDataEntryPresent(&guid); }
			inline uintptr_t GetAdditionalDataValue(_In_ const GUID* guid) { return this->add_data_helper.GetEntryValue(guid); }
			FORCEINLINE uintptr_t GetAdditionalDataValue(_In_ const GUID& guid) { return this->GetAdditionalDataValue(&guid); }
			inline void SetAdditionalDataValue(_In_ const GUID* guid, _In_opt_ uintptr_t value) { return this->add_data_helper.SetEntryValue(guid, value); }
			FORCEINLINE void SetAdditionalDataValue(_In_ const GUID& guid, _In_opt_ uintptr_t value) { return this->SetAdditionalDataValue(&guid, value); }
			inline void DeleteAdditionalDataEntry(_In_ const GUID* guid) { return this->add_data_helper.DeleteEntry(guid); }
			FORCEINLINE void DeleteAdditionalDataEntry(_In_ const GUID& guid) { return this->DeleteAdditionalDataEntry(&guid); }
		protected:
			ParamType * param = nullptr;
			Internal::InvocationPacketAdditionalDataHelper add_data_helper;
		};
		/// <summary>A hook entry of which the hook function and the cleanup function are non-static member functions of a class.</summary>
		/// <typeparam name="_Classty">The class that contains the hook function and the cleanup function.</typeparam>
		template <typename _Classty>
		class HookEntry final {
		public:
			using ParamType = ParamType;
			using ClassType = _Classty;
			typedef HookRet(__thiscall ClassType:: * HookFnptrType)(_In_ InvocationPacket* packet, _Inout_opt_ void* context, _Inout_ PureHookableProcedure* procedure, _Inout_ HookEntry* hook_entry);
			typedef void(__thiscall ClassType:: * HookCleanupFnptrType)(_Inout_opt_ void* context, _In_ const GUID* guid_hook);
			typedef HookRet(__thiscall ClassType::*LiteHookFnptrType)(_In_ InvocationPacket* packet);
			/// <summary>Create a hook entry.</summary>
			/// <param name="guid">
			/// The GUID of this hook entry.
			/// No other hook entries in the same hookable procedure may have an identical GUID.
			/// </param>
			/// <param name="hook_fnptr">A pointer to the member function that is called when the hook is invoked.</param>
			/// <param name="cleanup_fnptr">
			/// A pointer to the member function that is called when the hook entry is being deleted.
			/// If this hook entry is copied, the function is called only when the last instance of the hook entry is being deleted.
			/// </param>
			/// <param name="obj">
			/// A pointer to the object that's used as the <c>this</c> pointer when calling <paramref name="hook_fnptr" /> and <paramref name="cleanup_fnptr" />.
			/// The hook entry itself will not delete the object pointed to by it.
			/// However, the member function pointed by <paramref name="cleanup_fnptr" /> may delete it using <c>delete this</c>.
			/// </param>
			/// <param name="context">An optional context value associated with this hook entry.</param>
			HookEntry(_In_ const GUID* guid, _In_opt_ HookFnptrType hook_fnptr, _In_opt_ HookCleanupFnptrType cleanup_fnptr, _In_ ClassType* obj, _Inout_opt_ void* context) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				if (!obj) THROW_INVALID_PARAMETER_EXCEPTION_THIS(3);
				ctx_t* ctx = new ctx_t();
				ctx->hook_fnptr = hook_fnptr;
				ctx->cleanup_fnptr = cleanup_fnptr;
				ctx->obj = obj;
				ctx->context = context;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->obj || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						PureHookableProcedure* procedure = reinterpret_cast<PureHookableProcedure*>(rawproc->GetProcedureContext());
						if (!procedure || !procedure->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->hook_fnptr)
							return (ctx->obj->*(ctx->hook_fnptr))(packet, ctx->context, procedure, ctx->hook_entry);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx || !ctx->obj || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->cleanup_fnptr)
							(ctx->obj->*(ctx->cleanup_fnptr))(ctx->context, guid_hook);
						delete ctx->hook_entry;
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			/// <summary>Create a hook entry.</summary>
			/// <param name="guid">
			/// The GUID of this hook entry.
			/// No other hook entries in the same hookable function may have an identical GUID.
			/// </param>
			/// <param name="lite_hook_fnptr">A pointer to the member function that is called when the hook is invoked.</param>
			/// <param name="obj">
			/// A pointer to the object that's used as the <c>this</c> pointer when calling <paramref name="hook_fnptr" /> and <paramref name="cleanup_fnptr" />.
			/// The hook entry itself will not delete the object pointed to by it.
			/// However, the member function pointed by <paramref name="cleanup_fnptr" /> may delete it using <c>delete this</c>.
			/// </param>
			HookEntry(_In_ const GUID* guid, _In_opt_ LiteHookFnptrType lite_hook_fnptr, _In_ ClassType* obj) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				if (!obj) THROW_INVALID_PARAMETER_EXCEPTION_THIS(2);
				ctx_t* ctx = new ctx_t();
				ctx->lite_hook_fnptr = lite_hook_fnptr;
				ctx->obj = obj;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->obj || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						PureHookableProcedure* procedure = reinterpret_cast<PureHookableProcedure*>(rawproc->GetProcedureContext());
						if (!procedure || !procedure->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->lite_hook_fnptr)
							return (ctx->obj->*(ctx->lite_hook_fnptr))(packet);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx || !ctx->obj) THROW_UNEXPECTED_ERROR_EXCEPTION();
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			HookEntry(const HookEntry& t)
				: raw_entry(t.raw_entry ? new Raw::RawHookEntry(*t.raw_entry) : nullptr) {}
			HookEntry(HookEntry&& t)
				: raw_entry(t.raw_entry) {
				t.raw_entry = nullptr;
			}
			~HookEntry() {
				if (raw_entry) {
					delete raw_entry;
					raw_entry = nullptr;
				}
			}
			/// <summary>Get the GUID of this hook entry.</summary>
			/// <returns>
			/// A pointer to the GUID of this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const GUID* GetGuid() const { return this->raw_entry.GetGuid(); }
			/// <summary>Get a pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry of this hook entry.</summary>
			/// <returns>
			/// A pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const Raw::RawHookEntry* GetRawEntry() const { return this->raw_entry; }
		protected:
			struct ctx_t final {
				union {
					HookFnptrType hook_fnptr = nullptr;
					LiteHookFnptrType lite_hook_fnptr;
				};
				HookCleanupFnptrType cleanup_fnptr = nullptr;
				ClassType* obj = nullptr;
				void* context = nullptr;
				HookEntry* hook_entry = nullptr;
				explicit ctx_t() {}
				ctx_t(const ctx_t&) = delete;
				ctx_t(ctx_t&&) = delete;
			};
			Raw::RawHookEntry* raw_entry = nullptr;
		};
		/// <summary>A hook entry of which the hook function and the cleanup function are non-member functions or static member functions.</summary>
		template <>
		class HookEntry<void> final {
		public:
			using ParamType = ParamType;
			typedef HookRet(__stdcall * HookFnptrType)(_In_ InvocationPacket* packet, _Inout_opt_ void* context, _Inout_ PureHookableProcedure* procedure, _Inout_ HookEntry* hook_entry);
			typedef void(__stdcall * HookCleanupFnptrType)(_Inout_opt_ void* context, _In_ const GUID* guid_hook);
			typedef HookRet(__stdcall *LiteHookFnptrType)(_In_ InvocationPacket* packet);
			/// <summary>Create a hook entry.</summary>
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
			HookEntry(_In_ const GUID* guid, _In_opt_ HookFnptrType hook_fnptr, _In_opt_ HookCleanupFnptrType cleanup_fnptr, _Inout_opt_ void* context) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				ctx_t* ctx = new ctx_t();
				ctx->hook_fnptr = hook_fnptr;
				ctx->cleanup_fnptr = cleanup_fnptr;
				ctx->context = context;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						PureHookableProcedure* procedure = reinterpret_cast<PureHookableProcedure*>(rawproc->GetProcedureContext());
						if (!procedure || !procedure->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->hook_fnptr)
							return (*ctx->hook_fnptr)(packet, ctx->context, procedure, ctx->hook_entry);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->cleanup_fnptr)
							(*ctx->cleanup_fnptr)(ctx->context, guid_hook);
						delete ctx->hook_entry;
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			/// <summary>Create a hook entry.</summary>
			/// <param name="guid">
			/// The GUID of this hook entry.
			/// No other hook entries in the same hookable function may have an identical GUID.
			/// </param>
			/// <param name="lite_hook_fnptr">A pointer to the function that is called when the hook is invoked.</param>
			HookEntry(_In_ const GUID* guid, _In_opt_ LiteHookFnptrType lite_hook_fnptr) {
				if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
				ctx_t* ctx = new ctx_t();
				ctx->lite_hook_fnptr = lite_hook_fnptr;
				this->raw_entry = new Raw::RawHookEntry(guid,
					[](_In_opt_ void* rawparam, _Inout_opt_ void* rawcontext, _Inout_ Raw::RawProcedure* rawproc, _Inout_ Raw::RawHookEntry* raw_hook_entry)->HookRet {
						UNREFERENCED_PARAMETER(raw_hook_entry);
						InvocationPacket* packet = reinterpret_cast<InvocationPacket*>(rawparam);
						if (!packet) THROW_UNEXPECTED_ERROR_EXCEPTION();
						ctx_t* ctx = reinterpret_cast<ctx_t*>(rawcontext);
						if (!ctx || !ctx->hook_entry) THROW_UNEXPECTED_ERROR_EXCEPTION();
						PureHookableProcedure* procedure = reinterpret_cast<PureHookableProcedure*>(rawproc->GetProcedureContext());
						if (!procedure || !procedure->constructed) THROW_UNEXPECTED_ERROR_EXCEPTION();
						if (ctx->lite_hook_fnptr)
							return (*ctx->lite_hook_fnptr)(packet);
						else
							return HookRet_Continue;
					},
					[](_Inout_opt_ void* context, _In_ const GUID* guid_hook) {
						UNREFERENCED_PARAMETER(guid_hook);
						ctx_t* ctx = reinterpret_cast<ctx_t*>(context);
						if (!ctx) THROW_UNEXPECTED_ERROR_EXCEPTION();
						delete ctx;
						ctx = nullptr;
					}, reinterpret_cast<void*>(ctx));
				ctx->hook_entry = new HookEntry(*this);
			}
			HookEntry(const HookEntry& t)
				: raw_entry(t.raw_entry ? new Raw::RawHookEntry(*t.raw_entry) : nullptr) {}
			HookEntry(HookEntry&& t)
				: raw_entry(t.raw_entry) {
				t.raw_entry = nullptr;
			}
			~HookEntry() {
				if (raw_entry) {
					delete raw_entry;
					raw_entry = nullptr;
				}
			}
			/// <summary>Get the GUID of this hook entry.</summary>
			/// <returns>
			/// A pointer to the GUID of this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const GUID* GetGuid() const { return this->raw_entry.GetGuid(); }
			/// <summary>Get a pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry of this hook entry.</summary>
			/// <returns>
			/// A pointer to the <c>Raw::RawHookEntry</c> object that resembles this hook entry.
			/// It's guranteed that the returned pointer will not be an empty pointer.
			/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
			/// </returns>
			const Raw::RawHookEntry* GetRawEntry() const { return this->raw_entry; }
		protected:
			struct ctx_t final {
				union {
					HookFnptrType hook_fnptr = nullptr;
					LiteHookFnptrType lite_hook_fnptr;
				};
				HookCleanupFnptrType cleanup_fnptr = nullptr;
				void* context = nullptr;
				HookEntry* hook_entry = nullptr;
				explicit ctx_t() {}
				ctx_t(const ctx_t&) = delete;
				ctx_t(ctx_t&&) = delete;
			};
			Raw::RawHookEntry* raw_entry = nullptr;
		};
		/// <summary>Create a pure hookable procedure.</summary>
		/// <param name="guid_procedure">
		/// The GUID of the hookable procedure to be created.
		/// No other hookable functions or hookable procedures may have an identical GUID.
		/// </param>
		/// <param name="context_procedure">An optional context value associated with the procedure.</param>
		/// <returns>
		/// A pointer to the newly-created hookable procedure.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		static PureHookableProcedure* CreateProcedure(_In_ const GUID* guid_procedure, _Inout_opt_ void* context_proc) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 0);
			void* mem_procedure = malloc(sizeof(PureHookableProcedure));
			if (!mem_procedure) THROW_EXTERNAL_API_ERROR_EXCEPTION_NOLIB("malloc");
			memset(mem_procedure, 0, sizeof(PureHookableProcedure));
			reinterpret_cast<PureHookableProcedure*>(mem_procedure)->constructed = false;
			MemoryBarrier();
			Raw::RawProcedure* rawproc = Raw::RawProcedure::CreateProcedure(guid_procedure, reinterpret_cast<void*>(reinterpret_cast<PureHookableProcedure*>(mem_procedure)));
			return new(mem_procedure) PureHookableProcedure(rawproc, context_proc);
		}
		/// <summary>Get the pointer to an existing pure hookable procedure.</summary>
		/// <param name="guid_procedure">The GUID of the hookable procedure.</param>
		/// <returns>
		/// A pointer to the hookable procedure.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		static PureHookableProcedure* GetProcedure(_In_ const GUID* guid_procedure) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 0);
			Raw::RawProcedure* rawproc = Raw::RawProcedure::GetProcedure(guid_procedure);
			PureHookableProcedure* procedure = reinterpret_cast<PureHookableProcedure*>(rawproc->GetProcedureContext());
			if (!procedure || !procedure->constructed) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 0);
			return procedure;
		}
		/// <summary>Create a hook.</summary>
		/// <param name="guid_procedure">The GUID of the hookable procedure.</param>
		/// <param name="hook_entry">
		/// The hook entry.
		/// This function copies the hook entry and keeps an internal storage of it until the hook is deleted.
		/// </param>
		/// <param name="hook_position">The hook position at which to create the hook.</param>
		template <typename _Classty>
		static void CreateHook(_In_ const GUID* guid_procedure, _In_ const HookEntry<_Classty>* hook_entry, _In_ const HookPosition* hook_position) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 0);
			if (!hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 1);
			if (!hook_position) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 2);
			return Raw::RawProcedure::CreateHook(guid_procedure, hook_entry->GetRawEntry(), hook_position);
		}
		/// <summary>Create a hook.</summary>
		/// <param name="guid_procedure">The GUID of the hookable procedure.</param>
		/// <param name="hook_entry">
		/// The hook entry.
		/// This function copies the hook entry and keeps an internal storage of it until the hook is deleted.
		/// </param>
		/// <param name="hook_position">The hook position at which to create the hook.</param>
		template <typename _Classty>
		FORCEINLINE static void CreateHook(_In_ const GUID& guid_procedure, _In_ const HookEntry<_Classty>& hook_entry, _In_ const HookPosition& hook_position) {
			return CreateHook(&guid_procedure, &hook_entry, &hook_position);
		}
		/// <summary>Delete a hook.</summary>
		/// <param name="guid_procedure">The GUID of the hookable procedure.</param>
		/// <param name="guid_hook">The GUID of the hook to be deleted.</param>
		static void DeleteHook(_In_ const GUID* guid_procedure, _In_ const GUID* guid_hook) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 0);
			if (!guid_hook) THROW_INVALID_PARAMETER_EXCEPTION_CLASS(PureHookableProcedure, 1);
			return Raw::RawProcedure::DeleteHook(guid_procedure, guid_hook);
		}
		PureHookableProcedure(_In_ const GUID* guid, _Inout_opt_ void* context_proc)
			: own_rawproc(true), rawproc(new Raw::RawProcedure(guid, reinterpret_cast<void*>(this))), context_proc(context_proc) {
			MemoryBarrier();
			this->constructed = true;
		}
		PureHookableProcedure(const PureHookableProcedure&) = delete;
		PureHookableProcedure(PureHookableProcedure&&) = delete;
		~PureHookableProcedure() {
			if (own_rawproc) delete rawproc;
		}
		/// <summary>Get the GUID of this hookable procedure.</summary>
		/// <returns>
		/// A pointer to the GUID of this hookable procedure.
		/// It's guranteed that the returned pointer will not be an empty pointer.
		/// The caller should NOT delete the returned object, NOR should the caller pass it to a managed pointer object.
		/// </returns>
		const GUID* GetGuid() { return this->rawproc->GetGuid(); }
		/// <summary>Get the context value associated with this hookable procedure.</summary>
		/// <returns>The context value associated with this hookable procedure.</returns>
		void* GetProcedureContext() const { return this->context_proc; }
		template <typename _Classty>
		void CreateHook(_In_ const HookEntry<_Classty>* hook_entry, _In_ const HookPosition* hook_position) {
			if (!hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!hook_position) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			return this->rawproc->CreateHook(hook_entry->GetRawEntry(), hook_position);
		}
		template <typename _Classty>
		FORCEINLINE void CreateHook(_In_ const HookEntry<_Classty>& hook_entry, _In_ const HookPosition& hook_position) {
			return this->CreateHook(&hook_entry, &hook_position);
		}
		void DeleteHook(_In_ const GUID* guid_hook) {
			if (!guid_hook) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			return this->rawproc->DeleteHook(guid_hook);
		}
		/// <summary>Invoke all inserted hooks associated with this procedure.</summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		void __thiscall Invoke(_Inout_opt_ ParamType* param) {
			InvocationPacket packet(param);
			this->rawproc->Invoke(reinterpret_cast<void*>(&packet));
		}
		/// <summary>Invoke all inserted hooks associated with this procedure.</summary>
		/// <param name="packet">The invocation packet to be passed to the hooks.</param>
		void __thiscall Invoke(_Inout_ InvocationPacket* packet) {
			this->rawproc->Invoke(reinterpret_cast<void*>(packet));
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this procedure.
		/// The hooks that are after (not including) the one identified by <paramref name="guid_hook_ref" /> are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		void __thiscall InvokePartial(_Inout_opt_ ParamType* param, _In_opt_ const GUID* guid_hook_ref, after_not_including_t) {
			InvocationPacket packet(param);
			this->rawproc->InvokePartial(reinterpret_cast<void*>(&packet), guid_hook_ref, after_not_including);
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this procedure.
		/// The hooks that are after (not including) the one identified by <paramref name="guid_hook_ref" /> are invoked.
		/// </summary>
		/// <param name="packet">The invocation packet to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		void __thiscall InvokePartial(_Inout_ InvocationPacket* packet, _In_opt_ const GUID* guid_hook_ref, after_not_including_t) {
			this->rawproc->InvokePartial(reinterpret_cast<void*>(packet), guid_hook_ref, after_not_including);
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this procedure.
		/// The hook identified by <paramref name="guid_hook_ref" /> and the hooks that are after it are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		void __thiscall InvokePartial(_Inout_opt_ ParamType* param, _In_opt_ const GUID* guid_hook_ref, after_including_t) {
			InvocationPacket packet(param);
			this->rawproc->InvokePartial(reinterpret_cast<void*>(&packet), guid_hook_ref, after_including);
		}
		/// <summary>
		/// Invoke some inserted hooks associated with this procedure.
		/// The hook identified by <paramref name="guid_hook_ref" /> and the hooks that are after it are invoked.
		/// </summary>
		/// <param name="packet">The invocation packet to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		void __thiscall InvokePartial(_Inout_ InvocationPacket* packet, _In_opt_ const GUID* guid_hook_ref, after_including_t) {
			this->rawproc->InvokePartial(reinterpret_cast<void*>(packet), guid_hook_ref, after_including);
		}
	protected:
		bool constructed = false;
		const bool own_rawproc = false;
		Raw::RawProcedure* const rawproc = nullptr;
		void* const context_proc = nullptr;
		PureHookableProcedure(_In_ Raw::RawProcedure* rawproc, _Inout_opt_ void* context_proc)
			: own_rawproc(false), rawproc(rawproc), context_proc(context_proc) {
			MemoryBarrier();
			this->constructed = true;
		}
	};
}
#endif
