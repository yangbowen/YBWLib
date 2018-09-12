#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <cstdio>
#include "Exception.h"

namespace Exception {
	const wchar_t* ExceptionWithSourceCodePosition::DescriptionPostfixFormat = L"\nat %2!s!.%1!s!";
	const wchar_t* UnexpectedErrorException::DescriptionFormat = L"Unexpected error.%1!s!";
	const wchar_t* ExternalAPIErrorException::DescriptionFormat_Lib = L"External API \"%3!S!\" in library \"%2!S!\" failure. GetLastError returns %4!u!.%1!s!";
	const wchar_t* ExternalAPIErrorException::DescriptionFormat_NoLib = L"External API \"%2!S!\" failure. GetLastError returns %3!u!.%1!s!";
	const wchar_t* InvalidParameterException::DescriptionFormat_Class_ThisPtr = L"Invalid parameter %5!d! when calling member function \"%4!S!\" of \"%2!S!\". this: %3!s!.%1!s!";
	const wchar_t* InvalidParameterException::DescriptionFormat_Class = L"Invalid parameter %4!d! when calling member function \"%3!S!\" of \"%2!S!\".%1!s!";
	const wchar_t* InvalidParameterException::DescriptionFormat = L"Invalid parameter %3!d! when calling function \"%2!S!\".%1!s!";
	const wchar_t* ThreadInitializationFailureException::DescriptionFormat = L"Thread initialization failure.%1!s!";
	const wchar_t* WindowInitializationFailureException::DescriptionFormat = L"Window initialization failure. Window class name: \"%2!s!\".%1!s!";
	const wchar_t* ControlErrorException::DescriptionFormat_CtrlId = L"Control error. Control window class name: \"%2!s!\". Control identifier: %3!d!.%1!s!";
	const wchar_t* ControlErrorException::DescriptionFormat_NoCtrlId = L"Control error. Control window class name: \"%2!s!\".%1!s!";
	const wchar_t* NotImplementedException::DescriptionFormat = L"This part of the program has not been implemented yet.%1!s!";

	SourceCodePosition_t::SourceCodePosition_t(const wchar_t* file, int line)
		:FileName(_wcsdup(file)), LineNumber(line) {}

	SourceCodePosition_t::SourceCodePosition_t(const SourceCodePosition_t& t)
		: FileName(_wcsdup(t.FileName)), LineNumber(t.LineNumber) {}

	SourceCodePosition_t::SourceCodePosition_t(SourceCodePosition_t&& t)
		: FileName(t.FileName), LineNumber(t.LineNumber) {}

	SourceCodePosition_t::~SourceCodePosition_t() {
		if (this->FileName) {
			free(this->FileName);
			this->FileName = nullptr;
		}
		this->LineNumber = -1;
	}

	wchar_t* SourceCodePosition_t::ToString() const {
		return format_string(L"%1!s!(%2!d!)", this->FileName, this->LineNumber);
	}

	BaseException::BaseException() {}

	BaseException::BaseException(const BaseException& t) { UNREFERENCED_PARAMETER(t); }

	BaseException::BaseException(BaseException&& t) { UNREFERENCED_PARAMETER(t); }

	BaseException::~BaseException() {}

	wchar_t* BaseException::GetDescriptionPostfix() const {
		return format_string(L"\n");
	}

	ExceptionWithSourceCodePosition::ExceptionWithSourceCodePosition(const SourceCodePosition_t& source_code_position)
		: SourceCodePosition(source_code_position) {}

	ExceptionWithSourceCodePosition::ExceptionWithSourceCodePosition(const ExceptionWithSourceCodePosition& t)
		: SourceCodePosition(t.SourceCodePosition) {}

	ExceptionWithSourceCodePosition::ExceptionWithSourceCodePosition(ExceptionWithSourceCodePosition&& t)
		: SourceCodePosition(t.SourceCodePosition) {}

	ExceptionWithSourceCodePosition::~ExceptionWithSourceCodePosition() {}

