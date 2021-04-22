//---------------------------------------------------------------------------

#ifndef UnitSettingsH
#define UnitSettingsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IniFiles.hpp>

//Тип сохранения результата
typedef enum tagResultSavingMode
{
	rsmAtTheEndOfEachBattle = 0,
	rsmPeriodically
} ResultSavingMode;

//Действие по завершению задачи
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

//ООбщие настройки для отдельных игровых режимов
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
	void ShiftCoordinates(float fXCoeff, float fYCoeff)
	{
		STARTScreenControlPoint.x *= fXCoeff;
		STARTScreenControlPoint.y *= fYCoeff;
		REPLAYScreenControlPoint.x *= fXCoeff;
		REPLAYScreenControlPoint.y *= fYCoeff;
	};
} GameModeSpecSettings;

//Возможные действия с диалогами, прерывающими выполнение задачи
typedef enum tagPromptDialogAction
{
	pdaAccept = 0,
	pdaSkip
} PromptDialogAction;

const String g_strDefaultResultsFolder = L"Results";

//Менеджер настроек (чтение-сохранение данных из *.ini)
class TSettingsManager
{
public:
	TSettingsManager();
	~TSettingsManager();

	bool ReadINI(); //Чтение всех настроек из *.ini
	bool UpdateINI(); //Запись всех настроек в *.ini

	__property GameModeSpecSettings CampaignSettings = { read = GetCampaignSettings, write = SetCampaignSettings };
	__property GameModeSpecSettings DungeonsSettings = { read = GetDungeonsSettings, write = SetDungeonsSettings };
	__property GameModeSpecSettings FactionWarsSettings = { read = GetFactionWarsSettings, write = SetFactionWarsSettings };

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
	__property unsigned int ColorTolerance = { read = GetColorTolerance, write = SetColorTolerance };

	__property TPoint EnergyDialogControlPoint = { read = GetEDControlPoint, write = SetEDControlPoint };
	__property TColor EnergyDialogControlPointColor = { read = GetEDControlPointColor, write = SetEDControlPointColor };
	__property PromptDialogAction EnergyDialogPreferredAction = { read = GetEDPreferredAction, write = SetEDPreferredAction };

	__property TPoint SMDialogControlPoint = { read = GetSMDControlPoint, write = SetSMDControlPoint };
	__property TColor SMDialogControlPointColor = { read = GetSMDControlPointColor, write = SetSMDControlPointColor };
	__property PromptDialogAction SMDialogPreferredAction = { read = GetSMDPreferredAction, write = SetSMDPreferredAction };

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
	unsigned int m_uColorTolerance;

	TPoint m_EnergyDialogControlPoint;
	TColor m_EnergyDialogControlPointColor;
	PromptDialogAction m_EnergyDialogAction;
	TPoint m_SMDialogControlPoint;
	TColor m_SMDialogControlPointColor;
	PromptDialogAction m_SMDialogAction;

	unsigned int m_uRecentActivePageIndex;

	GameModeSpecSettings GetCampaignSettings() { return m_CampaignSettings; }
	void SetCampaignSettings(GameModeSpecSettings NewValue) { m_CampaignSettings = NewValue; }
	GameModeSpecSettings GetDungeonsSettings() { return m_DungeonsSettings; }
	void SetDungeonsSettings(GameModeSpecSettings NewValue) { m_DungeonsSettings = NewValue; }
	GameModeSpecSettings GetFactionWarsSettings() { return m_FactionWarsSettings; }
	void SetFactionWarsSettings(GameModeSpecSettings NewValue) { m_FactionWarsSettings = NewValue; }

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
	unsigned int GetColorTolerance() { return m_uColorTolerance; }
	void SetColorTolerance(unsigned int uNewValue) { m_uColorTolerance = uNewValue; }

	TPoint GetEDControlPoint() { return m_EnergyDialogControlPoint; }
	void SetEDControlPoint(TPoint NewValue) { m_EnergyDialogControlPoint = NewValue; }
	TColor GetEDControlPointColor() { return m_EnergyDialogControlPointColor; }
	void SetEDControlPointColor(TColor NewValue) { m_EnergyDialogControlPointColor = NewValue; }
	PromptDialogAction GetEDPreferredAction() { return m_EnergyDialogAction; }
	void SetEDPreferredAction(PromptDialogAction NewValue) { m_EnergyDialogAction = NewValue; }

	TPoint GetSMDControlPoint() { return m_SMDialogControlPoint; }
	void SetSMDControlPoint(TPoint NewValue) { m_SMDialogControlPoint = NewValue; }
	TColor GetSMDControlPointColor() { return m_SMDialogControlPointColor; }
	void SetSMDControlPointColor(TColor NewValue) { m_SMDialogControlPointColor = NewValue; }
	PromptDialogAction GetSMDPreferredAction() { return m_SMDialogAction; }
	void SetSMDPreferredAction(PromptDialogAction NewValue) { m_SMDialogAction = NewValue; }

	unsigned int GetRecentActivePage() { return m_uRecentActivePageIndex; }
	void SetRecentActivePage(unsigned int uNewValue) { m_uRecentActivePageIndex = uNewValue; }
};

//---------------------------------------------------------------------------
#endif
