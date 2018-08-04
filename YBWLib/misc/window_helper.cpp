#pragma include_alias("stdafx.h","..\stdafx.h")
#include "stdafx.h"
#include <mutex>
#include <unordered_set>
#include "..\exception\Exception.h"
#include "ptr_helper.h"
#include "window_helper.h"

double dpiscale_x = 1.0;
double dpiscale_y = 1.0;

static INIT_ONCE initonce = INIT_ONCE_STATIC_INIT;
static std::mutex mutex_window_set;
static std::unordered_set<HWND> window_set;

static void init();

void init() {
	if (!InitOnceExecuteOnce(&initonce,
		[](_Inout_ PINIT_ONCE, _Inout_opt_ PVOID, _Out_opt_ PVOID*)->BOOL {
			unique_hdc hdc(CreateDC(L"DISPLAY", nullptr, nullptr, nullptr));
			if (!hdc) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "GetDC");
			dpiscale_x = GetDeviceCaps(hdc, LOGPIXELSX) / 72.0;
			dpiscale_y = GetDeviceCaps(hdc, LOGPIXELSY) / 72.0;
			return TRUE;
		}, nullptr, nullptr)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("kernel32", "InitOnceExecuteOnce");
}

double get_dpiscale_x() {
	init();
	return dpiscale_x;
}

double get_dpiscale_y() {
	init();
	return dpiscale_y;
}

void adjust_rect_topx(_Inout_ RECT* rect) {
	if (!rect) THROW_INVALID_PARAMETER_EXCEPTION(0);
#pragma warning(push)
#pragma warning(disable:4244)
	rect->left = ((double)rect->left) * get_dpiscale_x();
	rect->top = ((double)rect->top) * get_dpiscale_y();
	rect->right = ((double)rect->right) * get_dpiscale_x();
	rect->bottom = ((double)rect->bottom) * get_dpiscale_y();
#pragma warning(pop)
}

void adjust_rect_topx_dlg(_Inout_ RECT* rect, _In_ HWND hdlg) {
	RECT rect_ref;
	if (!rect) THROW_INVALID_PARAMETER_EXCEPTION(0);
	if (!hdlg) THROW_INVALID_PARAMETER_EXCEPTION(1);
	rect_ref.left = 0;
	rect_ref.top = 0;
	rect_ref.right = 1024;
	rect_ref.bottom = 1024;
	if (!MapDialogRect(hdlg, &rect_ref)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "MapDialogRect");
#pragma warning(push)
#pragma warning(disable:4244)
	rect->left = ((double)rect->left) * ((double)rect_ref.right) / 1024;
	rect->top = ((double)rect->top) * ((double)rect_ref.bottom) / 1024;
	rect->right = ((double)rect->right) * ((double)rect_ref.right) / 1024;
	rect->bottom = ((double)rect->bottom) * ((double)rect_ref.bottom) / 1024;
#pragma warning(pop)
}

void adjust_rect_frompx(_Inout_ RECT* rect) {
	if (!rect) THROW_INVALID_PARAMETER_EXCEPTION(0);
#pragma warning(push)
#pragma warning(disable:4244)
	rect->left = ((double)rect->left) / get_dpiscale_x();
	rect->top = ((double)rect->top) / get_dpiscale_y();
	rect->right = ((double)rect->right) / get_dpiscale_x();
	rect->bottom = ((double)rect->bottom) / get_dpiscale_y();
#pragma warning(pop)
}

