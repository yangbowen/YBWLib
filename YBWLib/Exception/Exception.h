#pragma once

#include <minwindef.h>
#include <typeinfo>
#include <type_traits>
#include "..\YBWLib.h"

#ifndef EXCEPTION_NO_MACROS
#define MAKE_SOURCE_CODE_POSITION() Exception::SourceCodePosition_t(__FILEW__, __LINE__)
#define THROW_UNEXPECTED_ERROR_EXCEPTION() throw(Exception::UnexpectedErrorException(MAKE_SOURCE_CODE_POSITION()))
#define THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB(library_name, function_name) throw(Exception::ExternalAPIErrorException(MAKE_SOURCE_CODE_POSITION(), (library_name), (function_name)))
#define THROW_EXTERNAL_API_ERROR_EXCEPTION_NOLIB(function_name) throw(Exception::ExternalAPIErrorException(MAKE_SOURCE_CODE_POSITION(), (function_name)))
#define THROW_INVALID_PARAMETER_EXCEPTION_THIS(parameter_index) throw(Exception::InvalidParameterException(MAKE_SOURCE_CODE_POSITION(), Exception::get_type_name<std::remove_pointer<decltype(this)>::type>(), this, __func__, (parameter_index)))
#define THROW_INVALID_PARAMETER_EXCEPTION_CLASS(class_type, parameter_index) throw(Exception::InvalidParameterException(MAKE_SOURCE_CODE_POSITION(), Exception::get_type_name<class_type>(), __func__, (parameter_index)))
#define THROW_INVALID_PARAMETER_EXCEPTION(parameter_index) throw(Exception::InvalidParameterException(MAKE_SOURCE_CODE_POSITION(), __func__, (parameter_index)))
#define THROW_THREAD_INITIALIZATION_FAILURE_EXCEPTION(hthread) throw(Exception::ThreadInitializationFailureException(MAKE_SOURCE_CODE_POSITION(), (hthread)))
#define THROW_WINDOW_INITIALIZATION_FAILURE_EXCEPTION(window_class_name) throw(Exception::WindowInitializationFailureException(MAKE_SOURCE_CODE_POSITION(), (window_class_name)))
#define THROW_CONTROL_ERROR_EXCEPTION(hwnd) throw(Exception::ControlErrorException(MAKE_SOURCE_CODE_POSITION(), (hwnd)))
#define THROW_NOT_IMPLEMENTED_EXCEPTION() throw(Exception::NotImplementedException(MAKE_SOURCE_CODE_POSITION()))
#endif

namespace Exception {
	class YBWLIB_API SourceCodePosition_t {
	public:
		wchar_t* FileName = nullptr;
		int LineNumber = -1;
		SourceCodePosition_t(const wchar_t* file, int line);
		SourceCodePosition_t(const SourceCodePosition_t& t);
		SourceCodePosition_t(SourceCodePosition_t&& t);
		virtual ~SourceCodePosition_t();
		virtual wchar_t* ToString() const;
	};

	class YBWLIB_API BaseException abstract {
	public:
		BaseException();
		BaseException(const BaseException& t);
		BaseException(BaseException&& t);
		virtual ~BaseException();
		virtual wchar_t* GetDescription() const = 0;
	protected:
		virtual wchar_t* GetDescriptionPostfix() const;
	};

	class YBWLIB_API ExceptionWithSourceCodePosition abstract :public BaseException {
	public:
		static const wchar_t* DescriptionPostfixFormat;
		SourceCodePosition_t SourceCodePosition;
		ExceptionWithSourceCodePosition(const SourceCodePosition_t& source_code_position);
		ExceptionWithSourceCodePosition(const ExceptionWithSourceCodePosition& t);
		ExceptionWithSourceCodePosition(ExceptionWithSourceCodePosition&& t);
		virtual ~ExceptionWithSourceCodePosition();
	protected:
		virtual wchar_t* GetDescriptionPostfix() const;
	};

	// Some unexpected error occured in the program.
	// This means that there's some bugs in the program.
	class YBWLIB_API UnexpectedErrorException :public ExceptionWithSourceCodePosition {
	public:
		static const wchar_t* DescriptionFormat;
		explicit UnexpectedErrorException(const SourceCodePosition_t& source_code_position);
		UnexpectedErrorException(const UnexpectedErrorException& t);
		UnexpectedErrorException(UnexpectedErrorException&& t);
		virtual ~UnexpectedErrorException();
		virtual wchar_t* GetDescription() const;
	};

