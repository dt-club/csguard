#include <Windows.h>
#include <WinNT.h>
#include <intrin.h>
#include "detours/detours.h"
#include <ImageHlp.h>
#include <math.h>
#include <atlstr.h>
#include "resource.h"


extern HINSTANCE g_hInstance;

BOOL WriteDataToFile( CString lpszFileName, LPVOID lpData, DWORD dwSize )
{
	HANDLE FileHandle;
	DWORD dwIoSize;

	DeleteFile(lpszFileName);

	FileHandle = CreateFile( lpszFileName,
							 GENERIC_WRITE,
							 0,
							 NULL,
							 CREATE_ALWAYS,
							 FILE_ATTRIBUTE_NORMAL,
							 NULL
	);

	if( FileHandle == INVALID_HANDLE_VALUE )
		return FALSE;

	if( !WriteFile( FileHandle, lpData, dwSize, &dwIoSize, NULL ) )
	{
		CloseHandle( FileHandle );
		return FALSE;
	}

	CloseHandle( FileHandle );

	return TRUE;
}


void WriteResourceFile(CString szFileName, INT nResourceID)
{
	HRSRC hRsrc;
	HGLOBAL hGlobalResource;
	LPVOID lpResourceData;
	DWORD dwResourceSize;
	BOOL fOK;

	hRsrc = NULL;
	hGlobalResource = NULL;
	lpResourceData = NULL;
	fOK = FALSE;

	do
	{
		hRsrc = FindResource( g_hInstance,
							  MAKEINTRESOURCE( nResourceID ),
							  TEXT( "GAMERES" )
		);
		if( !hRsrc )
			break;

		hGlobalResource = LoadResource( g_hInstance, hRsrc );
		if( !hGlobalResource )
			break;

		lpResourceData = LockResource( hGlobalResource );
		if( !lpResourceData )
			break;

		dwResourceSize = SizeofResource( g_hInstance, hRsrc );

		fOK = WriteDataToFile( szFileName,
							   lpResourceData,
								  dwResourceSize
		);

	} while( FALSE );


	if( hGlobalResource != NULL )
		FreeResource( hGlobalResource );
}


