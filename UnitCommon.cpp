//---------------------------------------------------------------------------
#include "URAIDAPCH.h"
#include <shlobj.h>
#include <timeapi.h>
#pragma hdrstop

#include "UnitCommon.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

bool GetFileVersion(String strFileName, String& strResult, FVFormat Format)
{
	DWORD dwDummy;
	BYTE *bBuffer;
	VS_FIXEDFILEINFO *VS_BUFF;
	DWORD dwBufferLen;
	UINT uVerBufferLen;

	dwBufferLen = GetFileVersionInfoSize(strFileName.c_str(), &dwDummy);
	if (dwBufferLen < 1)
	{
		strResult = L"none";
		return false;
	}
	bBuffer = new BYTE[dwBufferLen];
	GetFileVersionInfo(strFileName.c_str(), NULL, dwBufferLen, &bBuffer[0]);
	VerQueryValue(&bBuffer[0], L"\\", reinterpret_cast<PVOID*>(&VS_BUFF), &uVerBufferLen);

	switch (Format)
	{
		case fvfMajorMinor:
			strResult.sprintf(L"%i.%i", HIWORD(VS_BUFF->dwProductVersionMS), LOWORD(VS_BUFF->dwProductVersionMS));
			break;
		case fvfMajorMinorBuild:
			strResult.sprintf(L"%i.%i.%i", HIWORD(VS_BUFF->dwProductVersionMS), LOWORD(VS_BUFF->dwProductVersionMS),
				LOWORD(VS_BUFF->dwProductVersionLS));
			break;
		case fvfMajorMinorReleaseBuild:
			strResult.sprintf(L"%i.%i.%i.%i", HIWORD(VS_BUFF->dwProductVersionMS), LOWORD(VS_BUFF->dwProductVersionMS),
				HIWORD(VS_BUFF->dwProductVersionLS), LOWORD(VS_BUFF->dwProductVersionLS));
			break;
	}

	delete[] bBuffer;

	return true;
}
//---------------------------------------------------------------------------
bool BrowseForFolderDialog(HWND hWnd, String& strFolder, const String& strTitle)
{
	HRESULT hResult = CoInitialize(NULL);
	if (FAILED(hResult)) return false;

	BROWSEINFO BI = {};
	LPITEMIDLIST pIIDL;
	BI.hwndOwner = hWnd;
	BI.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX;
	BI.lpszTitle = strTitle.c_str();
	BI.lpfn = BFFCallbackProc;
	pIIDL = SHBrowseForFolder(&BI);
	if (!pIIDL)
	{
		CoUninitialize();
		return false;
	}
	SHGetPathFromIDList(pIIDL, strFolder.c_str());

	CoTaskMemFree(pIIDL);
	if (hResult == S_OK)
		CoUninitialize();

	return true;
}
//---------------------------------------------------------------------------
int CALLBACK BFFCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg == BFFM_INITIALIZED)
	{
		if (lpData)
			SendMessage(hWnd, BFFM_SETSELECTION, TRUE, lpData);
	}
	return 0;
}

//---------------------------------------------------------------------------
void AdjustPrivilege(String strPrivilege)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES TKP = {};
	TOKEN_PRIVILEGES TKPOLD = {};
	DWORD dwRetLen;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LookupPrivilegeValue(0, strPrivilege.c_str(), &TKP.Privileges[0].Luid);
		TKP.PrivilegeCount = 1;
		TKP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, 0, &TKP, sizeof(TOKEN_PRIVILEGES), &TKPOLD, &dwRetLen);
	}
}
//---------------------------------------------------------------------------
void Wait(DWORD dwMilliseconds)
{
	timeBeginPeriod(1);

	DWORD dwStartTime = timeGetTime();

	while ((timeGetTime() - dwStartTime) < dwMilliseconds)
	{
		Application->ProcessMessages();
	}

	timeEndPeriod(1);
}
//---------------------------------------------------------------------------

