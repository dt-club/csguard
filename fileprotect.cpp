#include <Windows.h>
#include <WinNT.h>
#include <intrin.h>
#include "detours/detours.h"
#include <ImageHlp.h>
#include <math.h>
#include <atlstr.h>

typedef 
HANDLE (WINAPI *fnCreateFileA)(
    __in     LPCSTR lpFileName,
    __in     DWORD dwDesiredAccess,
    __in     DWORD dwShareMode,
    __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __in     DWORD dwCreationDisposition,
    __in     DWORD dwFlagsAndAttributes,
    __in_opt HANDLE hTemplateFile
);

typedef
HANDLE (WINAPI *fnCreateFileW)(
    __in     LPCWSTR lpFileName,
    __in     DWORD dwDesiredAccess,
    __in     DWORD dwShareMode,
    __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __in     DWORD dwCreationDisposition,
    __in     DWORD dwFlagsAndAttributes,
    __in_opt HANDLE hTemplateFile
    );

typedef 
HMODULE (WINAPI *fnLoadLibraryA)(
    __in LPCSTR lpLibFileName
    );

typedef BOOL (WINAPI *fnCreateProcessW)(
    __in_opt    LPCWSTR lpApplicationName,
    __inout_opt LPWSTR lpCommandLine,
    __in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    __in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    __in        BOOL bInheritHandles,
    __in        DWORD dwCreationFlags,
    __in_opt    LPVOID lpEnvironment,
    __in_opt    LPCWSTR lpCurrentDirectory,
    __in        LPSTARTUPINFOW lpStartupInfo,
    __out       LPPROCESS_INFORMATION lpProcessInformation
    );

static BOOL g_fInit;
static fnCreateFileA g_pfnCreateFileA;
static fnCreateFileW g_pfnCreateFileW;
static fnLoadLibraryA g_pfnLoadLibraryA;
static fnCreateProcessW g_pfnCreateProcessW;
CString g_szCurrentDirectory;