bool DiffInResource(CString szFileName, INT nResourceID)
{
	HANDLE FileHandle;
	DWORD dwSize;
	DWORD dwIoSize;
	static char szData[8192];
	HRSRC hRsrc;
	HGLOBAL hGlobalResource;
	LPVOID lpResourceData;
	DWORD dwResourceSize;

	FileHandle = CreateFileA(szFileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(FileHandle == INVALID_HANDLE_VALUE)
		return true;

	dwSize = GetFileSize(FileHandle, NULL);

	if(dwSize < sizeof(szData))
	{
		memset(szData, 0, sizeof(szData));

		if(!ReadFile(FileHandle, szData, dwSize, &dwIoSize, NULL))
		{
			CloseHandle(FileHandle);
			return false;
		}

		hRsrc = FindResource( g_hInstance,
							  MAKEINTRESOURCE( nResourceID ),
							  TEXT( "GAMERES" )
		);

		if(hRsrc == NULL)
		{
			CloseHandle(FileHandle);
			return false;
		}


		hGlobalResource = LoadResource( g_hInstance, hRsrc );
		if( hGlobalResource == NULL)
		{
			CloseHandle(FileHandle);
			return false;
		}

		lpResourceData = LockResource( hGlobalResource );
		if( lpResourceData == NULL )
		{
			FreeResource( hGlobalResource );
			CloseHandle(FileHandle);
			return false;
		}

		dwResourceSize = SizeofResource( g_hInstance, hRsrc );

		if( dwResourceSize != dwIoSize )
		{
			FreeResource( hGlobalResource );
			CloseHandle(FileHandle);
			return true;
		}

		if(!memcmp(lpResourceData, szData, dwIoSize))
		{
			FreeResource( hGlobalResource );
			CloseHandle(FileHandle);
			return false;
		}

		FreeResource( hGlobalResource );
		CloseHandle(FileHandle);
		return true;
	}

	CloseHandle(FileHandle);
	return false;
}

bool CheckFileHasConnectCommand(CString szFileName)
{
	FILE *fp;
	static char szLine[8192];

	fp = fopen(szFileName, "r");

	if( fp == NULL )
		return false;

	while( fgets(szLine, 8191, fp) != NULL )
	{
		_strlwr(szLine);
		if(strstr(szLine, "connect") != NULL)
		{
			fclose(fp);
			return true;
		}
	}

	fclose(fp);
	return false;
}

static void cstrike(CString szCurrentDirectory)
{
	CString szFileName;

	szFileName.Format("%s\\cstrike\\valve.rc", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\joystick.CFG", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\userconfig.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\autoexec.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\default.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\language.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\violence.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\server.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		WriteResourceFile(szFileName, IDR_CSTRIKE_SERVER_CFG);

	szFileName.Format("%s\\cstrike\\listenserver.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		WriteResourceFile(szFileName, IDR_CSTRIKE_SERVER_CFG);

	szFileName.Format("%s\\cstrike\\resource\\LoadingDialog.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\resource\\LoadingDialogNoBanner.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\resource\\LoadingDialogNoBannerSingle.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\resource\\LoadingDialogVAC.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike\\resource\\GameMenu.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
	{
		if(DiffInResource(szFileName, IDR_CSTRIKE_RESOURCE_GAMEMENU))
		{
			WriteResourceFile(szFileName, IDR_CSTRIKE_RESOURCE_GAMEMENU);
		}
	}

	szFileName.Format("%s\\cstrike\\hw", szCurrentDirectory);
	RemoveDirectory(szFileName);
}

static void cstrike_schinese(CString szCurrentDirectory)
{
	CString szFileName;

	szFileName.Format("%s\\cstrike_schinese\\valve.rc", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\joystick.CFG", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\userconfig.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\autoexec.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	

	szFileName.Format("%s\\cstrike_schinese\\default.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\language.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\violence.cfg", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\server.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\listenserver.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\resource\\LoadingDialog.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\resource\\LoadingDialogNoBanner.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\resource\\LoadingDialogNoBannerSingle.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\resource\\LoadingDialogVAC.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\resource\\GameMenu.res", szCurrentDirectory);
	if(PathFileExists(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\cstrike_schinese\\hw", szCurrentDirectory);
	RemoveDirectory(szFileName);
}


static void valve(CString szCurrentDirectory)
{
	CString szFileName;

	szFileName.Format("%s\\valve\\userconfig.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);


	szFileName.Format("%s\\valve\\autoexec.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_AUTOEXEC_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_AUTOEXEC_CFG);
	}

	szFileName.Format("%s\\valve\\default.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_DEFAULT_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_DEFAULT_CFG);
	}

	szFileName.Format("%s\\valve\\language.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_LANGUAGE_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_LANGUAGE_CFG);
	}

	szFileName.Format("%s\\valve\\valve.rc", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_VALVE_RC))
	{
		WriteResourceFile(szFileName, IDR_VALVE_VALVE_RC);
	}

	szFileName.Format("%s\\valve\\violence.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_VIOLENCE_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_VIOLENCE_CFG);
	}

	szFileName.Format("%s\\valve\\server.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
	{
		WriteResourceFile(szFileName, IDR_VALVE_SERVER_CFG);
	}

	szFileName.Format("%s\\valve\\listenserver.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
	{
		WriteResourceFile(szFileName, IDR_VALVE_LISTENSERVER_CFG);
	}

	szFileName.Format("%s\\valve\\resource\\LoadingDialog.res", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_LOADING_DIALOG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_LOADING_DIALOG);
	}

	szFileName.Format("%s\\valve\\resource\\LoadingDialogNoBanner.res", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_LOADING_DIALOG_NO_BANNER))
	{
		WriteResourceFile(szFileName, IDR_VALVE_LOADING_DIALOG_NO_BANNER);
	}

	szFileName.Format("%s\\valve\\resource\\LoadingDialogNoBannerSingle.res", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_LOADING_DIALOG_NO_BANNER_S))
	{
		WriteResourceFile(szFileName, IDR_VALVE_LOADING_DIALOG_NO_BANNER_S);
	}

	szFileName.Format("%s\\valve\\resource\\LoadingDialogVAC.res", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_LOADING_DIALOG_VAC))
	{
		WriteResourceFile(szFileName, IDR_VALVE_LOADING_DIALOG_VAC);
	}

	szFileName.Format("%s\\valve\\resource\\GameMenu.res", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_RESOURCE_GAMEMENU))
	{
		WriteResourceFile(szFileName, IDR_VALVE_RESOURCE_GAMEMENU);
	}

	szFileName.Format("%s\\valve\\hw\\ATIRage128.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_HW_ATIRAGE128_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_HW_ATIRAGE128_CFG);
	}

	szFileName.Format("%s\\valve\\hw\\ATIRage128d3d.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_HW_ATIRAGE128D3D_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_HW_ATIRAGE128D3D_CFG);
	}

	szFileName.Format("%s\\valve\\hw\\d3d.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_HW_D3D_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_HW_D3D_CFG);
	}

	szFileName.Format("%s\\valve\\hw\\geforce.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_HW_GEFORCE_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_HW_GEFORCE_CFG);
	}

	szFileName.Format("%s\\valve\\hw\\nvidiad3d.cfg", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_HW_NVIDIAD3D_CFG))
	{
		WriteResourceFile(szFileName, IDR_VALVE_HW_NVIDIAD3D_CFG);
	}

	szFileName.Format("%s\\valve\\steamcomm.lst", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_STEAMCOMM_LST))
	{
		WriteResourceFile(szFileName, IDR_VALVE_STEAMCOMM_LST);
	}

	szFileName.Format("%s\\valve\\valvecomm.lst", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_VALVE_VALVECOMM_LST))
	{
		WriteResourceFile(szFileName, IDR_VALVE_VALVECOMM_LST);
	}
}

static void valve_schinese(CString szCurrentDirectory)
{
	CString szFileName;

	szFileName.Format("%s\\valve_schinese\\userconfig.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);


	szFileName.Format("%s\\valve_schinese\\autoexec.cfg", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\default.cfg", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\language.cfg", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\valve.rc", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\violence.cfg", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\server.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\listenserver.cfg", szCurrentDirectory);
	if(CheckFileHasConnectCommand(szFileName))
		DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\resource\\LoadingDialog.res", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\resource\\LoadingDialogNoBanner.res", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\resource\\LoadingDialogNoBannerSingle.res", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\resource\\LoadingDialogVAC.res", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\resource\\GameMenu.res", szCurrentDirectory);
	DeleteFile(szFileName);

	szFileName.Format("%s\\valve_schinese\\hw", szCurrentDirectory);
	RemoveDirectory(szFileName);
}


void RemoveAndFixIllegalResources()
{
	CString szCurrentDirectory;
	CString szFileName;

	GetCurrentDirectory(MAX_PATH, szCurrentDirectory.GetBuffer(MAX_PATH));
	szCurrentDirectory.ReleaseBuffer();

	cstrike(szCurrentDirectory);
	cstrike_schinese(szCurrentDirectory);
	valve(szCurrentDirectory);
	valve_schinese(szCurrentDirectory);

	szFileName.Format("%s\\platform\\config\\MasterServers.vdf", szCurrentDirectory);
	if(DiffInResource(szFileName, IDR_MASTERSERVER_VDF))
	{
		WriteResourceFile(szFileName, IDR_MASTERSERVER_VDF);
	}
}