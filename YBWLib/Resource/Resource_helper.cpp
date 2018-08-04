#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "..\exception\Exception.h"
#include "resource_helper.h"
#include "..\main.h"

wchar_t* load_res_string(_In_ UINT id) {
	wchar_t* tempstr = new wchar_t[65536];
	if (!LoadString(hmod, id, tempstr, 65536)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "LoadStringW");
	return tempstr;
}