//---------------------------------------------------------------------------

#ifndef UnitRAIDWorkerH
#define UnitRAIDWorkerH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>

class TRAIDWorker
{
public:
	TRAIDWorker();
	~TRAIDWorker();

	bool IsGameRunning();
	bool CaptureFrame(TCanvas *pDestination, const TSize& Size);
	bool ComparePixels(const TPoint& PositionInFrame, TColor KeyColor, unsigned int uTolerance = 1);
	TRect GetGameWindowSize(bool bClient = false);
	void ResizeGameWindow(const TSize& NewSize);
	void SendKey(System::WideChar Key);
	void SendMouseClick(const TPoint& Coordinates);
	HWND GetGameWindowHandle() { this->GainGameWindow(); return m_hGameWindow; }
	void CloseGame();
	void ValidateGameWindow();
	bool CheckGameStateWithMessage(TWinControl* pParent);

private:
	static const String m_strRAIDWindowTitle;
	static const String m_strRAIDWindowClass;
    static const String m_strRAIDProcessName;

	HWND m_hGameWindow;
	HDC m_hRecentFrameDC;
	HBITMAP m_hRecentFrame;

	bool GainGameWindow();
	TRect UpdateRecentFrame();
};

#endif
