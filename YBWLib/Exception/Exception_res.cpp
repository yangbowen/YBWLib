#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include "Exception.h"
#include "..\resource\resource_helper.h"
#include "..\resource\resource.h"
#include "Exception_res.h"

namespace Exception {
	static bool res_loaded = false;

	void res_load() {
		if (!res_loaded) {
			ExceptionWithSourceCodePosition::DescriptionPostfixFormat = load_res_string(IDS_EXCEPTDESCPFIX_EXCEPTION_WITH_SOURCE_CODE_POSITION);
			UnexpectedErrorException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_UNEXPECTED);
			ExternalAPIErrorException::DescriptionFormat_Lib = load_res_string(IDS_EXCEPTDESC_EXTERNALAPI_LIB);
			ExternalAPIErrorException::DescriptionFormat_NoLib = load_res_string(IDS_EXCEPTDESC_EXTERNALAPI_NOLIB);
			InvalidParameterException::DescriptionFormat_Class_ThisPtr = load_res_string(IDS_EXCEPTDESC_INVALIDPARAM_CLASS_THISPTR);
			InvalidParameterException::DescriptionFormat_Class = load_res_string(IDS_EXCEPTDESC_INVALIDPARAM_CLASS);
			InvalidParameterException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_INVALIDPARAM);
			ThreadInitializationFailureException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_THREADINIT);
			WindowInitializationFailureException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_WINDOWINIT);
			ControlErrorException::DescriptionFormat_CtrlId = load_res_string(IDS_EXCEPTDESC_CONTROL_CTRLID);
			ControlErrorException::DescriptionFormat_NoCtrlId = load_res_string(IDS_EXCEPTDESC_CONTROL_NOCTRLID);
			NotImplementedException::DescriptionFormat = load_res_string(IDS_EXCEPTDESC_NOT_IMPLEMENTED);
			res_loaded = true;
		}
	}
}