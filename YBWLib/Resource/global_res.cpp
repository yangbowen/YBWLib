#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "resource_helper.h"
#include "resource.h"
#include "global_res.h"

const wchar_t* str_YBWLib_TITLE = nullptr;

static bool global_res_loaded = false;

void global_res_load() {
	if (!global_res_loaded) {
		str_YBWLib_TITLE = load_res_string(IDS_YBWLib_TITLE);
		global_res_loaded = true;
	}
}