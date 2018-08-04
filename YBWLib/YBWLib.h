#pragma once

#ifdef YBWLIB_EXPORTS
#define YBWLIB_API __declspec(dllexport)
#else
#define YBWLIB_API __declspec(dllimport)
#endif
