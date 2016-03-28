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

std::wstring FormatError(DWORD errorId, wformat format, std::wstring name)
{
	if (errorId == 0)
		return std::wstring();

	LPTSTR msgBuffer = nullptr;
	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorId, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuffer, 0, NULL);

	std::wstring message(msgBuffer, size);

	LocalFree(msgBuffer);

	return (format % name % (wformat(L"ID: %1%, Message: %2%") % errorId % message).str()).str();
}