	wchar_t* ExceptionWithSourceCodePosition::GetDescriptionPostfix() const {
		wchar_t* ret = nullptr;
		wchar_t* str_postfix_parent = this->BaseException::GetDescriptionPostfix();
		wchar_t* str_source_code_position = this->SourceCodePosition.ToString();
		ret = format_string(this->DescriptionPostfixFormat, str_postfix_parent, str_source_code_position);
		if (str_source_code_position) {
			delete[] str_source_code_position;
			str_source_code_position = nullptr;
		}
		if (str_postfix_parent) {
			delete[] str_postfix_parent;
			str_postfix_parent = nullptr;
		}
		return ret;
	}

	UnexpectedErrorException::UnexpectedErrorException(const SourceCodePosition_t& source_code_position)
		: ExceptionWithSourceCodePosition(source_code_position) {}

	UnexpectedErrorException::UnexpectedErrorException(const UnexpectedErrorException& t)
		: ExceptionWithSourceCodePosition(t) {}

	UnexpectedErrorException::UnexpectedErrorException(UnexpectedErrorException&& t)
		: ExceptionWithSourceCodePosition(t) {}

	UnexpectedErrorException::~UnexpectedErrorException() {}

	wchar_t* UnexpectedErrorException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = format_string(this->DescriptionFormat, this->GetDescriptionPostfix());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	ExternalAPIErrorException::ExternalAPIErrorException(const SourceCodePosition_t& source_code_position, const char* library_name, const char* function_name)
		: ExceptionWithSourceCodePosition(source_code_position), LibraryName(_strdup(library_name)), FunctionName(_strdup(function_name)) {}

	ExternalAPIErrorException::ExternalAPIErrorException(const SourceCodePosition_t& source_code_position, const char* function_name)
		: ExceptionWithSourceCodePosition(source_code_position), FunctionName(_strdup(function_name)) {}

	ExternalAPIErrorException::ExternalAPIErrorException(const ExternalAPIErrorException& t)
		: ExceptionWithSourceCodePosition(t), LibraryName(_strdup(t.LibraryName)), FunctionName(_strdup(t.FunctionName)) {}

	ExternalAPIErrorException::ExternalAPIErrorException(ExternalAPIErrorException&& t)
		: ExceptionWithSourceCodePosition(t), LibraryName(t.LibraryName), FunctionName(t.FunctionName) {}

	ExternalAPIErrorException::~ExternalAPIErrorException() {
		if (this->LibraryName) {
			free(this->LibraryName);
			this->LibraryName = nullptr;
		}
		if (this->FunctionName) {
			free(this->FunctionName);
			this->FunctionName = nullptr;
		}
	}

	wchar_t* ExternalAPIErrorException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = nullptr;
		if (this->LibraryName) {
			ret = format_string(this->DescriptionFormat_Lib, this->GetDescriptionPostfix(), this->LibraryName, this->FunctionName, GetLastError());
		} else {
			ret = format_string(this->DescriptionFormat_NoLib, this->GetDescriptionPostfix(), this->FunctionName, GetLastError());
		}
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	InvalidParameterException::InvalidParameterException(const SourceCodePosition_t& source_code_position, const char* func, int parameter_index)
		: ExceptionWithSourceCodePosition(source_code_position), FunctionName(_strdup(func)), ParameterIndex(parameter_index) {}

	InvalidParameterException::InvalidParameterException(const SourceCodePosition_t& source_code_position, const char* classname, const void* thisptr, const char* func, int parameter_index)
		: ExceptionWithSourceCodePosition(source_code_position), HasClass(true), ClassName(_strdup(classname)), HasThisPtr(true), ThisPtr(thisptr), FunctionName(_strdup(func)), ParameterIndex(parameter_index) {}

	InvalidParameterException::InvalidParameterException(const SourceCodePosition_t& source_code_position, const char* classname, const char* func, int parameter_index)
		: ExceptionWithSourceCodePosition(source_code_position), HasClass(true), ClassName(_strdup(classname)), FunctionName(_strdup(func)), ParameterIndex(parameter_index) {}