	// An external API call failed.
	class YBWLIB_API ExternalAPIErrorException :public ExceptionWithSourceCodePosition {
	public:
		static const wchar_t* DescriptionFormat_Lib;
		static const wchar_t* DescriptionFormat_NoLib;
		char* LibraryName = nullptr;
		char* FunctionName = nullptr;
		explicit ExternalAPIErrorException(const SourceCodePosition_t& source_code_position, const char* library_name, const char* function_name);
		explicit ExternalAPIErrorException(const SourceCodePosition_t& source_code_position, const char* function_name);
		ExternalAPIErrorException(const ExternalAPIErrorException& t);
		ExternalAPIErrorException(ExternalAPIErrorException&& t);
		virtual ~ExternalAPIErrorException();
		virtual wchar_t* GetDescription() const;
	};

	// A parameter is invalid.
	class YBWLIB_API InvalidParameterException :public ExceptionWithSourceCodePosition {
	public:
		static const wchar_t* DescriptionFormat_Class_ThisPtr;
		static const wchar_t* DescriptionFormat_Class;
		static const wchar_t* DescriptionFormat;
		bool HasClass = false;
		char* ClassName = nullptr;
		bool HasThisPtr = false;
		const void* ThisPtr = nullptr;
		char* FunctionName = nullptr;
		int ParameterIndex = -1;
		explicit InvalidParameterException(const SourceCodePosition_t& source_code_position, const char* classname, const void* thisptr, const char* func, int parameter_index);
		explicit InvalidParameterException(const SourceCodePosition_t& source_code_position, const char* classname, const char* func, int parameter_index);
		explicit InvalidParameterException(const SourceCodePosition_t& source_code_position, const char* func, int parameter_index);
		InvalidParameterException(const InvalidParameterException& t);
		InvalidParameterException(InvalidParameterException&& t);
		virtual ~InvalidParameterException();
		virtual wchar_t* GetDescription() const;
	};

	// Thread initialization failure.
	class YBWLIB_API ThreadInitializationFailureException :public ExceptionWithSourceCodePosition {
	public:
		static const wchar_t* DescriptionFormat;
		HANDLE ThreadHandle = nullptr;
		explicit ThreadInitializationFailureException(const SourceCodePosition_t& source_code_position, HANDLE hthread);
		ThreadInitializationFailureException(const ThreadInitializationFailureException& t);
		ThreadInitializationFailureException(ThreadInitializationFailureException&& t);
		virtual ~ThreadInitializationFailureException();
		virtual wchar_t* GetDescription() const;
	};

	// Window initialization failure.
	class YBWLIB_API WindowInitializationFailureException :public ExceptionWithSourceCodePosition {
	public:
		static const wchar_t* DescriptionFormat;
		wchar_t* WindowClassName = nullptr;
		explicit WindowInitializationFailureException(const SourceCodePosition_t& source_code_position, const wchar_t* window_class_name);
		WindowInitializationFailureException(const WindowInitializationFailureException& t);
		WindowInitializationFailureException(WindowInitializationFailureException&& t);
		virtual ~WindowInitializationFailureException();
		virtual wchar_t* GetDescription() const;
	};

	// Control error.
	class YBWLIB_API ControlErrorException :public ExceptionWithSourceCodePosition {
	public:
		static const wchar_t* DescriptionFormat_CtrlId;
		static const wchar_t* DescriptionFormat_NoCtrlId;
		HWND Hwnd = nullptr;
		explicit ControlErrorException(const SourceCodePosition_t& source_code_position, HWND hwnd);
		ControlErrorException(const ControlErrorException& t);
		ControlErrorException(ControlErrorException&& t);
		virtual ~ControlErrorException();
		virtual wchar_t* GetDescription() const;
	};

	// This part of the program has not been implemented yet.
	class YBWLIB_API NotImplementedException :public ExceptionWithSourceCodePosition {
	public:
		static const wchar_t* DescriptionFormat;
		explicit NotImplementedException(const SourceCodePosition_t& source_code_position);
		NotImplementedException(const NotImplementedException& t);
		NotImplementedException(NotImplementedException&& t);
		virtual ~NotImplementedException();
		virtual wchar_t* GetDescription() const;
	};

	template <typename _Ty>
	FORCEINLINE const char* get_type_name() { return typeid(_Ty).name(); }
	YBWLIB_API wchar_t* format_string(const wchar_t* format, ...);
	YBWLIB_API wchar_t* pointer_to_string(const void* ptr);
	YBWLIB_API wchar_t* size_to_string(size_t size);
}
