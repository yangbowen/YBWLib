#include "stdafx.h"
#include "exception\Exception.h"
#include "misc\ptr_helper.h"
#include "misc\window_helper.h"
#include "exception\exception_helper.h"
#include "resource\global_res.h"
#include "exception\Exception_res.h"
#include "HookableProc\HookableProc_res.h"
#include "main.h"

HINSTANCE hmod = nullptr;
HINSTANCE hinst = nullptr;

static INIT_ONCE initonce_YBWLib = INIT_ONCE_STATIC_INIT;

static void init();

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
) {
	UNREFERENCED_PARAMETER(lpReserved);
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		hmod = hModule;
		hinst = GetModuleHandle(nullptr);
		init();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

static void init() {
	if (!InitOnceExecuteOnce(&initonce_YBWLib,
		[](_Inout_ PINIT_ONCE, _Inout_opt_ PVOID, _Out_opt_ PVOID*)->BOOL {
			try {
				global_res_load();
				Exception::res_load();
				HookableProc::res_load();
			} catch (Exception::BaseException& err) {
				Exception::display_exception(&err);
				abort();
			} catch (std::exception& err) {
				Exception::display_exception(&err);
				abort();
			} catch (...) {
				Exception::display_exception();
				abort();
			}
			return TRUE;
		}
	, nullptr, nullptr)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "InitOnceExecuteOnce");
}