char const* Q_stristr( char const* pStr, char const* pSearch )
{
	char const* pLetter;

	if (!pStr || !pSearch) 
		return 0;

	pLetter = pStr;

	// Check the entire string
	while (*pLetter != 0)
	{
		// Skip over non-matches
		if (tolower(*pLetter) == tolower(*pSearch))
		{
			// Check for match
			char const* pMatch = pLetter + 1;
			char const* pTest = pSearch + 1;
			while (*pTest != 0)
			{
				// We've run off the end; don't bother.
				if (*pMatch == 0)
					return 0;

				if (tolower(*pMatch) != tolower(*pTest))
					break;

				++pMatch;
				++pTest;
			}

			// Found a match!
			if (*pTest == 0)
				return pLetter;
		}

		++pLetter;
	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool IsSafeFileToDownload( const char *filename )
{
	char *last;
	char lwrfilename[4096];

	if ( !filename )
		return false;

	strncpy( lwrfilename, filename, 4095 );
	lwrfilename[4095] = 0;
	strlwr( lwrfilename );

	if ( strstr( lwrfilename, ".." ) )
	{
		return false;
	}

	last = strrchr( lwrfilename, '.' );

	if ( last == NULL )
		return true;

	if(!stricmp( last, ".cfg" ))
	{
		if(Q_stristr(lwrfilename, "\\config.cfg"))
			return true;

		return false;
	}

	if ( !stricmp( last, ".lst" ) || !stricmp( last, ".res" ) || 
		!stricmp( last, ".exe" ) || !stricmp( last, ".vbs" ) || 
		!stricmp( last, ".com" ) || !stricmp( last, ".bat" ) || 
		!stricmp( last, ".dll" ) || !stricmp( last, ".ini" ) || 
		!stricmp( last, ".log" ) ||  !stricmp( last, ".so" ) || 
		!stricmp( last, ".dylib" ) || !stricmp( last, ".sys" ) || 
		 !stricmp( last, ".asi"))
	{
		return false;
	}

	return true;
}

bool IsSafeFileToDownloadW( const wchar_t *filename )
{
	wchar_t *last;
	wchar_t lwrfilename[4096];

	if ( !filename )
		return false;

	wcsncpy( lwrfilename, filename, 4096 );
	lwrfilename[4095] = 0;
	wcslwr( lwrfilename );

	if ( wcsstr( lwrfilename, L".." ) )
	{
		return false;
	}

	last = wcsrchr( lwrfilename, L'.' );

	if ( last == NULL )
		return true;

	if(!wcsicmp( last, L".cfg" ))
	{
		if(wcsstr(lwrfilename, L"\\config.cfg"))
			return true;

		return false;
	}

	if ( !wcsicmp( last, L".lst" ) || !wcsicmp( last, L".res" ) || 
		!wcsicmp( last, L".exe" ) || !wcsicmp( last, L".vbs" ) || 
		!wcsicmp( last, L".com" ) || !wcsicmp( last, L".bat" ) || 
		!wcsicmp( last, L".dll" ) || !wcsicmp( last, L".ini" ) || 
		!wcsicmp( last, L".log" ) ||  !wcsicmp( last, L".so" ) || 
		!wcsicmp( last, L".dylib" ) || !wcsicmp( last, L".sys" ) || 
		!wcsicmp( last, L".asi"))
	{
		return false;
	}

	return true;
}

HMODULE WINAPI newLoadLibraryA(LPCSTR lpLibFileName)
{
	CString szPrefixDirectory;
	CString szLibFileName;
	int nIndex ;

	if(lpLibFileName == NULL)
		return g_pfnLoadLibraryA(lpLibFileName);

	szLibFileName = lpLibFileName;

	LPSTR lpTemp = szLibFileName.GetBuffer();
	size_t nLength = strlen(lpTemp);

	for(size_t i =0; i < nLength;i++)
	{
		if(lpTemp[i] == '/')
			lpTemp[i] = '\\';
	}

	_strlwr(lpTemp);

	szLibFileName.ReleaseBuffer();

	

	szPrefixDirectory.Format("%s\\cstrike", g_szCurrentDirectory);
	_strlwr(szPrefixDirectory.GetBuffer());
	szPrefixDirectory.ReleaseBuffer();

	nIndex = szLibFileName.Find(szPrefixDirectory);

	if(nIndex != -1)
	{
		szPrefixDirectory.Format("%s\\cstrike\\cl_dlls\\client.dll", g_szCurrentDirectory);
		_strlwr(szPrefixDirectory.GetBuffer());
		szPrefixDirectory.ReleaseBuffer();

		if(szPrefixDirectory == szLibFileName)
			return g_pfnLoadLibraryA(lpLibFileName);

		szPrefixDirectory.Format("%s\\cstrike\\dlls\\mp.dll", g_szCurrentDirectory);
		_strlwr(szPrefixDirectory.GetBuffer());
		szPrefixDirectory.ReleaseBuffer();

		if(szPrefixDirectory == szLibFileName)
			return g_pfnLoadLibraryA(lpLibFileName);

		SetLastError(ERROR_FILE_NOT_FOUND);
		return NULL;
	}

	szPrefixDirectory.Format("%s\\valve", g_szCurrentDirectory);
	_strlwr(szPrefixDirectory.GetBuffer());
	szPrefixDirectory.ReleaseBuffer();

	nIndex = szLibFileName.Find(szPrefixDirectory);

	if(nIndex != -1)
	{
		szPrefixDirectory.Format("%s\\valve\\cl_dlls\\client.dll", g_szCurrentDirectory);
		_strlwr(szPrefixDirectory.GetBuffer());
		szPrefixDirectory.ReleaseBuffer();

		if(szPrefixDirectory == szLibFileName)
			return g_pfnLoadLibraryA(lpLibFileName);

		szPrefixDirectory.Format("%s\\valve\\cl_dlls\\GameUI.dll", g_szCurrentDirectory);
		_strlwr(szPrefixDirectory.GetBuffer());
		szPrefixDirectory.ReleaseBuffer();

		if(szPrefixDirectory == szLibFileName)
			return g_pfnLoadLibraryA(lpLibFileName);

		szPrefixDirectory.Format("%s\\valve\\cl_dlls\\particleman.dll", g_szCurrentDirectory);
		_strlwr(szPrefixDirectory.GetBuffer());
		szPrefixDirectory.ReleaseBuffer();

		if(szPrefixDirectory == szLibFileName)
			return g_pfnLoadLibraryA(lpLibFileName);

		szPrefixDirectory.Format("%s\\valve\\dlls\\hl.dll", g_szCurrentDirectory);
		_strlwr(szPrefixDirectory.GetBuffer());
		szPrefixDirectory.ReleaseBuffer();

		if(szPrefixDirectory == szLibFileName)
			return g_pfnLoadLibraryA(lpLibFileName);

		SetLastError(ERROR_FILE_NOT_FOUND);
		return NULL;
	}


	return g_pfnLoadLibraryA(lpLibFileName);
}

HANDLE WINAPI newCreateFileA(
    __in     LPCSTR lpFileName,
    __in     DWORD dwDesiredAccess,
    __in     DWORD dwShareMode,
    __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __in     DWORD dwCreationDisposition,
    __in     DWORD dwFlagsAndAttributes,
    __in_opt HANDLE hTemplateFile
)
{
	if(!(dwDesiredAccess & GENERIC_WRITE))
	{
		return g_pfnCreateFileA(lpFileName, 
			dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	if(IsSafeFileToDownload(lpFileName))
	{
		return g_pfnCreateFileA(lpFileName, 
			dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	SetLastError(ERROR_ACCESS_DENIED);
	return INVALID_HANDLE_VALUE;
}

HANDLE WINAPI newCreateFileW(
    __in     LPCWSTR lpFileName,
    __in     DWORD dwDesiredAccess,
    __in     DWORD dwShareMode,
    __in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    __in     DWORD dwCreationDisposition,
    __in     DWORD dwFlagsAndAttributes,
    __in_opt HANDLE hTemplateFile
)
{
	if(!(dwDesiredAccess & GENERIC_WRITE))
	{
		return g_pfnCreateFileW(lpFileName, 
			dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	if(IsSafeFileToDownloadW(lpFileName))
	{
		return g_pfnCreateFileW(lpFileName, 
			dwDesiredAccess, dwShareMode, lpSecurityAttributes, 
			dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}

	SetLastError(ERROR_ACCESS_DENIED);
	return INVALID_HANDLE_VALUE;
}

BOOL WINAPI newCreateProcessW(
    __in_opt    LPCWSTR lpApplicationName,
    __inout_opt LPWSTR lpCommandLine,
    __in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
    __in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    __in        BOOL bInheritHandles,
    __in        DWORD dwCreationFlags,
    __in_opt    LPVOID lpEnvironment,
    __in_opt    LPCWSTR lpCurrentDirectory,
    __in        LPSTARTUPINFOW lpStartupInfo,
    __out       LPPROCESS_INFORMATION lpProcessInformation
)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return FALSE;
}

void FP_Initialize( void )
{
	if(g_fInit)
		return;

	GetCurrentDirectory(MAX_PATH, g_szCurrentDirectory.GetBuffer(MAX_PATH));
	g_szCurrentDirectory.ReleaseBuffer();
	
	g_pfnCreateFileA = (fnCreateFileA)GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileA");
	g_pfnCreateFileW = (fnCreateFileW)GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateFileW");
	g_pfnLoadLibraryA = (fnLoadLibraryA)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	g_pfnCreateProcessW = (fnCreateProcessW)GetProcAddress(GetModuleHandle("kernel32.dll"), "CreateProcessW");

	DetourTransactionBegin( );
	DetourAttach( (PVOID *)&g_pfnCreateFileA, newCreateFileA );
	DetourAttach( (PVOID *)&g_pfnCreateFileW, newCreateFileW );
	DetourAttach( (PVOID *)&g_pfnLoadLibraryA, newLoadLibraryA );
	DetourAttach( (PVOID *)&g_pfnCreateProcessW, newCreateProcessW );
	DetourTransactionCommit( );

	g_fInit = TRUE;
}

void FP_Shutdown( void )
{
	if(!g_fInit)
		return;

	DetourTransactionBegin( );
	DetourDetach( (PVOID *)&g_pfnCreateProcessW, newCreateProcessW );
	DetourDetach( (PVOID *)&g_pfnLoadLibraryA, newLoadLibraryA );
	DetourDetach( (PVOID *)&g_pfnCreateFileW, newCreateFileW );
	DetourDetach( (PVOID *)&g_pfnCreateFileA, newCreateFileA );
	DetourTransactionCommit( );

	g_fInit = FALSE;
}