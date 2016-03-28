// stdafx.cpp : source file that includes just the standard includes
// SOSHook.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

HMODULE g_hModule = NULL;

#ifdef _DEBUG

void* __cdecl operator new(size_t nSize, LPCSTR lpszFileName, int nLine)
{
	return ::_malloc_dbg(nSize, 1, lpszFileName, nLine);
}

void __cdecl operator delete(void* pData, LPCSTR /* lpszFileName */, int /* nLine */)
{
	::operator delete(pData);
}

#endif

#ifdef _DEBUG
#include <stdio.h>

void Trace(const wchar_t* pszFormat, ...)
{
	wchar_t szOutput[1024];
	va_list	vaList;

	va_start(vaList, pszFormat);
	vswprintf(szOutput, _countof(szOutput), pszFormat, vaList);
	::OutputDebugString(szOutput);
}

#endif // _DEBUG

