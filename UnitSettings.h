//---------------------------------------------------------------------------

#ifndef UnitSettingsH
#define UnitSettingsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IniFiles.hpp>

//��� ���������� ����������
typedef enum tagResultSavingMode
{
	rsmAtTheEndOfEachBattle = 0,
	rsmPeriodically
} ResultSavingMode;

//�������� �� ���������� ������
typedef enum tagTaskEndAction
{
	teaDoNothing = 0,
	teaShowNotification,
	teaPlayAlert,
	teaGoToSleep,
	teaTurnOffPC
} TaskEndAction;

typedef enum tagSupportedGameModes
{
	gmCampaign = 1,
	gmDungeons,
	gmFactionWars
} SupportedGameModes;

//������ ��������� ��� ��������� ������� �������
typedef struct tagGameModeSpecSettings
{
    SupportedGameModes GameMode;
	bool bProcessSTARTScreenFirst;
	TPoint STARTScreenControlPoint;
	TColor STARTScreenControlPointColor;
	TPoint REPLAYScreenControlPoint;
	TColor REPLAYScreenControlPointColor;
	unsigned int uDelay;
	unsigned int nNumberOfBattles;

	tagGameModeSpecSettings() { ZeroMemory(this, sizeof(*this)); }
} GameModeSpecSettings;

const String g_strDefaultResultsFolder = L"Results";

//�������� �������� (������-���������� ������ �� *.ini)
class TSettingsManager
{
public:
	TSettingsManager();
	~TSettingsManager();

	bool ReadINI(); //������ ���� �������� �� *.ini
	bool UpdateINI(); //������ ���� �������� � *.ini

	__property GameModeSpecSettings CampaignSettings = { read = GetCampaignSettings, write = SetCampaignSettings };
	__property GameModeSpecSettings DungeonsSettings = { read = GetDungeonsSettings, write = SetDungeonsSettings };
	__property GameModeSpecSettings FactionWarsSettings = { read = GetFactionWarsSettings, write = SetFactionWarsSettings };

	__property bool AutoResizeGameWindow = { read = GetAutoResizeGameWnd, write = SetAutoResizeGameWnd };
	__property TSize RAIDWindowSize = { read = GetGameWindowSize, write = SetGameWindowSize };

	__property bool SaveResults = { read = GetSaveResults, write = SetSaveResults };
	__property ResultSavingMode ResultSavingMethod = { read = GetResultSavingMode, write = SetResultSavingMode };
	__property unsigned int ResultSavingPeriod = { read = GetResultSavingPeriod, write = SetResultSavingPeriod };
	__property String PathForResults = { read = GetPathForResults, write = SetPathForResults };

	__property TaskEndAction TaskEndBehavior = { read = GetTaskEndBehavior, write = SetTaskEndBehavior };
	__property bool ExitOnTaskEnding = { read = GetExitOnTaskEnding, write = SetExitOnTaskEnding };
	__property bool CloseGameOnTaskEnding = { read = GetCloseGameOnTaskEnding, write = SetCloseGameOnTaskEnding };

	__property unsigned int TriesBeforeForceTaskEnding = { read = GetTriesBeforeForceTaskEnding, write = SetTriesBeforeForceTaskEnding };
	__property unsigned int ScreenCheckingPeriod = { read = GetScreenCheckingPeriod, write = SetScreenCheckingPeriod };

	__property unsigned int RecentActivePage = { read = GetRecentActivePage, write = SetRecentActivePage };

private:
	static const String m_strSettingsFileName;

	static const String m_strSectionCampaign;
	static const String m_strSectionDungeons;
	static const String m_strSectionFactionWars;
	static const String m_strSectionCommon;
    static const String m_strSectionInternal;

	GameModeSpecSettings m_CampaignSettings;
	GameModeSpecSettings m_DungeonsSettings;
	GameModeSpecSettings m_FactionWarsSettings;

	bool m_bAutoResizeWindow;
	TSize m_GameWindowSize;

	bool m_bSaveResults;
	ResultSavingMode m_ResultSavingMode;
	unsigned int m_uResultSavingPeriod;
	String m_strPathForResults;

	TaskEndAction m_TaskEndAction;
	bool m_bExitOnTaskEnding;
	bool m_bCloseGameOnTaskEnding;

	unsigned int m_uTriesBeforeForceTaskEnding;
	unsigned int m_uScreenCheckingPeriod;

	unsigned int m_uRecentActivePageIndex;

	GameModeSpecSettings GetCampaignSettings() { return m_CampaignSettings; }
	void SetCampaignSettings(GameModeSpecSettings NewValue) { m_CampaignSettings = NewValue; }
	GameModeSpecSettings GetDungeonsSettings() { return m_DungeonsSettings; }
	void SetDungeonsSettings(GameModeSpecSettings NewValue) { m_DungeonsSettings = NewValue; }
	GameModeSpecSettings GetFactionWarsSettings() { return m_FactionWarsSettings; }
	void SetFactionWarsSettings(GameModeSpecSettings NewValue) { m_FactionWarsSettings = NewValue; }

	bool GetAutoResizeGameWnd() { return m_bAutoResizeWindow; }
	void SetAutoResizeGameWnd(bool bNewValue) { m_bAutoResizeWindow = bNewValue; }

	TSize GetGameWindowSize() { return m_GameWindowSize; }
	void SetGameWindowSize(TSize NewValue) { m_GameWindowSize = NewValue; }

	bool GetSaveResults() { return m_bSaveResults; }
	void SetSaveResults(bool bNewValue) { m_bSaveResults = bNewValue; }

	ResultSavingMode GetResultSavingMode() { return m_ResultSavingMode; }
	void SetResultSavingMode(ResultSavingMode NewValue) { m_ResultSavingMode = NewValue; }

	unsigned int GetResultSavingPeriod() { return m_uResultSavingPeriod; }
	void SetResultSavingPeriod(unsigned int uNewValue) { m_uResultSavingPeriod = uNewValue; }

	String GetPathForResults() { return m_strPathForResults; }
	void SetPathForResults(String strNewValue) { m_strPathForResults = strNewValue; }

	TaskEndAction GetTaskEndBehavior() { return m_TaskEndAction; }
	void SetTaskEndBehavior(TaskEndAction NewValue) { m_TaskEndAction = NewValue; }

	bool GetExitOnTaskEnding() { return m_bExitOnTaskEnding; }
	void SetExitOnTaskEnding(bool bNewValue) { m_bExitOnTaskEnding = bNewValue; }

	bool GetCloseGameOnTaskEnding() { return m_bCloseGameOnTaskEnding; }
	void SetCloseGameOnTaskEnding(bool bNewValue) { m_bCloseGameOnTaskEnding = bNewValue; }

	unsigned int GetTriesBeforeForceTaskEnding() { return m_uTriesBeforeForceTaskEnding; }
	void SetTriesBeforeForceTaskEnding(unsigned int uNewValue) { m_uTriesBeforeForceTaskEnding = uNewValue; }

	unsigned int GetScreenCheckingPeriod() { return m_uScreenCheckingPeriod; }
	void SetScreenCheckingPeriod(unsigned int uNewValue) { m_uScreenCheckingPeriod = uNewValue; }

	unsigned int GetRecentActivePage() { return m_uRecentActivePageIndex; }
	void SetRecentActivePage(unsigned int uNewValue) { m_uRecentActivePageIndex = uNewValue; }
};

//---------------------------------------------------------------------------
#endif
