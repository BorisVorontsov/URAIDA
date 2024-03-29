//---------------------------------------------------------------------------
#include "URAIDAPCH.h"
#include <tlhelp32.h>
#include <VersionHelpers.h>
#pragma hdrstop

#include "UnitRAIDWorker.h"
#include "UnitCommon.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


const String TRAIDWorker::m_strRAIDWindowTitle = L"Raid: Shadow Legends";
const String TRAIDWorker::m_strRAIDWindowClass = L"UnityWndClass";
const String TRAIDWorker::m_strRAIDProcessName = L"Raid.exe";
const String TRAIDWorker::m_strLauncherName = L"PlariumPlay.exe";

//---------------------------------------------------------------------------
TRAIDWorker::TRAIDWorker()
{
	AdjustPrivilege(SE_DEBUG_NAME);

	this->GainGameWindow();

	HDC hRAIDDC = GetDC(m_hGameWindow);
	m_hRecentFrameDC = CreateCompatibleDC(hRAIDDC);
    ReleaseDC(m_hGameWindow, hRAIDDC);

	this->UpdateRecentFrame();
}
//---------------------------------------------------------------------------
TRAIDWorker::~TRAIDWorker()
{
	DeleteDC(m_hRecentFrameDC);
	DeleteObject(m_hRecentFrame);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::IsGameRunning()
{
    this->GainGameWindow();
	return (IsWindow(m_hGameWindow) == TRUE);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::CaptureFrame()
{
	if (!this->IsGameRunning())
		return false;

	this->UpdateRecentFrame();
	TSize FrameSize;
	this->GetFrameSize(FrameSize);

	return ((FrameSize.cx > 0) && (FrameSize.cy > 0));
}
//---------------------------------------------------------------------------
bool TRAIDWorker::GetFrameSize(TSize& Result)
{
	if (!m_hRecentFrame)
		return false;

	BITMAP RecentFrame = {0};
	if (GetObject(m_hRecentFrame, sizeof(BITMAP), &RecentFrame))
	{
		Result = TSize(RecentFrame.bmWidth, RecentFrame.bmHeight);
		return true;
	}

	return false;
}
//---------------------------------------------------------------------------
bool TRAIDWorker::DrawFrame(TCanvas *pDestination, const TSize& Size)
{
	this->GainGameWindow();
	if (!this->IsGameRunning())
		return false;

	TSize FrameSize;
	this->GetFrameSize(FrameSize);
	//StretchBlt(pDestination->Handle, 0, 0, Size.cx, Size.cy, m_hRecentFrameDC, 0, 0,
	//	FrameSize.cx, FrameSize.cy, SRCCOPY);
	BitBlt(pDestination->Handle, 0, 0, FrameSize.cx, FrameSize.cy, m_hRecentFrameDC,
		0, 0, SRCCOPY);

	return true;
}
//---------------------------------------------------------------------------
bool TRAIDWorker::ComparePixels(const TPoint& PositionInFrame, TColor KeyColor, unsigned int uTolerance)
{
	this->GainGameWindow();
	if (!this->IsGameRunning())
		return false;

	COLORREF FramePixel = GetPixel(m_hRecentFrameDC, PositionInFrame.x, PositionInFrame.y);
	unsigned int uDistance =
		(GetRValue(FramePixel) - GetRValue(KeyColor)) ^ 2 +
		(GetGValue(FramePixel) - GetGValue(KeyColor)) ^ 2 +
		(GetBValue(FramePixel) - GetBValue(KeyColor)) ^ 2;
	uDistance = sqrt(uDistance);
	return (uDistance <= uTolerance);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::GetGameWindowSize(TRect& Result, bool bClient)
{
	this->GainGameWindow();
	if (!this->IsGameRunning())
		return false;

	RECT GameWindowSize;
	WINDOWPLACEMENT WindowPlacement;

	ZeroMemory(&WindowPlacement, sizeof(WINDOWPLACEMENT));
	WindowPlacement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(m_hGameWindow, &WindowPlacement);

	if (WindowPlacement.showCmd == SW_SHOWMINIMIZED)
	{
		ShowWindow(m_hGameWindow, SW_RESTORE);
	}

	if (bClient) {
		GetClientRect(m_hGameWindow, &GameWindowSize);
	} else {
		GetWindowRect(m_hGameWindow, &GameWindowSize);
	}

	Result = TRect(GameWindowSize);
	return true;
}
//---------------------------------------------------------------------------
void TRAIDWorker::ResizeGameWindow(const TSize& NewSize)
{
	this->GainGameWindow();

	TRect CurrentSize;
	this->GetGameWindowSize(CurrentSize, false);

	if ((CurrentSize.Width() != NewSize.cx) || (CurrentSize.Height() != NewSize.cy))
	{
		RECT NewWindowSize;
		SetRect(&NewWindowSize, 0, 0, NewSize.cx, NewSize.cy);
		AdjustWindowRectEx(&NewWindowSize, GetWindowLong(m_hGameWindow, GWL_STYLE),
			FALSE, GetWindowLong(m_hGameWindow, GWL_EXSTYLE));
		SetWindowPos(m_hGameWindow, NULL, 0, 0, NewWindowSize.right - NewWindowSize.left,
			NewWindowSize.bottom - NewWindowSize.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);

		//�������������� ����� ��� ����������� ����� ���� ����� ���������������
		Wait(1000);
    }
}
//---------------------------------------------------------------------------
void TRAIDWorker::SendKey(System::WideChar Key)
{
	this->GainGameWindow();

	//������� ��� ������ ����������� ������� � ����
	//���������� ����, ��� � ���� �������
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_INACTIVE, 0);
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_ACTIVE, 0);
	//���������� �������
	SendMessage(m_hGameWindow, WM_KEYDOWN, static_cast<WPARAM>(Key), 0);
	SendMessage(m_hGameWindow, WM_KEYUP, static_cast<WPARAM>(Key), 0);
}
//---------------------------------------------------------------------------
void TRAIDWorker::SendMouseClick(const TPoint& Coordinates)
{
	this->GainGameWindow();

	//������� ��� ������ ����������� ������� � ����
	//���������� ����, ��� � ���� �������
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_INACTIVE, 0);
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_ACTIVE, 0);

	//���������� ����
    //��� ������� ������������� ����������� ������� ��� ������ ����
	POINT CurrentCursorPos, ScreenCoords;
    RECT Clip, OldClip;

	GetCursorPos(&CurrentCursorPos);
	ScreenCoords = Coordinates;
	ClientToScreen(m_hGameWindow, &ScreenCoords);
	SetCursorPos(ScreenCoords.x, ScreenCoords.y);

	SendMessage(m_hGameWindow, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(Coordinates.x, Coordinates.y));
	SendMessage(m_hGameWindow, WM_LBUTTONUP, 0, MAKELPARAM(Coordinates.x, Coordinates.y));

	//������������� ��������� �� ����� ������ ��� ���������� ��������� ����� �����
	GetClipCursor(&OldClip);

	SetRect(&Clip, ScreenCoords.x, ScreenCoords.y, ScreenCoords.x + 1, ScreenCoords.y + 1);
	ClipCursor(&Clip);

	Wait(1000);

	ClipCursor(&OldClip);
	SetCursorPos(CurrentCursorPos.x, CurrentCursorPos.y);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::GainGameWindow()
{
	if (IsWindow(m_hGameWindow)) return true;

	m_hGameWindow = FindWindow(m_strRAIDWindowClass.c_str(), m_strRAIDWindowTitle.c_str());

	return (m_hGameWindow != 0);
}
//---------------------------------------------------------------------------
void TRAIDWorker::CloseGame()
{
	//��� ��� RAID �� �������� ������� �������� (WM_CLOSE, "�������" ����) ���������� ������ �������������,
	//��� �� �� ��������� ����� �������� ��� ������ � ���������� ����������� ������ ������� ������

	auto TerminateProcessByName
	{
		[](String strProcessName)
		{
			HANDLE hSnapshot;
			PROCESSENTRY32 PE;
			HANDLE hProcess;

			hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hSnapshot != INVALID_HANDLE_VALUE)
			{
				PE.dwSize = sizeof(PROCESSENTRY32);
				Process32First(hSnapshot, &PE);
				do {
					if (CompareStr(String(PE.szExeFile), strProcessName) == 0)
					{
						hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, PE.th32ProcessID);
						if (TerminateProcess(hProcess, 0) == 0)
						{
							//�� ����������
						}
						CloseHandle(hProcess);
						break;
					}
				} while (Process32Next(hSnapshot, &PE));
			}
			CloseHandle(hSnapshot);
        }
	};

	//��������� ������� ����
	TerminateProcessByName(m_strRAIDProcessName);
	//��������� ��� �������� ��������
	TerminateProcessByName(m_strLauncherName);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::UpdateRecentFrame()
{
	this->GainGameWindow();
	if (!this->IsGameRunning())
		return false;

	RECT ClientRect;
	WINDOWINFO WndInfo;
	GetWindowInfo(m_hGameWindow, &WndInfo);
	CopyRect(&ClientRect, &WndInfo.rcClient);

	HDC hRAIDDC = GetDC(m_hGameWindow);

	m_hRecentFrame = CreateCompatibleBitmap(hRAIDDC, ClientRect.right - ClientRect.left,
		ClientRect.bottom - ClientRect.top);

	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(m_hRecentFrameDC, m_hRecentFrame));
	//������� ������ ����� �����
	if (hOldBitmap && (hOldBitmap != HGDI_ERROR))
	{
		DeleteObject(hOldBitmap);
	}

	if (IsWindows8OrGreater())
	{
		PrintWindow(m_hGameWindow, m_hRecentFrameDC, PW_CLIENTONLY | PW_RENDERFULLCONTENT);
	}
	else if (IsWindowsVistaOrGreater())
	{
		BitBlt(m_hRecentFrameDC, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top,
			hRAIDDC, 0, 0, SRCCOPY);
	}

	ReleaseDC(m_hGameWindow, hRAIDDC);

	return true;
}
//---------------------------------------------------------------------------
void TRAIDWorker::ValidateGameWindow()
{
	//������� �������� � ������������� � ������ ������������� �������� � ��������� ���� ����
	TRect GameWindowSize;
	this->GetGameWindowSize(GameWindowSize, true);
	TSize GWSizeFromSettings = g_pSettingsManager->RAIDWindowSize;
	if ((GameWindowSize.Width() != GWSizeFromSettings.cx) || (GameWindowSize.Height() != GWSizeFromSettings.cy))
	{
		//������������� ������� ���� ��� ���������� ������ ���������
		this->ResizeGameWindow(GWSizeFromSettings);
	}
}
//---------------------------------------------------------------------------
bool TRAIDWorker::CheckGameStateWithMessage(TWinControl* pParent)
{
	if (!this->IsGameRunning())
	{
		MessageBox(pParent->Handle, L"�� ������ ����� ���� ����! ������ ��������", L"��������������",
			MB_ICONEXCLAMATION);

		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
