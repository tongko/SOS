// SOSHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"



BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
	TRACE(L"::DLL Main\n");

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		g_hModule = (HMODULE)hModule;
		break;
	}
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		TRACE(L"::Hook exiting\n");
		break;
	}

	return TRUE;
}