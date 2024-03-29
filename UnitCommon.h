//---------------------------------------------------------------------------

#ifndef UnitCommonH
#define UnitCommonH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>

typedef enum tagFVFormat
{
	fvfMajorMinor = 1,
    fvfMajorMinorRelease,
	fvfMajorMinorBuild,
	fvfMajorMinorReleaseBuild
} FVFormat;

bool GetFileVersion(String strFileName, String& strResult, FVFormat Format);
bool BrowseForFolderDialog(HWND hWnd, String& strFolder, const String& strTitle);
int CALLBACK BFFCallbackProc(HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData);
void AdjustPrivilege(String strPrivilege);
void Wait(DWORD dwMilliseconds);
bool SplitCommandToFileAndArgs(const String& strCommand, String& strFile, String& strArgs);

#endif
