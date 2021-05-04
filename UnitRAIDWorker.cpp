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
bool TRAIDWorker::CaptureFrame(TCanvas *pDestination, const TSize& Size)
{
	this->GainGameWindow();
	if (!this->IsGameRunning())
		return false;

	TRect ClientRect = this->UpdateRecentFrame();
	//StretchBlt(pDestination->Handle, 0, 0, Size.cx, Size.cy, m_hRecentFrameDC, 0, 0,
	//	ClientRect.Right - ClientRect.Left, ClientRect.Bottom - ClientRect.Top, SRCCOPY);
	BitBlt(pDestination->Handle, 0, 0, ClientRect.Right - ClientRect.Left,
		ClientRect.Bottom - ClientRect.Top, m_hRecentFrameDC, 0, 0, SRCCOPY);

	return true;
}
//---------------------------------------------------------------------------
bool TRAIDWorker::ComparePixels(const TPoint& PositionInFrame, TColor KeyColor, unsigned int uTolerance)
{
	this->GainGameWindow();
	if (!this->IsGameRunning())
		return false;

	this->UpdateRecentFrame();
	COLORREF FramePixel = GetPixel(m_hRecentFrameDC, PositionInFrame.x, PositionInFrame.y);
	unsigned int uDistance =
		(GetRValue(FramePixel) - GetRValue(KeyColor)) ^ 2 +
		(GetGValue(FramePixel) - GetGValue(KeyColor)) ^ 2 +
		(GetBValue(FramePixel) - GetBValue(KeyColor)) ^ 2;
	uDistance = sqrt(uDistance);
	return (uDistance <= uTolerance);
}
//---------------------------------------------------------------------------
TRect TRAIDWorker::GetGameWindowSize(bool bClient)
{
	this->GainGameWindow();

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
	this->GainGameWindow();

	RECT NewWindowSize;
	SetRect(&NewWindowSize, 0, 0, NewSize.cx, NewSize.cy);
	AdjustWindowRectEx(&NewWindowSize, GetWindowLong(m_hGameWindow, GWL_STYLE),
		FALSE, GetWindowLong(m_hGameWindow, GWL_EXSTYLE));
	SetWindowPos(m_hGameWindow, NULL, 0, 0, NewWindowSize.right - NewWindowSize.left,
		NewWindowSize.bottom - NewWindowSize.top, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
}
//---------------------------------------------------------------------------
void TRAIDWorker::SendKey(System::WideChar Key)
{
	this->GainGameWindow();

	//Костыль для обхода обработчика нажатия у игры
	//Обманываем игру, что её окно активно
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_INACTIVE, 0);
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_ACTIVE, 0);
	//Симулируем нажатие
	SendMessage(m_hGameWindow, WM_KEYDOWN, static_cast<WPARAM>(Key), 0);
	SendMessage(m_hGameWindow, WM_KEYUP, static_cast<WPARAM>(Key), 0);
}
//---------------------------------------------------------------------------
void TRAIDWorker::SendMouseClick(const TPoint& Coordinates)
{
	this->GainGameWindow();

	//Костыль для обхода обработчика нажатия у игры
	//Обманываем игру, что её окно активно
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_INACTIVE, 0);
	SendMessage(m_hGameWindow, WM_ACTIVATE, WA_ACTIVE, 0);

	//Симулируем клик
    //Это требует моментального перемещения курсора для обмана игры
	POINT CurrentCursorPos, ScreenCoords;

	GetCursorPos(&CurrentCursorPos);
	ScreenCoords = Coordinates;
	ClientToScreen(m_hGameWindow, &ScreenCoords);
	SetCursorPos(ScreenCoords.x, ScreenCoords.y);

	SendMessage(m_hGameWindow, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(Coordinates.x, Coordinates.y));
	SendMessage(m_hGameWindow, WM_LBUTTONUP, 0, MAKELPARAM(Coordinates.x, Coordinates.y));

	SetCursorPos(CurrentCursorPos.x, CurrentCursorPos.y);
}
//---------------------------------------------------------------------------
bool TRAIDWorker::GainGameWindow()
{
	m_hGameWindow = FindWindow(m_strRAIDWindowClass.c_str(), m_strRAIDWindowTitle.c_str());

	return (m_hGameWindow != 0);
}
//---------------------------------------------------------------------------
void TRAIDWorker::CloseGame()
{
	//Так как RAID по закрытию обычным способом (WM_CLOSE, "крестик" окна) показывает диалог подтверждения,
	//что бы не добавлять новых настроек для кнопки – используем радикальный способ решения задачи
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
					//Не получилось
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
    this->GainGameWindow();

	RECT ClientRect;
	WINDOWINFO WndInfo;
	GetWindowInfo(m_hGameWindow, &WndInfo);
	CopyRect(&ClientRect, &WndInfo.rcClient);

	HDC hRAIDDC = GetDC(m_hGameWindow);

	m_hRecentFrame = CreateCompatibleBitmap(hRAIDDC, ClientRect.right - ClientRect.left,
		ClientRect.bottom - ClientRect.top);

	HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(m_hRecentFrameDC, m_hRecentFrame));
	//Удаляем старый растр кадра
	if (hOldBitmap && (hOldBitmap != HGDI_ERROR))
	{
		DeleteObject(hOldBitmap);
	}

	//PrintWindow(m_hGameWindow, m_hRecentFrameDC, PW_CLIENTONLY);
	BitBlt(m_hRecentFrameDC, 0, 0, ClientRect.right - ClientRect.left, ClientRect.bottom - ClientRect.top,
		hRAIDDC, 0, 0, SRCCOPY);

	ReleaseDC(m_hGameWindow, hRAIDDC);

	return TRect(ClientRect);
}
//---------------------------------------------------------------------------
void TRAIDWorker::ValidateGameWindow()
{
	//Функция проверки и корректировки в случае необходимости размеров и состояния окна игры
	TRect GameWindowSize = this->GetGameWindowSize(true);
	TSize GWSizeFromSettings = g_pSettingsManager->RAIDWindowSize;
	if ((GameWindowSize.Width() != GWSizeFromSettings.cx) || (GameWindowSize.Height() != GWSizeFromSettings.cy))
	{
		//Устанавливаем размеры окна для корректной работы алгоритма
		this->ResizeGameWindow(GWSizeFromSettings);
	}
}
//---------------------------------------------------------------------------
bool TRAIDWorker::CheckGameStateWithMessage(TWinControl* pParent)
{
	if (!this->IsGameRunning())
	{
		MessageBox(pParent->Handle, L"Не удаётся найти окно игры! Отмена действия", L"Предупреждение",
			MB_ICONEXCLAMATION);

		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