void adjust_rect_frompx_dlg(_Inout_ RECT* rect, _In_ HWND hdlg) {
	RECT rect_ref;
	if (!rect) THROW_INVALID_PARAMETER_EXCEPTION(0);
	if (!hdlg) THROW_INVALID_PARAMETER_EXCEPTION(1);
	rect_ref.left = 0;
	rect_ref.top = 0;
	rect_ref.right = 1024;
	rect_ref.bottom = 1024;
	if (!MapDialogRect(hdlg, &rect_ref)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "MapDialogRect");
#pragma warning(push)
#pragma warning(disable:4244)
	rect->left = ((double)rect->left) / ((double)rect_ref.right) * 1024;
	rect->top = ((double)rect->top) / ((double)rect_ref.bottom) * 1024;
	rect->right = ((double)rect->right) / ((double)rect_ref.right) * 1024;
	rect->bottom = ((double)rect->bottom) / ((double)rect_ref.bottom) * 1024;
#pragma warning(pop)
}

void get_window_client_size(_In_ HWND hwnd, _Out_ double* width, _Out_ double* height) {
	RECT rect_client;
	if (!hwnd) THROW_INVALID_PARAMETER_EXCEPTION(0);
	if (!width) THROW_INVALID_PARAMETER_EXCEPTION(1);
	if (!height) THROW_INVALID_PARAMETER_EXCEPTION(2);
	if (!GetClientRect(hwnd, &rect_client)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "GetClientRect");
	adjust_rect_frompx(&rect_client);
	*width = rect_client.right - rect_client.left;
	*height = rect_client.bottom - rect_client.top;
}

void get_window_client_size_dlg(_In_ HWND hwnd, _Out_ double* width, _Out_ double* height, _In_ HWND hdlg) {
	RECT rect_client;
	if (!hwnd) THROW_INVALID_PARAMETER_EXCEPTION(0);
	if (!width) THROW_INVALID_PARAMETER_EXCEPTION(1);
	if (!height) THROW_INVALID_PARAMETER_EXCEPTION(2);
	if (!hdlg) THROW_INVALID_PARAMETER_EXCEPTION(3);
	if (!GetClientRect(hwnd, &rect_client)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "GetClientRect");
	adjust_rect_frompx_dlg(&rect_client, hdlg);
	*width = rect_client.right - rect_client.left;
	*height = rect_client.bottom - rect_client.top;
}

void move_window(_In_ HWND hwnd, _In_ double x, _In_ double y, _In_ double width, _In_ double height, _In_opt_ BOOL repaint) {
	RECT rect;
	if (!hwnd) THROW_INVALID_PARAMETER_EXCEPTION(0);
#pragma warning(push)
#pragma warning(disable:4244)
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
#pragma warning(pop)
	adjust_rect_topx(&rect);
	if (!MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, repaint)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "MoveWindow");
}

void move_window_dlg(_In_ HWND hwnd, _In_ double x, _In_ double y, _In_ double width, _In_ double height, _In_opt_ BOOL repaint, _In_ HWND hdlg) {
	RECT rect;
	if (!hwnd) THROW_INVALID_PARAMETER_EXCEPTION(0);
	if (!hdlg) THROW_INVALID_PARAMETER_EXCEPTION(6);
#pragma warning(push)
#pragma warning(disable:4244)
	rect.left = x;
	rect.top = y;
	rect.right = x + width;
	rect.bottom = y + height;
#pragma warning(pop)
	adjust_rect_topx_dlg(&rect, hdlg);
	if (!MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, repaint)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "MoveWindow");
}

void window_register(_In_ HWND hwnd) {
	std::lock_guard<std::mutex> unique_lock_com(mutex_window_set);
	window_set.insert(hwnd);
}

void window_unregister(_In_ HWND hwnd) {
	std::lock_guard<std::mutex> unique_lock_com(mutex_window_set);
	window_set.erase(hwnd);
}

void window_broadcast(_In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam) {
	std::lock_guard<std::mutex> unique_lock_com(mutex_window_set);
	std::unordered_set<HWND>::const_iterator it;
	for (it = window_set.cbegin(); it != window_set.cend(); ++it)
		if (!PostMessage(*it, msg, wparam, lparam)) THROW_EXTERNAL_API_ERROR_EXCEPTION_LIB("user32", "PostMessageW");
}