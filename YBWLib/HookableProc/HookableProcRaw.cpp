#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <memory>
#include <algorithm>
#include <list>
#include <unordered_map>
#include <map>
#include <mutex>
#include <guiddef.h>
#include "..\misc\pimpl_helper.h"
#include "..\exception\Exception.h"
#include "..\misc\guid_helper.h"
#include "HookableProc_exception.h"
#include "HookableProcCommon.h"
#include "HookableProcCommon_internal.h"
#include "HookableProcRaw.h"

namespace HookableProc::Raw {
	class _impl_RawHookEntry final {
	public:
		RawHookEntry* pdecl = nullptr;
		struct cleanup_obj_t final {
			void* context = nullptr;
			RawHookCleanupFnptrType cleanup_fnptr = nullptr;
			GUID guid_hook = GUID_NULL;
			cleanup_obj_t(_Inout_opt_ void* context, _In_opt_ RawHookCleanupFnptrType cleanup_fnptr, _In_ const GUID& guid_hook)
				: context(context), cleanup_fnptr(cleanup_fnptr), guid_hook(guid_hook) {}
			cleanup_obj_t(const cleanup_obj_t&) = delete;
			cleanup_obj_t(cleanup_obj_t&&) = delete;
			inline ~cleanup_obj_t() {
				if (this->cleanup_fnptr) {
					this->cleanup_fnptr(this->context, &guid_hook);
					this->context = nullptr;
				}
			}
		};
		const GUID guid = GUID_NULL;
		RawHookFnptrType hook_fnptr = nullptr;
		std::shared_ptr<cleanup_obj_t> p_cleanup_obj;
		_impl_RawHookEntry(_In_ const GUID& guid, _In_opt_ RawHookFnptrType hook_fnptr, _In_opt_ RawHookCleanupFnptrType cleanup_fnptr, _Inout_opt_ void* context)
			: guid(guid), hook_fnptr(hook_fnptr), p_cleanup_obj(new cleanup_obj_t(context, cleanup_fnptr, guid)) {}
		_impl_RawHookEntry(const _impl_RawHookEntry& t)
			: guid(t.guid), hook_fnptr(t.hook_fnptr), p_cleanup_obj(t.p_cleanup_obj) {}
		_impl_RawHookEntry(_impl_RawHookEntry&& t)
			: guid(t.guid), hook_fnptr(t.hook_fnptr), p_cleanup_obj(t.p_cleanup_obj) {}
		const GUID* GetGuid() const { return &this->guid; }
		HookRet InvokeHook(_In_opt_ void* param, _Inout_ RawProcedure* procedure) const {
			if (!procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			if (this->hook_fnptr) {
				if (!this->p_cleanup_obj) THROW_UNEXPECTED_ERROR_EXCEPTION();
				try {
					try {
						return this->hook_fnptr(param, this->p_cleanup_obj->context, procedure, this->pdecl);
					} catch (HookableProcHookException& err) {
						++err.CountShouldPassThroughHookTable;
						throw;
					}
				} catch (HookableProcException& err) {
					++err.CountShouldPassThroughProcedure;
					throw;
				}
			} else {
				return HookRet_Continue;
			}
		}
	};

