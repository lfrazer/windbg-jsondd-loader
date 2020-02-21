// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

// ----------------------------------------------------------------------------

extern "C"
{

// ----------------------------------------------------------------------------

HRESULT
CALLBACK
DebugExtensionInitialize(
    __out PULONG Version,
    __out PULONG Flags
)
{
    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;

    return S_OK;
}

// ----------------------------------------------------------------------------

VOID
CALLBACK
DebugExtensionUninitialize()
{
}

// ----------------------------------------------------------------------------

BOOL
APIENTRY
DllMain(
    __in HMODULE hModule,
    __in DWORD  nReason,
    __in LPVOID lpReserved
)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);

    switch (nReason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// ----------------------------------------------------------------------------

}

// ----------------------------------------------------------------------------