//---------------------------------------------------------------------------
#include "URAIDAPCH.h"
#include <tlhelp32.h>
#pragma hdrstop

#include "UnitRAIDWorker.h"
#include "UnitCommon.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


const String TRAIDWorker::m_strRAIDWindowTitle = L"Raid: Shadow Legends";
const String TRAIDWorker::m_strRAIDWindowClass = L"UnityWndClass";
const String TRAIDWorker::m_strRAIDProcessName = L"Raid.exe";

//---------------------------------------------------------------------------
TRAIDWorker::TRAIDWorker()
{
	AdjustPrivilege(SE_DEBUG_NAME);

	this->GainRAIDWindow();

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
    this->GainRAIDWindow();
	return (IsWindow(m_hGameWindow) == TRUE);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::CaptureFrame(TCanvas *pDestination, const TSize& Size)
{
	this->GainRAIDWindow();
	if (!this->IsGameRunning())
		return false;

	TRect ClientRect = this->UpdateRecentFrame();
	StretchBlt(pDestination->Handle, 0, 0, Size.cx, Size.cy, m_hRecentFrameDC, 0, 0,
		ClientRect.Width(), ClientRect.Height(), SRCCOPY);

	return true;
}
//---------------------------------------------------------------------------
bool TRAIDWorker::ComparePixels(const TPoint& PositionInFrame, TColor KeyColor)
{
	this->GainRAIDWindow();
	if (!this->IsGameRunning())
		return false;

	this->UpdateRecentFrame();

	return (GetPixel(m_hRecentFrameDC, PositionInFrame.x, PositionInFrame.y) == static_cast<COLORREF>(KeyColor));
}
//---------------------------------------------------------------------------
TRect TRAIDWorker::GetGameWindowSize(bool bClient)
{
	this->GainRAIDWindow();

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

	return TRect(GameWindowSize);
}
//---------------------------------------------------------------------------
void TRAIDWorker::ResizeGameWindow(const TSize& NewSize)
{
	this->GainRAIDWindow();
	SetWindowPos(m_hGameWindow, HWND_TOP, 0, 0, NewSize.cx, NewSize.cy, SWP_NOMOVE);
}
//---------------------------------------------------------------------------
void TRAIDWorker::SendKey(System::WideChar Key)
{
	this->GainRAIDWindow();
	//Костыль для обхода обработчика нажатия у игры
	PostMessage(m_hGameWindow, WM_ACTIVATE, WA_INACTIVE, 0);
	PostMessage(m_hGameWindow, WM_ACTIVATE, WA_ACTIVE, 0);
	//Симулируем нажатие
	PostMessage(m_hGameWindow, WM_KEYDOWN, static_cast<WPARAM>(Key), 0);
	PostMessage(m_hGameWindow, WM_KEYUP, static_cast<WPARAM>(Key), 0);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::GainRAIDWindow()
{
	m_hGameWindow = FindWindow(m_strRAIDWindowClass.c_str(), m_strRAIDWindowTitle.c_str());

	return (m_hGameWindow != 0);
}
//---------------------------------------------------------------------------
void TRAIDWorker::CloseGame()
{
	//Так как RAID по закрытию обычным способом (WM_CLOSE, "крестик" окна) показывает диалог подтверждения,
    //используем радикальный способ решения задачи
	HANDLE hSnapshot;
	PROCESSENTRY32 PE;
	HANDLE hProcess;

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		PE.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnapshot, &PE);
		do {
			if (CompareStr(String(PE.szExeFile), m_strRAIDProcessName) == 0)
			{
				hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, PE.th32ProcessID);
				if (TerminateProcess(hProcess, 0) == 0)
				{
					//
				}
				CloseHandle(hProcess);
                break;
			}
		} while (Process32Next(hSnapshot, &PE));
	}

	CloseHandle(hSnapshot);
}
//---------------------------------------------------------------------------
TRect TRAIDWorker::UpdateRecentFrame()
{
	RECT ClientRect;
	WINDOWINFO WndInfo;
	GetWindowInfo(m_hGameWindow, &WndInfo);
	CopyRect(&ClientRect, &WndInfo.rcClient);

	HDC hRAIDDC = GetDC(m_hGameWindow);

	//Удаляем старый растр кадра и создаём актуальный
	if (m_hRecentFrame) {
		DeleteObject(m_hRecentFrame);
	}
	m_hRecentFrame = CreateCompatibleBitmap(hRAIDDC, ClientRect.right - ClientRect.left,
		ClientRect.bottom - ClientRect.top);
	SelectObject(m_hRecentFrameDC, m_hRecentFrame);

	//PrintWindow(m_hGameWindow, m_hRecentFrameDC, PW_CLIENTONLY);
	BitBlt(m_hRecentFrameDC, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top,
		hRAIDDC, 0, 0, SRCCOPY);

	ReleaseDC(m_hGameWindow, hRAIDDC);

	return TRect(ClientRect);
}
//---------------------------------------------------------------------------