	class RawHookTable final {
	public:
		RawHookTable()
			: node_root(new node_t()) {
			this->node_root->is_inserted = true;
		}
		/// <summary>Create a hook.</summary>
		/// <param name="hook_entry">
		/// The hook entry.
		/// This function copies the hook entry and keeps an internal storage of it until the hook is deleted.
		/// </param>
		/// <param name="hook_position">The hook position at which to create the hook.</param>
		void CreateHook(_In_ const RawHookEntry* hook_entry, _In_ const _impl_HookPosition* hook_position) {
			if (!hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!hook_position) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			this->ProcessHookableProcHookException(hook_entry->GetGuid(), [&]() {
				// Check for existing hook that has the specified GUID.
				if (this->guid_map.count(*hook_entry->GetGuid())) throw(HookAlreadyExistException());
				// Create node.
				std::shared_ptr<node_t> node_new(new node_t(*hook_entry, *hook_position));
				// Insert node.
				this->InsertHook(&node_new);
				if (node_new->is_inserted) {
					// This node is not awaiting insert.
					// Check which nodes that were awaiting insert should be inserted, (possibly indirectly) because of this newly-created node.
					// Nodes that should be inserted, (possibly indirectly) because of this newly-created node.
					guid_map_t map_pending_insert;
					// Recursively add nodes that are dependent on previously added nodes.
					guid_map_t map_pending_insert_prev({ std::make_pair(*node_new->hook_entry->GetGuid(), node_new) });
					while (map_pending_insert_prev.size()) {
						guid_map_t map_pending_insert_new;
						std::for_each(map_pending_insert_prev.begin(), map_pending_insert_prev.end(),
							[this, &map_pending_insert_new](guid_map_t::value_type& val_map_pending_insert_prev) {
								const GUID* guid_dep_search = val_map_pending_insert_prev.second->hook_entry->GetGuid();
								// Search for nodes that are both awaiting insert and dependent on the GUID guid_dep_search.
								std::pair<map_dep_t::iterator, map_dep_t::iterator> range_dep_found = this->map_dep_awaiting_insert.equal_range(*guid_dep_search);
								std::for_each(range_dep_found.first, range_dep_found.second,
									[&map_pending_insert_new](map_dep_t::value_type& val_map_dep_found) {
										if (val_map_dep_found.second->is_inserted || val_map_dep_found.second->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
										map_pending_insert_new[*val_map_dep_found.second->hook_entry->GetGuid()] = val_map_dep_found.second;
									});
							});
						// Remove these nodes and add these nodes to map_pending_insert.
						std::for_each(map_pending_insert_new.begin(), map_pending_insert_new.end(),
							[this, &map_pending_insert](guid_map_t::value_type& val_map_pending_insert_new) {
								this->RemoveHookIgnoreChildren(&val_map_pending_insert_new.second);
								map_pending_insert.insert(val_map_pending_insert_new);
								val_map_pending_insert_new.second->is_pending_insert = true;
							});
						map_pending_insert_prev = map_pending_insert_new;
					}
					// Remove the initially inserted node, which is the newly-created node, from map_pending_insert.
					map_pending_insert.erase(*node_new->hook_entry->GetGuid());
					// Insert the nodes that are pending insert.
					std::for_each(map_pending_insert.begin(), map_pending_insert.end(),
						[this](guid_map_t::value_type& val_map_pending_insert) {
							this->InsertHook(&val_map_pending_insert.second);
						});
					std::for_each(map_pending_insert.begin(), map_pending_insert.end(),
						[](guid_map_t::value_type& val_map_pending_insert) {
							if (val_map_pending_insert.second->is_pending_insert) {
								std::shared_ptr<node_t> node_current(val_map_pending_insert.second);
								intptr_t dep_chain_length = 1;
								do {
									node_current = node_current->parent.lock();
									++dep_chain_length;
								} while (node_current && node_current->is_pending_insert && node_current != val_map_pending_insert.second);
								if (!node_current || !node_current->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
								GUID** guid_dep_chain = new GUID*[dep_chain_length + 1];
								node_current = val_map_pending_insert.second;
								for (intptr_t i = 0; i < dep_chain_length; ++i) {
									guid_dep_chain[i] = new GUID(*node_current->hook_entry->GetGuid());
									node_current = node_current->parent.lock();
								}
								guid_dep_chain[dep_chain_length] = nullptr;
								throw(HookCyclicDependencyException(guid_dep_chain));
							}
						});
					// Check which nodes that were awaiting move should be moved, (possibly indirectly) because of this newly-created node.
					// Nodes that should be moved, (possibly indirectly) because of this newly-created node.
					guid_map_t map_pending_move;
					std::for_each(map_pending_insert.begin(), map_pending_insert.end(),
						[this, &map_pending_move](guid_map_t::value_type& val_map_pending_insert) {
							const GUID* guid_dep_search = val_map_pending_insert.second->hook_entry->GetGuid();
							// Search for nodes that are both awaiting move and dependent on the GUID guid_dep_search.
							std::pair<map_dep_t::iterator, map_dep_t::iterator> range_dep_found = this->map_dep_awaiting_move.equal_range(*guid_dep_search);
							std::for_each(range_dep_found.first, range_dep_found.second,
								[&map_pending_move](map_dep_t::value_type& val_map_dep_found) {
									if (!val_map_dep_found.second->is_inserted || val_map_dep_found.second->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
									map_pending_move[*val_map_dep_found.second->hook_entry->GetGuid()] = val_map_dep_found.second;
								});
						});
					// Remove these nodes.
					std::for_each(map_pending_move.begin(), map_pending_move.end(),
						[this](guid_map_t::value_type& val_map_pending_move) {
							this->RemoveHookIgnoreChildren(&val_map_pending_move.second);
						});
					// Insert these nodes.
					std::for_each(map_pending_move.begin(), map_pending_move.end(),
						[this](guid_map_t::value_type& val_map_pending_move) {
							this->InsertHook(&val_map_pending_move.second);
						});
				}
				});
		}
		/// <summary>Delete a hook.</summary>
		/// <param name="guid_hook">The GUID of the hook to be deleted.</param>
		void DeleteHook(_In_ const GUID* guid_hook) {
			if (!guid_hook) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			this->ProcessHookableProcHookException(guid_hook, [&]() {
				std::shared_ptr<node_t> node = nullptr;
				try {
					node = this->guid_map.at(*guid_hook);
				} catch (std::out_of_range&) {
					throw (HookNotExistException());
				}
				if (!node) THROW_UNEXPECTED_ERROR_EXCEPTION();
				THROW_NOT_IMPLEMENTED_EXCEPTION();
				// TODO: Implement RawHookTable::DeleteHook.
				});
		}
		/// <summary>Invoke all hooks that have been inserted in the table.</summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
		void Invoke(_In_opt_ void* param, _Inout_ RawProcedure* procedure) {
			if (!procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			this->node_root->Invoke(param, procedure);
		}
		/// <summary>
		/// Invoke some of the hooks that have been inserted in the table.
		/// The hooks that are after (not including) the one identified by <paramref name="guid_hook_ref" /> are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
		void InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_not_including_t, _Inout_ RawProcedure* procedure) {
			if (!procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(3);
			std::shared_ptr<node_t> node_ref = nullptr;
			this->ProcessHookableProcHookException(guid_hook_ref, [&]() {
				if (guid_hook_ref) {
					try {
						node_ref = guid_map.at(*guid_hook_ref);
					} catch (std::out_of_range&) {
						throw (HookNotExistException());
					}
				} else {
					node_ref = this->node_root;
				}
				});
			if (!node_ref) THROW_UNEXPECTED_ERROR_EXCEPTION();
			if (!node_ref->is_inserted || node_ref->is_pending_insert) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			node_ref->InvokePartialAfterMe(param, procedure);
		}
		/// <summary>
		/// Invoke some of the hooks that have been inserted in the table.
		/// The hook identified by <paramref name="guid_hook_ref" /> and the hooks that are after it are invoked.
		/// </summary>
		/// <param name="param">An optional parameter to be passed to the hooks.</param>
		/// <param name="guid_hook_ref">
		/// The GUID of the hook.
		/// If an empty pointer is passed, the root node is specifed.
		/// </param>
		/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
		void InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_including_t, _Inout_ RawProcedure* procedure) {
			if (!procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(3);
			std::shared_ptr<node_t> node_ref = nullptr;
			this->ProcessHookableProcHookException(guid_hook_ref, [&]() {
				if (guid_hook_ref) {
					try {
						node_ref = guid_map.at(*guid_hook_ref);
					} catch (std::out_of_range&) {
						throw (HookNotExistException());
					}
				} else {
					node_ref = this->node_root;
				}
				});
			if (!node_ref) THROW_UNEXPECTED_ERROR_EXCEPTION();
			if (!node_ref->is_inserted || node_ref->is_pending_insert) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			switch (node_ref->InvokeNoPropagate(param, procedure)) {
			case HookRet_Continue:
				this->InvokePartial(param, guid_hook_ref, after_not_including, procedure);
				break;
			case HookRet_Yield:
				break;
			default:
				THROW_UNEXPECTED_ERROR_EXCEPTION();
			}
		}
	protected:
		struct node_t {
			std::weak_ptr<node_t> parent;
			std::list<std::shared_ptr<node_t> > children_before;
			std::list<std::shared_ptr<node_t> > children_after;
			bool is_inserted = false;
			bool is_pending_insert = false;
			bool is_awaiting_move = false;
			std::unique_ptr<RawHookEntry> hook_entry;
			std::unique_ptr<_impl_HookPosition> hook_position;
			node_t()
				: parent(), children_before(), children_after(), hook_entry(), hook_position() {}
			node_t(const RawHookEntry& hook_entry, const _impl_HookPosition& hook_position)
				: children_before(), children_after(), hook_entry(new RawHookEntry(hook_entry)), hook_position(new _impl_HookPosition(hook_position)) {}
			node_t(const node_t&) = delete;
			node_t(node_t&& t) = delete;
			/// <summary>Invoke the hook entry associated with this node (if any).</summary>
			/// <param name="param">An optional parameter from the caller.</param>
			/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
			/// <returns>A <c>HookRet</c> value to tell the caller what further actions it should take after calling this function.</returns>
			HookRet InvokeNoPropagate(_In_opt_ void* param, _Inout_ RawProcedure* procedure) {
				if (!procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
				if (!this->is_inserted || this->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
				if (this->hook_entry) {
					return RawHookTable::ProcessHookableProcHookException(this->hook_entry->GetGuid(), [&]()->HookRet {
						struct hold_is_invoking_t {
							intptr_t* const p_count_is_invoking;
							const GUID** const p_guid_hook_invoking;
							const GUID* const guid_hook;
							const GUID* guid_hook_invoking_prev = nullptr;
							hold_is_invoking_t(intptr_t* p_count_is_invoking, const GUID** p_guid_hook_invoking, const GUID* guid_hook)
								: p_count_is_invoking(p_count_is_invoking), p_guid_hook_invoking(p_guid_hook_invoking), guid_hook(guid_hook) {
								++*p_count_is_invoking;
								guid_hook_invoking_prev = *p_guid_hook_invoking;
								*p_guid_hook_invoking = guid_hook;
							}
							hold_is_invoking_t(const hold_is_invoking_t&) = delete;
							hold_is_invoking_t(hold_is_invoking_t&&) = delete;
							~hold_is_invoking_t() {
								*p_guid_hook_invoking = guid_hook_invoking_prev;
								--*p_count_is_invoking;
							}
						} hold_is_invoking(procedure->GetPCountIsInvoking(), procedure->GetPGuidHookInvoking(), this->hook_entry->GetGuid());
						return this->hook_entry->InvokeHook(param, procedure);
						});
				} else {
					return HookRet_Continue;
				}
			}
			/// <summary>Invoke the hook entry associated with this node (if any) and all children of this node.</summary>
			/// <param name="param">An optional parameter from the caller.</param>
			/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
			/// <returns>A <c>HookRet</c> value to tell the caller what further actions it should take after calling this function.</returns>
			HookRet Invoke(_In_opt_ void* param, _Inout_ RawProcedure* procedure) {
				if (!this->is_inserted || this->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
				HookRet hook_ret = HookRet_Continue;
				if (hook_ret != HookRet_Yield) for (
					std::list<std::shared_ptr<node_t> >::iterator it_child = this->children_before.begin();
					it_child != this->children_before.end();
					++it_child
					) {
					if (!(*it_child)) THROW_UNEXPECTED_ERROR_EXCEPTION();
					switch ((*it_child)->Invoke(param, procedure)) {
					case HookRet_Continue:
						break;
					case HookRet_Yield:
						hook_ret = HookRet_Yield;
						break;
					default:
						THROW_UNEXPECTED_ERROR_EXCEPTION();
					}
					if (hook_ret == HookRet_Yield) break;
				}
				if (hook_ret != HookRet_Yield) switch (this->InvokeNoPropagate(param, procedure)) {
				case HookRet_Continue:
					break;
				case HookRet_Yield:
					hook_ret = HookRet_Yield;
					break;
				default:
					THROW_UNEXPECTED_ERROR_EXCEPTION();
				}
				if (hook_ret != HookRet_Yield) for (
					std::list<std::shared_ptr<node_t> >::iterator it_child = this->children_after.begin();
					it_child != this->children_after.end();
					++it_child
					) {
					if (!(*it_child)) THROW_UNEXPECTED_ERROR_EXCEPTION();
					switch ((*it_child)->Invoke(param, procedure)) {
					case HookRet_Continue:
						break;
					case HookRet_Yield:
						hook_ret = HookRet_Yield;
						break;
					default:
						THROW_UNEXPECTED_ERROR_EXCEPTION();
					}
					if (hook_ret == HookRet_Yield) break;
				}
				return hook_ret;
			}
			/// <summary>Invoke all hook entries after this node.</summary>
			/// <param name="param">An optional parameter from the caller.</param>
			/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
			/// <returns>A <c>HookRet</c> value to tell the caller what further actions it should take after calling this function.</returns>
			HookRet InvokePartialAfterMe(_In_opt_ void* param, _Inout_ RawProcedure* procedure) {
				if (!this->is_inserted || this->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
				HookRet hook_ret = HookRet_Continue;
				if (hook_ret != HookRet_Yield) for (
					std::list<std::shared_ptr<node_t> >::iterator it_child = this->children_after.begin();
					it_child != this->children_after.end();
					++it_child
					) {
					if (!(*it_child)) THROW_UNEXPECTED_ERROR_EXCEPTION();
					switch ((*it_child)->Invoke(param, procedure)) {
					case HookRet_Continue:
						break;
					case HookRet_Yield:
						hook_ret = HookRet_Yield;
						break;
					default:
						THROW_UNEXPECTED_ERROR_EXCEPTION();
					}
					if (hook_ret == HookRet_Yield) break;
				}
				if (hook_ret != HookRet_Yield) {
					std::shared_ptr<node_t> node_parent = this->parent.lock();
					if (node_parent) switch (node_parent->InvokePartialAfterChild(param, this, procedure)) {
					case HookRet_Continue:
						break;
					case HookRet_Yield:
						hook_ret = HookRet_Yield;
						break;
					default:
						THROW_UNEXPECTED_ERROR_EXCEPTION();
					}
				}
				return hook_ret;
			}
			/// <summary>Invoke all hook entries after the child node <paramref name="child_ref" />.</summary>
			/// <param name="param">An optional parameter from the caller.</param>
			/// <param name="child_ref">The child node.</param>
			/// <param name="procedure>The hookable procedure from which the invocation originated.</param>
			/// <returns>A <c>HookRet</c> value to tell the caller what further actions it should take after calling this function.</returns>
			HookRet InvokePartialAfterChild(_In_opt_ void* param, _In_ const node_t* child_ref, _Inout_ RawProcedure* procedure) {
				if (!child_ref) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
				if (!this->is_inserted || this->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
				HookRet hook_ret = HookRet_Continue;
				bool is_after_ref = false;
				if (hook_ret != HookRet_Yield) for (
					std::list<std::shared_ptr<node_t> >::iterator it_child = this->children_before.begin();
					it_child != this->children_before.end();
					++it_child
					) {
					if (!(*it_child)) THROW_UNEXPECTED_ERROR_EXCEPTION();
					if (is_after_ref) {
						switch ((*it_child)->Invoke(param, procedure)) {
						case HookRet_Continue:
							break;
						case HookRet_Yield:
							hook_ret = HookRet_Yield;
							break;
						default:
							THROW_UNEXPECTED_ERROR_EXCEPTION();
						}
					} else {
						if ((*it_child).get() == child_ref) is_after_ref = true;
					}
					if (hook_ret == HookRet_Yield) break;
				}
				if (hook_ret != HookRet_Yield) if (is_after_ref) {
					switch (this->InvokeNoPropagate(param, procedure)) {
					case HookRet_Continue:
						break;
					case HookRet_Yield:
						hook_ret = HookRet_Yield;
						break;
					default:
						THROW_UNEXPECTED_ERROR_EXCEPTION();
					}
				}
				if (hook_ret != HookRet_Yield) for (
					std::list<std::shared_ptr<node_t> >::iterator it_child = this->children_after.begin();
					it_child != this->children_after.end();
					++it_child
					) {
					if (!(*it_child)) THROW_UNEXPECTED_ERROR_EXCEPTION();
					if (is_after_ref) {
						switch ((*it_child)->Invoke(param, procedure)) {
						case HookRet_Continue:
							break;
						case HookRet_Yield:
							hook_ret = HookRet_Yield;
							break;
						default:
							THROW_UNEXPECTED_ERROR_EXCEPTION();
						}
					} else {
						if ((*it_child).get() == child_ref) is_after_ref = true;
					}
					if (hook_ret == HookRet_Yield) break;
				}
				if (hook_ret != HookRet_Yield) {
					std::shared_ptr<node_t> node_parent = this->parent.lock();
					if (node_parent) switch (node_parent->InvokePartialAfterChild(param, this, procedure)) {
					case HookRet_Continue:
						break;
					case HookRet_Yield:
						hook_ret = HookRet_Yield;
						break;
					default:
						THROW_UNEXPECTED_ERROR_EXCEPTION();
					}
				}
				return hook_ret;
			}
		};
		typedef std::unordered_map<GUID, std::shared_ptr<node_t>, hash_guid, equal_to_guid> guid_map_t;
		typedef std::unordered_multimap<GUID, std::shared_ptr<node_t>, hash_guid, equal_to_guid> map_dep_t;
		/// <summary>
		/// The root node.
		/// This node has no hook entry.
		/// </summary>
		std::shared_ptr<node_t> node_root;
		/// <summary>
		/// The map from the GUID of the hook entries to the nodes.
		/// </summary>
		guid_map_t guid_map;
		/// <summary>
		/// The map from the depended GUIDs to the nodes that are awaiting insert.
		/// </summary>
		map_dep_t map_dep_awaiting_insert;
		/// <summary>
		/// The map from the depended GUIDs to the nodes that are awaiting move.
		/// </summary>
		map_dep_t map_dep_awaiting_move;
		template <typename _Fn>
		static auto ProcessHookableProcHookException(_In_ const GUID* guid_hook, _Fn fn) {
			try {
				return fn();
			} catch (HookableProcHookException& err) {
				if (err.CountShouldPassThroughHookTable > 0) {
					--err.CountShouldPassThroughHookTable;
				} else {
					err.SetHookGuid(guid_hook);
				}
				throw;
			}
		}
		/// <summary>
		/// Try to insert the specified node at the specified hook position.
		/// This function adjusts the <c>is_inserted</c>, <c>is_pending_insert</c> & <c>is_awaiting_move</c> flags on the node.
		/// </summary>
		/// <param name="node">The node to be tried to insert.</param>
		/// <param name="hook_position">
		/// The hook position to try to insert at.
		/// If this parameter is <c>nullptr</c>, this function adds the dependencies of this node into <c>this->map_dep_awaiting_insert</c> and returns <c>true</c>.
		/// </param>
		/// <param name="map_dep">
		/// The dependency map of the node.
		/// The caller should create an empty map and preserve it in the process of inserting this node.
		/// However, the caller should not insert the content of this map into <c>this->map_dep_awaiting_insert</c> or <c>this->map_dep_awaiting_move</c>.
		/// </param>
		/// <returns>
		/// If the caller should continue trying for the failsafe hook position, the function returns <c>false</c>,
		/// otherwise it returns <c>true</c>.
		/// </returns>
		bool TryInsertHook(_In_ const std::shared_ptr<node_t>* node, _In_opt_ const _impl_HookPosition* hook_position, _Inout_ map_dep_t* map_dep) {
			if (!node || !*node || !(*node)->hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!map_dep) THROW_INVALID_PARAMETER_EXCEPTION_THIS(2);
			if (hook_position) {
				if (hook_position->ref) {
					std::shared_ptr<node_t> node_ref = nullptr;
					try {
						node_ref = this->guid_map.at(hook_position->guid_ref);
					} catch (std::out_of_range&) {}
					if (node_ref && (node_ref->is_inserted || node_ref->is_pending_insert)) {
						// This node will be inserted.
						(*node)->parent = node_ref;
						if (hook_position->side) {
							node_ref->children_after.push_back(*node);
						} else {
							node_ref->children_before.push_front(*node);
						}
						(*node)->is_inserted = true;
						if (node_ref->is_inserted && (*node)->is_pending_insert) {
							(*node)->is_pending_insert = false;
							std::for_each((*node)->children_before.begin(), (*node)->children_before.end(),
								[](std::list<std::shared_ptr<node_t> >::value_type& val_child) {
									val_child->is_inserted = true;
									val_child->is_pending_insert = false;
								});
							std::for_each((*node)->children_after.begin(), (*node)->children_after.end(),
								[](std::list<std::shared_ptr<node_t> >::value_type& val_child) {
									val_child->is_inserted = true;
									val_child->is_pending_insert = false;
								});
						}
						this->guid_map[*(*node)->hook_entry->GetGuid()] = *node;
						this->map_dep_awaiting_move.insert(map_dep->begin(), map_dep->end());
						return true;
					} else {
						// This node will be awaiting move, if it's inserted.
						(*node)->is_awaiting_move = true;
						map_dep->insert(std::make_pair(hook_position->guid_ref, *node));
						return false;
					}
				} else {
					// This node will be inserted.
					if (!this->node_root) THROW_UNEXPECTED_ERROR_EXCEPTION();
					(*node)->parent = this->node_root;
					if (hook_position->side) {
						this->node_root->children_after.push_back(*node);
					} else {
						this->node_root->children_before.push_front(*node);
					}
					(*node)->is_inserted = true;
					if ((*node)->is_pending_insert) {
						(*node)->is_pending_insert = false;
						std::for_each((*node)->children_before.begin(), (*node)->children_before.end(),
							[](std::list<std::shared_ptr<node_t> >::value_type& val_child) {
								val_child->is_inserted = true;
								val_child->is_pending_insert = false;
							});
						std::for_each((*node)->children_after.begin(), (*node)->children_after.end(),
							[](std::list<std::shared_ptr<node_t> >::value_type& val_child) {
								val_child->is_inserted = true;
								val_child->is_pending_insert = false;
							});
					}
					this->guid_map[*(*node)->hook_entry->GetGuid()] = *node;
					this->map_dep_awaiting_move.insert(map_dep->begin(), map_dep->end());
					return true;
				}
			} else {
				// This node will be awaiting insert.
				(*node)->is_inserted = false;
				(*node)->is_awaiting_move = false;
				this->guid_map[*(*node)->hook_entry->GetGuid()] = *node;
				this->map_dep_awaiting_insert.insert(map_dep->begin(), map_dep->end());
				return true;
			}
		}
		/// <summary>
		/// Insert the specified node.
		/// This function may make the specified node awaiting insert.
		/// </summary>
		/// <param name="node">The node to be inserted.</param>
		void InsertHook(_In_ const std::shared_ptr<node_t>* node) {
			if (!node || !*node || !(*node)->hook_entry || !(*node)->hook_position) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			const _impl_HookPosition* hook_position_current = nullptr;
			map_dep_t map_dep_new;
			for (
				hook_position_current = (*node)->hook_position.get();
				!this->TryInsertHook(node, hook_position_current, &map_dep_new);
				hook_position_current = hook_position_current->failsafe.get()
				) {
				// If hook_position_current is empty, this->TryInsertHook should return true.
				if (!hook_position_current) THROW_UNEXPECTED_ERROR_EXCEPTION();
			}
		}
		/// <summary>
		/// Remove all dependency entries of the specified node from the specified dependency map.
		/// </summary>
		/// <param name="node">The node of which the dependency entries are to be removed.</param>
		/// <param name="map_dep">The dependency map from which to remove the dependency entries.</param>
		void RemoveDependencyEntries(_In_ const std::shared_ptr<node_t>* node, _In_ map_dep_t* map_dep) {
			if (!node || !*node || !(*node)->hook_position) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!map_dep) THROW_INVALID_PARAMETER_EXCEPTION_THIS(2);
			const _impl_HookPosition* hook_position_current = nullptr;
			for (
				hook_position_current = (*node)->hook_position.get();
				hook_position_current;
				hook_position_current = hook_position_current->failsafe.get()
				) {
				if (hook_position_current->ref) {
					std::pair<map_dep_t::iterator, map_dep_t::iterator> range_dep_found = map_dep->equal_range(hook_position_current->guid_ref);
					map_dep_t::iterator it_map_dep_found = range_dep_found.first;
					while (it_map_dep_found != range_dep_found.second) {
						map_dep_t::iterator it_map_dep_found_temp = it_map_dep_found++;
						if ((*it_map_dep_found_temp).second == *node)
							map_dep->erase(it_map_dep_found_temp);
					}
				}
			}
		}
		/// <summary>
		/// Remove the node from its parent.
		/// This function adjusts the <c>is_inserted</c>, <c>is_pending_insert</c> & <c>is_awaiting_move</c> flags on the node.
		/// This function also removes its entry in the GUID map and all its dependency entries in the dependency map.
		/// This function does not delete the hook.
		/// </summary>
		/// <param name="node">The node to be removed.</param>
		void RemoveHookIgnoreChildren(_In_ const std::shared_ptr<node_t>* node) {
			if (!node || !*node || !(*node)->hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if ((*node)->is_pending_insert) THROW_UNEXPECTED_ERROR_EXCEPTION();
			if ((*node)->is_inserted) {
				this->RemoveDependencyEntries(node, &this->map_dep_awaiting_move);
				this->guid_map.erase(*(*node)->hook_entry->GetGuid());
				std::shared_ptr<node_t> node_parent((*node)->parent.lock());
				bool parent_has_removed_child = false;
				for (
					std::list<std::shared_ptr<node_t> >::iterator it_child = node_parent->children_before.begin();
					it_child != node_parent->children_before.end();
					++it_child
					) if (*it_child == *node) { node_parent->children_before.erase(it_child); parent_has_removed_child = true; break; }
				if (!parent_has_removed_child) for (
					std::list<std::shared_ptr<node_t> >::iterator it_child = node_parent->children_after.begin();
					it_child != node_parent->children_after.end();
					++it_child
					) if (*it_child == *node) { node_parent->children_after.erase(it_child); parent_has_removed_child = true; break; }
				if (!parent_has_removed_child) THROW_UNEXPECTED_ERROR_EXCEPTION();
				(*node)->parent.reset();
				(*node)->is_inserted = false;
				(*node)->is_awaiting_move = false;
			} else {
				this->RemoveDependencyEntries(node, &this->map_dep_awaiting_insert);
				this->guid_map.erase(*(*node)->hook_entry->GetGuid());
				(*node)->is_inserted = false;
				(*node)->is_awaiting_move = false;
			}
		}
	};

	class _impl_RawProcedure final {
	public:
		RawProcedure* pdecl = nullptr;
		const GUID guid = GUID_NULL;
		void* const context_procedure = nullptr;
		std::recursive_mutex mtx;
		intptr_t count_is_invoking = 0;
		const GUID* guid_hook_invoking = nullptr;
		RawHookTable hook_table;
		_impl_RawProcedure(const GUID& guid, _Inout_opt_ void* context_procedure) :guid(guid), context_procedure(context_procedure), mtx(), hook_table() {}
		_impl_RawProcedure(const _impl_RawProcedure&) = delete;
		_impl_RawProcedure(_impl_RawProcedure&& t) = delete;
		~_impl_RawProcedure() {}
		const GUID* GetGuid() const { return &this->guid; }
		void* GetProcedureContext() const { return this->context_procedure; }
		void CreateHook(_In_ const RawHookEntry* hook_entry, _In_ const _impl_HookPosition* hook_position) {
			std::unique_lock<std::recursive_mutex> unique_lock_mtx(this->mtx);
			this->ProcessHookableProcException([&]() {
				if (this->count_is_invoking) throw(InvokingNoModifyException(this->guid_hook_invoking));
				this->hook_table.CreateHook(hook_entry, hook_position);
				});
		}
		void DeleteHook(_In_ const GUID* guid_hook) {
			std::unique_lock<std::recursive_mutex> unique_lock_mtx(this->mtx);
			this->ProcessHookableProcException([&]() {
				if (this->count_is_invoking) throw(InvokingNoModifyException(this->guid_hook_invoking));
				this->hook_table.DeleteHook(guid_hook);
				});
		}
		void Invoke(_In_opt_ void* param) {
			std::unique_lock<std::recursive_mutex> unique_lock_mtx(this->mtx);
			this->ProcessHookableProcException([&]() {
				this->hook_table.Invoke(param, this->pdecl);
				});
		}
		void InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_not_including_t) {
			std::unique_lock<std::recursive_mutex> unique_lock_mtx(this->mtx);
			this->ProcessHookableProcException([&]() {
				this->hook_table.InvokePartial(param, guid_hook_ref, after_not_including, this->pdecl);
				});
		}
		void InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_including_t) {
			std::unique_lock<std::recursive_mutex> unique_lock_mtx(this->mtx);
			this->ProcessHookableProcException([&]() {
				this->hook_table.InvokePartial(param, guid_hook_ref, after_including, this->pdecl);
				});
		}
	protected:
		template <typename _Fn>
		auto ProcessHookableProcException(_Fn fn) {
			try {
				return fn();
			} catch (HookableProcException& err) {
				if (err.CountShouldPassThroughProcedure > 0) {
					--err.CountShouldPassThroughProcedure;
				} else {
					err.SetProcedureGuid(&this->guid);
				}
				throw;
			}
		}
	};

	class RawProcedureTable final {
	public:
		RawProcedure* CreateProcedure(_In_ const GUID* guid_procedure, _Inout_opt_ void* context_procedure) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			return this->ProcessHookableProcException(guid_procedure, [&]()->RawProcedure* {
				if (this->guid_map.count(*guid_procedure)) throw(ProcedureAlreadyExistException());
				std::shared_ptr<RawProcedure> proc = nullptr;
				try {
					proc.reset(new RawProcedure(guid_procedure, context_procedure));
				} catch (HookableProcException& err) {
					++err.HookableProcException::CountShouldPassThroughProcedureTable;
					throw;
				}
				this->guid_map[*guid_procedure] = proc;
				map_hook_awaiting_create_t::iterator it_hook_awaiting_create_begin = this->map_hook_awaiting_create.lower_bound(id_proc_hook_t(guid_procedure, id_proc_hook_t::guid_hook_ninf));
				map_hook_awaiting_create_t::iterator it_hook_awaiting_create_end = this->map_hook_awaiting_create.upper_bound(id_proc_hook_t(guid_procedure, id_proc_hook_t::guid_hook_pinf));
				std::for_each(it_hook_awaiting_create_begin, it_hook_awaiting_create_end,
					[&proc](map_hook_awaiting_create_t::value_type& val_hook_awaiting_create) {
						try {
							return proc->pimpl->CreateHook(val_hook_awaiting_create.second->hook_entry.get(), val_hook_awaiting_create.second->hook_position.get());
						} catch (HookableProcException& err) {
							++err.HookableProcException::CountShouldPassThroughProcedureTable;
							throw;
						}
					});
				this->map_hook_awaiting_create.erase(it_hook_awaiting_create_begin, it_hook_awaiting_create_end);
				return proc.get();
				});
		}
		RawProcedure* GetProcedure(_In_ const GUID* guid_procedure) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			return this->ProcessHookableProcException(guid_procedure, [&]()->RawProcedure* {
				std::shared_ptr<RawProcedure> proc(nullptr);
				try {
					proc = this->guid_map.at(*guid_procedure);
				} catch (std::out_of_range&) {
					throw (ProcedureNotExistException());
				}
				if (!proc) THROW_UNEXPECTED_ERROR_EXCEPTION();
				return proc.get();
				});
		}
		void CreateHook(_In_ const GUID* guid_procedure, _In_ const RawHookEntry* hook_entry, _In_ const HookPosition* hook_position) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!hook_entry) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			if (!hook_position) THROW_INVALID_PARAMETER_EXCEPTION_THIS(2);
			return this->ProcessHookableProcException(guid_procedure, [&]() {
				std::shared_ptr<RawProcedure> proc(nullptr);
				try {
					proc = this->guid_map.at(*guid_procedure);
				} catch (std::out_of_range&) {}
				if (proc) {
					try {
						return proc->CreateHook(hook_entry, hook_position);
					} catch (HookableProcException& err) {
						++err.HookableProcException::CountShouldPassThroughProcedureTable;
						throw;
					}
				} else {
					return this->ProcessHookableProcHookException(hook_entry->GetGuid(), [&]() {
						id_proc_hook_t id_proc_hook(guid_procedure, hook_entry->GetGuid());
						if (this->map_hook_awaiting_create.count(id_proc_hook)) throw(HookAlreadyExistException());
						this->map_hook_awaiting_create[id_proc_hook] = std::unique_ptr<hook_awaiting_create_t>(new hook_awaiting_create_t(hook_entry, hook_position->pimpl));
						});
				}
				});
		}
		void DeleteHook(_In_ const GUID* guid_procedure, _In_ const GUID* guid_hook) {
			if (!guid_procedure) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
			if (!guid_hook) THROW_INVALID_PARAMETER_EXCEPTION_THIS(1);
			return this->ProcessHookableProcException(guid_procedure, [&]() {
				std::shared_ptr<RawProcedure> proc(nullptr);
				try {
					proc = this->guid_map.at(*guid_procedure);
				} catch (std::out_of_range&) {}
				if (proc) {
					try {
						return proc->DeleteHook(guid_hook);
					} catch (HookableProcException& err) {
						++err.HookableProcException::CountShouldPassThroughProcedureTable;
						throw;
					}
				} else {
					return this->ProcessHookableProcHookException(guid_hook, [&]() {
						id_proc_hook_t id_proc_hook(guid_procedure, guid_hook);
						if (!this->map_hook_awaiting_create.count(id_proc_hook)) throw (HookNotExistException());
						this->map_hook_awaiting_create.erase(id_proc_hook);
						});
				}
				});
		}
	protected:
		struct hook_awaiting_create_t {
			std::unique_ptr<RawHookEntry> hook_entry;
			std::unique_ptr<_impl_HookPosition> hook_position;
			hook_awaiting_create_t(_In_ const RawHookEntry* hook_entry, _In_ const _impl_HookPosition* hook_position)
				: hook_entry(new RawHookEntry(*hook_entry)), hook_position(new _impl_HookPosition(*hook_position)) {}
			hook_awaiting_create_t(const hook_awaiting_create_t&) = delete;
			hook_awaiting_create_t(hook_awaiting_create_t&&) = delete;
		};
		struct id_proc_hook_t {
			struct guid_hook_ninf_t {};
			struct guid_hook_pinf_t {};
			static constexpr guid_hook_ninf_t guid_hook_ninf {};
			static constexpr guid_hook_pinf_t guid_hook_pinf {};
			GUID guid_procedure = GUID_NULL;
			bool is_guid_hook_ninf = false;
			bool is_guid_hook_pinf = false;
			GUID guid_hook = GUID_NULL;
			id_proc_hook_t(_In_ const GUID* guid_procedure, _In_ const GUID* guid_hook)
				:guid_procedure(*guid_procedure), guid_hook(*guid_hook) {}
			id_proc_hook_t(_In_ const GUID* guid_procedure, guid_hook_ninf_t)
				:guid_procedure(*guid_procedure), is_guid_hook_ninf(true) {}
			id_proc_hook_t(_In_ const GUID* guid_procedure, guid_hook_pinf_t)
				:guid_procedure(*guid_procedure), is_guid_hook_pinf(true) {}
			bool operator<(const id_proc_hook_t& t) const {
				if (obj_less_than_guid(this->guid_procedure, t.guid_procedure))
					return true;
				else if (!obj_equal_to_guid(t.guid_procedure, this->guid_procedure))
					return false;
				else if (this->is_guid_hook_ninf) {
					if (!t.is_guid_hook_ninf)
						return true;
					else
						return false;
				} else if (t.is_guid_hook_ninf || this->is_guid_hook_pinf)
					return false;
				else if (t.is_guid_hook_pinf)
					return true;
				else
					return obj_less_than_guid(this->guid_hook, t.guid_hook);
			}
		};
		typedef std::unordered_map<GUID, std::shared_ptr<RawProcedure>, hash_guid, equal_to_guid> guid_map_t;
		typedef std::map<id_proc_hook_t, std::unique_ptr<hook_awaiting_create_t> > map_hook_awaiting_create_t;
		guid_map_t guid_map;
		map_hook_awaiting_create_t map_hook_awaiting_create;
		template <typename _Fn>
		static auto ProcessHookableProcException(_In_ const GUID* guid_procedure, _Fn fn) {
			try {
				return fn();
			} catch (HookableProcException& err) {
				if (err.HookableProcException::CountShouldPassThroughProcedureTable > 0) {
					--err.HookableProcException::CountShouldPassThroughProcedureTable;
				} else {
					err.SetProcedureGuid(guid_procedure);
				}
				throw;
			}
		}
		template <typename _Fn>
		static auto ProcessHookableProcHookException(_In_ const GUID* guid_hook, _Fn fn) {
			try {
				return fn();
			} catch (HookableProcHookException& err) {
				if (err.HookableProcHookException::CountShouldPassThroughProcedureTable > 0) {
					--err.HookableProcHookException::CountShouldPassThroughProcedureTable;
				} else {
					err.SetHookGuid(guid_hook);
				}
				throw;
			}
		}
	};

	static INIT_ONCE initonce_HookableProcRaw = INIT_ONCE_STATIC_INIT;
	static std::recursive_mutex* mutex_procedure_table = nullptr;
	static RawProcedureTable* procedure_table = nullptr;

	static void init();

	RawHookEntry::RawHookEntry(_In_ const GUID* guid, _In_opt_ RawHookFnptrType hook_fnptr, _In_opt_ RawHookCleanupFnptrType cleanup_fnptr, _Inout_opt_ void* context) {
		this->pimpl = new _impl_RawHookEntry(*guid, hook_fnptr, cleanup_fnptr, context);
		this->pimpl->pdecl = this;
	}

	DEFINE_PIMPL_FUNCS(RawHookEntry);

	const GUID* RawHookEntry::GetGuid() const { return this->pimpl->GetGuid(); }

	HookRet __thiscall RawHookEntry::InvokeHook(_In_opt_ void* param, _Inout_ RawProcedure* procedure) const { return this->pimpl->InvokeHook(param, procedure); }

	RawProcedure* RawProcedure::CreateProcedure(_In_ const GUID* guid_procedure, _Inout_opt_ void* context_procedure) {
		init();
		{
			std::unique_lock<std::recursive_mutex> unique_lock_procedure_table(*mutex_procedure_table);
			return procedure_table->CreateProcedure(guid_procedure, context_procedure);
		}
	}

	RawProcedure* RawProcedure::GetProcedure(_In_ const GUID* guid_procedure) {
		init();
		{
			std::unique_lock<std::recursive_mutex> unique_lock_procedure_table(*mutex_procedure_table);
			return procedure_table->GetProcedure(guid_procedure);
		}
	}

	void RawProcedure::CreateHook(_In_ const GUID* guid_procedure, _In_ const RawHookEntry* hook_entry, _In_ const HookPosition* hook_position) {
		init();
		{
			std::unique_lock<std::recursive_mutex> unique_lock_procedure_table(*mutex_procedure_table);
			return procedure_table->CreateHook(guid_procedure, hook_entry, hook_position);
		}
	}

	void RawProcedure::DeleteHook(_In_ const GUID* guid_procedure, _In_ const GUID* guid_hook) {
		init();
		{
			std::unique_lock<std::recursive_mutex> unique_lock_procedure_table(*mutex_procedure_table);
			return procedure_table->DeleteHook(guid_procedure, guid_hook);
		}
	}

	RawProcedure::RawProcedure(_In_ const GUID* guid, _Inout_opt_ void* context_procedure) {
		if (!guid) THROW_INVALID_PARAMETER_EXCEPTION_THIS(0);
		this->pimpl = new _impl_RawProcedure(*guid, context_procedure);
		this->pimpl->pdecl = this;
	}

	DEFINE_PIMPL_DTOR(RawProcedure);

	const GUID* RawProcedure::GetGuid() const { return this->pimpl->GetGuid(); }

	void* RawProcedure::GetProcedureContext() const { return this->pimpl->GetProcedureContext(); }

	void RawProcedure::CreateHook(_In_ const RawHookEntry* hook_entry, _In_ const HookPosition* hook_position) { return this->pimpl->CreateHook(hook_entry, hook_position->pimpl); }

	void RawProcedure::DeleteHook(_In_ const GUID* guid_hook) { return this->pimpl->DeleteHook(guid_hook); }

	void __thiscall RawProcedure::Invoke(_In_opt_ void* param) { return this->pimpl->Invoke(param); }

	void __thiscall RawProcedure::InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_not_including_t) { return this->pimpl->InvokePartial(param, guid_hook_ref, after_not_including); }

	void __thiscall RawProcedure::InvokePartial(_In_opt_ void* param, _In_opt_ const GUID* guid_hook_ref, after_including_t) { return this->pimpl->InvokePartial(param, guid_hook_ref, after_including); }

	intptr_t* RawProcedure::GetPCountIsInvoking() { return &this->pimpl->count_is_invoking; }

	const GUID** RawProcedure::GetPGuidHookInvoking() { return &this->pimpl->guid_hook_invoking; }

	static void init() {
		if (!InitOnceExecuteOnce(&initonce_HookableProcRaw,
			[](_Inout_ PINIT_ONCE, _Inout_opt_ PVOID, _Out_opt_ PVOID*)->BOOL {
				if (!mutex_procedure_table) {
					mutex_procedure_table = new std::recursive_mutex();
				}
				if (!procedure_table) {
					procedure_table = new RawProcedureTable();
				}
				return TRUE;
			}, nullptr, nullptr)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "InitOnceExecuteOnce");
	}
}