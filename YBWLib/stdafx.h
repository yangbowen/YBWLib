#pragma once
#ifndef _INCLUDED_YBWLIB_STDAFX_H_
#define _INCLUDED_YBWLIB_STDAFX_H_

#include <cstdint>

#include <winsdkver.h>
#define _WIN32_WINNT 0x601
#include <sdkddkver.h>

#include <ntstatus.h>

#define WIN32_LEAN_AND_MEAN
#define WIN32_NO_STATUS
#include <Windows.h>

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
typedef NTSTATUS *PNTSTATUS;

#include <rpc.h>
#include <Ole2.h>
#endif
