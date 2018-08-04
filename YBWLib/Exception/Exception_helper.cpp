#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "exception_helper.h"
#include "..\resource\global_res.h"

namespace Exception {

	void display_error_string(_In_ const wchar_t* error_string) {
		MessageBox(nullptr, error_string, str_YBWLib_TITLE, 0);
	}

	void display_exception(_In_ const Exception::BaseException* err) {
		display_error_string(err->GetDescription());
	}

	void display_exception(_In_ const std::exception* err) {
		display_error_string(format_string(L"Unhandled STL exception: %1!S!", err->what()));
	}

	void display_exception() {
		display_error_string(L"Unhandled exception.");
	}
}
