// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

// Windows Header Files:
#include <windows.h>
#include <Psapi.h>
#include <DbgHelp.h>
#include <string>

// TODO: reference additional headers your program requires here
#ifndef _BOOST_
#define _BOOST_

#include <boost/format.hpp>
#include <boost/mem_fn.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace boost;

#endif // !_BOOST_

#include "SharedMemory.h"
#include "Structures.h"


extern HMODULE g_hModule;

#ifdef _DEBUG
#include <crtdbg.h>
void* __cdecl operator new(size_t nSize, LPCSTR lpszFileName, int nLine);
#define DEBUG_NEW new(__FILE__, __LINE__)

void __cdecl operator delete(void* p, LPCSTR lpszFileName, int nLine);
#endif

#ifdef _DEBUG

void Trace(const wchar_t* pszFormat, ...);

#define TRACE		::Trace

#else

#define TRACE		__noop

#endif // _DEBUG

std::wstring FormatError(DWORD errorId, wformat format, std::wstring name);