	InvalidParameterException::InvalidParameterException(const InvalidParameterException& t)
		: ExceptionWithSourceCodePosition(t), HasClass(true), ClassName(_strdup(t.ClassName)), HasThisPtr(true), ThisPtr(t.ThisPtr), FunctionName(_strdup(t.FunctionName)), ParameterIndex(t.ParameterIndex) {}

	InvalidParameterException::InvalidParameterException(InvalidParameterException&& t)
		: ExceptionWithSourceCodePosition(t), HasClass(true), ClassName(t.ClassName), HasThisPtr(true), ThisPtr(t.ThisPtr), FunctionName(t.FunctionName), ParameterIndex(t.ParameterIndex) {}

	InvalidParameterException::~InvalidParameterException() {
		this->HasClass = false;
		if (this->ClassName) {
			free(this->ClassName);
			this->ClassName = nullptr;
		}
		this->HasThisPtr = false;
		this->ThisPtr = nullptr;
		if (this->FunctionName) {
			free(this->FunctionName);
			this->FunctionName = nullptr;
		}
		this->ParameterIndex = -1;
	}

	wchar_t* InvalidParameterException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = nullptr;
		if (this->HasClass) {
			if (this->HasThisPtr) {
				wchar_t* str_ptr = pointer_to_string(this->ThisPtr);
				ret = format_string(this->DescriptionFormat_Class_ThisPtr, this->GetDescriptionPostfix(), this->ClassName, str_ptr, this->FunctionName, (int)this->ParameterIndex);
				if (str_ptr) {
					delete[] str_ptr;
					str_ptr = nullptr;
				}
			} else {
				ret = format_string(this->DescriptionFormat_Class, this->GetDescriptionPostfix(), this->ClassName, this->FunctionName, (int)this->ParameterIndex);
			}
		} else {
			ret = format_string(this->DescriptionFormat, this->GetDescriptionPostfix(), this->FunctionName, (int)this->ParameterIndex);
		}
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	ThreadInitializationFailureException::ThreadInitializationFailureException(const SourceCodePosition_t& source_code_position, HANDLE hthread)
		: ExceptionWithSourceCodePosition(source_code_position) {
		if (!hthread) {
			this->ThreadHandle = nullptr;
		} else {
			if (!DuplicateHandle(GetCurrentProcess(), hthread, GetCurrentProcess(), &this->ThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "DuplicateHandle");
		}
	}

	ThreadInitializationFailureException::ThreadInitializationFailureException(const ThreadInitializationFailureException& t)
		: ExceptionWithSourceCodePosition(t) {
		if (!t.ThreadHandle) {
			this->ThreadHandle = nullptr;
		} else {
			if (!DuplicateHandle(GetCurrentProcess(), t.ThreadHandle, GetCurrentProcess(), &this->ThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "DuplicateHandle");
		}
	}


	ThreadInitializationFailureException::ThreadInitializationFailureException(ThreadInitializationFailureException&& t)
		: ExceptionWithSourceCodePosition(t), ThreadHandle(t.ThreadHandle) {}

	ThreadInitializationFailureException::~ThreadInitializationFailureException() {
		if (this->ThreadHandle) {
			CloseHandle(this->ThreadHandle);
			this->ThreadHandle = nullptr;
		}
	}

	wchar_t* ThreadInitializationFailureException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = nullptr;
		ret = format_string(this->DescriptionFormat, this->GetDescriptionPostfix());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	WindowInitializationFailureException::WindowInitializationFailureException(const SourceCodePosition_t& source_code_position, const wchar_t* window_class_name)
		: ExceptionWithSourceCodePosition(source_code_position), WindowClassName(_wcsdup(window_class_name)) {}

	WindowInitializationFailureException::WindowInitializationFailureException(const WindowInitializationFailureException& t)
		: ExceptionWithSourceCodePosition(t), WindowClassName(_wcsdup(t.WindowClassName)) {}

	WindowInitializationFailureException::WindowInitializationFailureException(WindowInitializationFailureException&& t)
		: ExceptionWithSourceCodePosition(t), WindowClassName(t.WindowClassName) {}

	WindowInitializationFailureException::~WindowInitializationFailureException() {
		if (this->WindowClassName) {
			free(this->WindowClassName);
			this->WindowClassName = nullptr;
		}
	}

	wchar_t* WindowInitializationFailureException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = nullptr;
		ret = format_string(this->DescriptionFormat, this->GetDescriptionPostfix(), this->WindowClassName);
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	ControlErrorException::ControlErrorException(const SourceCodePosition_t& source_code_position, HWND hwnd)
		: ExceptionWithSourceCodePosition(source_code_position), Hwnd(hwnd) {}

	ControlErrorException::ControlErrorException(const ControlErrorException& t)
		: ExceptionWithSourceCodePosition(t), Hwnd(t.Hwnd) {}

	ControlErrorException::ControlErrorException(ControlErrorException&& t)
		: ExceptionWithSourceCodePosition(t), Hwnd(t.Hwnd) {}

	ControlErrorException::~ControlErrorException() {
		this->Hwnd = nullptr;
	}

	wchar_t* ControlErrorException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = nullptr;
		wchar_t window_class_name[1024];
		int ctrl_id = GetDlgCtrlID(this->Hwnd);
		if (!GetClassName(this->Hwnd, window_class_name, 1024)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "GetClassNameW");
		if (ctrl_id) {
			ret = format_string(this->DescriptionFormat_CtrlId, this->GetDescriptionPostfix(), window_class_name, ctrl_id);
		} else {
			ret = format_string(this->DescriptionFormat_NoCtrlId, this->GetDescriptionPostfix(), window_class_name);
		}
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	NotImplementedException::NotImplementedException(const SourceCodePosition_t& source_code_position)
		: ExceptionWithSourceCodePosition(source_code_position) {}

	NotImplementedException::NotImplementedException(const NotImplementedException& t)
		: ExceptionWithSourceCodePosition(t) {}

	NotImplementedException::NotImplementedException(NotImplementedException&& t)
		: ExceptionWithSourceCodePosition(t) {}

	NotImplementedException::~NotImplementedException() {}

	wchar_t* NotImplementedException::GetDescription() const {
		wchar_t* str_postfix = this->GetDescriptionPostfix();
		wchar_t* ret = format_string(this->DescriptionFormat, this->GetDescriptionPostfix());
		if (str_postfix) {
			delete[] str_postfix;
			str_postfix = nullptr;
		}
		return ret;
	}

	wchar_t* format_string(const wchar_t* format, ...) {
		if (!format) THROW_INVALID_PARAMETER_EXCEPTION(0);
		// Note: the size is 65536 instead of 65536 * sizeof(wchar_t), because FormatMessage limits the buffer size to 64K bytes.
		wchar_t* str = new wchar_t[65536 / sizeof(wchar_t)];
		va_list vl;
		va_start(vl, format);
		try {
			if (!FormatMessage(FORMAT_MESSAGE_FROM_STRING, format, 0, 0, str, 65536 / sizeof(wchar_t), &vl)) {
				THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "FormatMessageW");
			}
		} catch (...) {
			va_end(vl);
			throw;
		}
		va_end(vl);
		return str;
	}

	wchar_t* u8string_to_u16string(const char* u8str) {
		size_t cch_u16str = MultiByteToWideChar(CP_UTF8, 0, u8str, -1, nullptr, 0);
		wchar_t* u16str = new wchar_t[cch_u16str + 1];
		if (!MultiByteToWideChar(CP_UTF8, 0, u8str, -1, u16str, cch_u16str + 1)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "MultiByteToWideChar");
		return u16str;
	}

	wchar_t* pointer_to_string(const void* ptr) {
		wchar_t* str = new wchar_t[1024];
		swprintf_s(str, 1024, L"%p", ptr);
		return str;
	}

	wchar_t* size_to_string(size_t size) {
		wchar_t* str = new wchar_t[1024];
		swprintf_s(str, 1024, L"%zu", size);
		return str;
	}
}
