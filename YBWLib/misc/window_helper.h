#pragma once
#ifndef _INCLUDED_YBWLIB_WINDOW_HELPER_H_
#define _INCLUDED_YBWLIB_WINDOW_HELPER_H_
#include <windef.h>
#include "..\YBWLib.h"

YBWLIB_API double get_dpiscale_x();
YBWLIB_API double get_dpiscale_y();
// Calculates a rectangle in pixels from a rectangle in points.
YBWLIB_API void adjust_rect_topx(_Inout_ RECT* rect);
// Calculates a rectangle in pixels from a rectangle in dialog template units of the specified dialog.
YBWLIB_API void adjust_rect_topx_dlg(_Inout_ RECT* rect, _In_ HWND hdlg);
// Calculates a rectangle in points from a rectangle in pixels.
YBWLIB_API void adjust_rect_frompx(_Inout_ RECT* rect);
// Calculates a rectangle in dialog template units of the specified dialog from a rectangle in pixels.
YBWLIB_API void adjust_rect_frompx_dlg(_Inout_ RECT* rect, _In_ HWND hdlg);
// Get the size of the client area of a window.
// The coordinates are given in points.
YBWLIB_API void get_window_client_size(_In_ HWND hwnd, _Out_ double* width, _Out_ double* height);
// Get the size of the client area of a window.
// The coordinates are given in dialog template units of the specified dialog.
YBWLIB_API void get_window_client_size_dlg(_In_ HWND hwnd, _Out_ double* width, _Out_ double* height, _In_ HWND hdlg);
// Move and/or resize a window.
// The coordinates are given in points.
YBWLIB_API void move_window(_In_ HWND hwnd, _In_ double x, _In_ double y, _In_ double width, _In_ double height, _In_opt_ BOOL repaint);
// Move and/or resize a window.
// The coordinates are given in dialog template units of the specified dialog.
YBWLIB_API void move_window_dlg(_In_ HWND hwnd, _In_ double x, _In_ double y, _In_ double width, _In_ double height, _In_opt_ BOOL repaint, _In_ HWND hdlg);
// Register a window in the window list.
YBWLIB_API void window_register(_In_ HWND hwnd);
// Unregister a window in the window list.
YBWLIB_API void window_unregister(_In_ HWND hwnd);
// Broadcast a window message to all windows in the window list.
YBWLIB_API void window_broadcast(_In_ UINT msg, _In_ WPARAM wparam, _In_ LPARAM lparam);
#endif
