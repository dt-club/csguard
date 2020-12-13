#include <Windows.h>
#include <WinNT.h>
#include <intrin.h>
#include "detours/detours.h"
#include <ImageHlp.h>
#include <math.h>

void MF_Initialize( void );
void MF_Shutdown( void );

void FP_Initialize( void );
void FP_Shutdown( void );

void UnloadOldMouseFix(VOID)
{
	HMODULE hModule;
	CHAR szCurrentDirectory[MAX_PATH];

	hModule = GetModuleHandleA("moufix.dll");

	if(hModule != NULL)
	{
		GetModuleFileName(hModule, szCurrentDirectory, MAX_PATH);
		FreeLibrary(hModule);
		DeleteFile(szCurrentDirectory);
	}
}
void RemoveAndFixIllegalResources();
HINSTANCE g_hInstance;
BOOL WINAPI DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	UNREFERENCED_PARAMETER( lpReserved );

	if( dwReason == DLL_PROCESS_ATTACH )
	{
		g_hInstance = (HINSTANCE)hModule;

		UnloadOldMouseFix();
		RemoveAndFixIllegalResources();

		MF_Initialize( );
		FP_Initialize();
	}

	if( dwReason == DLL_PROCESS_DETACH )
	{
		FP_Shutdown();
		MF_Shutdown( );
	}

	return TRUE;
}