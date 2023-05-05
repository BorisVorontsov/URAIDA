//---------------------------------------------------------------------------

#ifndef UnitSettingsH
#define UnitSettingsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.IniFiles.hpp>
#include <memory>
#include <array>

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
	teaTurnOffPC,
	teaUserDefinedCommand
} TaskEndAction;

typedef enum tagSupportedGameModes
{
	gmCampaign = 1,
	gmDungeons,
	gmFactionWars
} SupportedGameModes;

//Интерфейс сериализации для классов настроек
class IIniSerialization
{
public:
	virtual void Load(TIniFile* pIniFile, String strSection) = 0;
	virtual void Load(String strIniFile, String strSection) = 0;
	virtual void Serialize(TIniFile* pIniFile, String strSection) = 0;
	virtual void Serialize(String strIniFile, String strSection) = 0;
};

const unsigned int g_uMaxControlPoints = 3; //Максимальное количество контрольных точек для задания

//Описатель контрольной точки
class TControlPoint : public IIniSerialization
{
public:
	TControlPoint()
	{
		m_Coordinates.SetLocation(0, 0);
		m_PixelColor = clBlack;
		m_uTolerance = 0;
		m_strControlPointName = L"";
	}
	~TControlPoint() { /* */ }

	void ShiftCoordinates(float fXCoeff, float fYCoeff)
	{
		m_Coordinates.x *= fXCoeff;
		m_Coordinates.y *= fYCoeff;
	}

	void Load(TIniFile* pIniFile, String strSection) override
	{
		m_Coordinates.x = pIniFile->ReadInteger(strSection, m_strControlPointName + L"X", 0);
		m_Coordinates.y = pIniFile->ReadInteger(strSection, m_strControlPointName + L"Y", 0);
		m_PixelColor = static_cast<TColor>(pIniFile->ReadInteger(strSection, m_strControlPointName + L"Color", clWhite));
		m_uTolerance = pIniFile->ReadInteger(strSection, m_strControlPointName + L"Tolerance", 1);
		m_bEnabled = pIniFile->ReadBool(strSection, m_strControlPointName + L"Enabled", false);
	}
	void Load(String strIniFile, String strSection) override
	{
		std::unique_ptr<TIniFile> pOutput(new TIniFile(strIniFile));
		this->Load(pOutput.get(), strSection);
	}

	void Serialize(TIniFile* pIniFile, String strSection) override
	{
		pIniFile->WriteInteger(strSection, m_strControlPointName + L"X", m_Coordinates.x);
		pIniFile->WriteInteger(strSection, m_strControlPointName + L"Y", m_Coordinates.y);
		pIniFile->WriteInteger(strSection, m_strControlPointName + L"Color", m_PixelColor);
		pIniFile->WriteInteger(strSection, m_strControlPointName + L"Tolerance", m_uTolerance);
		pIniFile->WriteBool(strSection, m_strControlPointName + L"Enabled", m_bEnabled);
	}
	void Serialize(String strIniFile, String strSection) override
	{
		std::unique_ptr<TIniFile> pOutput(new TIniFile(strIniFile));
		this->Serialize(pOutput.get(), strSection);
	}

	//Свойства
	__property TPoint Coordinates = { read = m_Coordinates, write = m_Coordinates };
	__property TColor PixelColor = { read = m_PixelColor, write = m_PixelColor };
	__property unsigned int Tolerance = { read = m_uTolerance, write = m_uTolerance };
	__property bool Enabled = { read = m_bEnabled, write = m_bEnabled };
	__property String Name = { read = m_strControlPointName, write = m_strControlPointName };

private:
	TPoint m_Coordinates;
	TColor m_PixelColor;
	unsigned int m_uTolerance;
	bool m_bEnabled;
	String m_strControlPointName;
};

//Действия на экране ПОВТОР/ДАЛЕЕ
typedef enum tagREPLAYScreenAction
{
	rsaReplay = 0,
	rsaGoNext
} REPLAYScreenAction;

//Возможные действия с диалогами, прерывающими выполнение задачи
typedef enum tagBattleInitiationMethod
{
	bimByHotkey = 0,
	bimByMouseClick
} BattleInitiationMethod;

//ООбщие настройки для отдельных игровых режимов
class TGameModeSpecSettings : public IIniSerialization
{
public:
	TGameModeSpecSettings()
	{
		for (int i = 0; i < m_STARTScreenControlPoints.size(); i++)
			m_STARTScreenControlPoints[i].Name = String(L"STARTScreenControlPoint") + IntToStr(i + 1);
		for (int i = 0; i < m_REPLAYScreenControlPoints.size(); i++)
			m_REPLAYScreenControlPoints[i].Name = String(L"REPLAYScreenControlPoint") + IntToStr(i + 1);
	}
	~TGameModeSpecSettings() { /* */ }

	void Load(TIniFile* pIniFile, String strSection) override
	{
		m_bProcessSTARTScreenFirst = pIniFile->ReadBool(strSection, L"ProcessSTARTScreenFirst", true);
		for (auto& ControlPoint : m_STARTScreenControlPoints)
			ControlPoint.Load(pIniFile, strSection);
		m_uSTARTScreenControlPointIndex = pIniFile->ReadInteger(strSection, L"STARTScreenControlPointIndex", 0);
		for (auto& ControlPoint : m_REPLAYScreenControlPoints)
			ControlPoint.Load(pIniFile, strSection);
		m_uREPLAYScreenControlPointIndex = pIniFile->ReadInteger(strSection, L"REPLAYScreenControlPointIndex", 0);
		m_REPLAYScreenPreferredAction = static_cast<REPLAYScreenAction>(pIniFile->ReadInteger(strSection, L"REPLAYScreenAction", REPLAYScreenAction::rsaReplay));
		m_uDelay = pIniFile->ReadInteger(strSection, L"Delay", 10);
		m_nNumberOfBattles = pIniFile->ReadInteger(strSection, L"NumberOfBattles", 1);
		m_bEndlessMode = pIniFile->ReadBool(strSection, L"EndlessMode", false);
		m_BattleInitiationMethod = static_cast<BattleInitiationMethod>(pIniFile->ReadInteger(strSection, L"BattleInitiationMethod", BattleInitiationMethod::bimByHotkey));
		m_BIWhereToClickPoint.x = pIniFile->ReadInteger(strSection, L"BattleInitiationWhereToClickPointX", 0);
		m_BIWhereToClickPoint.y = pIniFile->ReadInteger(strSection, L"BattleInitiationWhereToClickPointY", 0);
	}
	void Load(String strIniFile, String strSection) override
	{
		std::unique_ptr<TIniFile> pOutput(new TIniFile(strIniFile));
		this->Load(pOutput.get(), strSection);
	}

	void Serialize(TIniFile* pIniFile, String strSection) override
	{
		pIniFile->WriteBool(strSection, L"ProcessSTARTScreenFirst", m_bProcessSTARTScreenFirst);
		for (auto& ControlPoint : m_STARTScreenControlPoints)
			ControlPoint.Serialize(pIniFile, strSection);
		//m_STARTScreenControlPoints[m_uSTARTScreenControlPointIndex].Serialize(pIniFile, strSection);
		pIniFile->WriteInteger(strSection, L"STARTScreenControlPointIndex", m_uSTARTScreenControlPointIndex);
		for (auto& ControlPoint : m_REPLAYScreenControlPoints)
			ControlPoint.Serialize(pIniFile, strSection);
		//m_REPLAYScreenControlPoints[m_uREPLAYScreenControlPointIndex].Serialize(pIniFile, strSection);
		pIniFile->WriteInteger(strSection, L"REPLAYScreenControlPointIndex", m_uREPLAYScreenControlPointIndex);
		pIniFile->WriteInteger(strSection, L"REPLAYScreenAction", m_REPLAYScreenPreferredAction);
		pIniFile->WriteInteger(strSection, L"Delay", m_uDelay);
		pIniFile->WriteInteger(strSection, L"NumberOfBattles", m_nNumberOfBattles);
		pIniFile->WriteBool(strSection, L"EndlessMode", m_bEndlessMode);
		pIniFile->WriteInteger(strSection, L"BattleInitiationMethod", m_BattleInitiationMethod);
		pIniFile->WriteInteger(strSection, L"BattleInitiationWhereToClickPointX", m_BIWhereToClickPoint.x);
		pIniFile->WriteInteger(strSection, L"BattleInitiationWhereToClickPointY", m_BIWhereToClickPoint.y);
	}
	void Serialize(String strIniFile, String strSection) override
	{
		std::unique_ptr<TIniFile> pOutput(new TIniFile(strIniFile));
		this->Serialize(pOutput.get(), strSection);
	}

	//Свойства
	__property SupportedGameModes GameMode = { read = m_GameMode, write = m_GameMode };
	__property bool ProcessSTARTScreenFirst = { read = m_bProcessSTARTScreenFirst, write = m_bProcessSTARTScreenFirst };
	__property std::array<TControlPoint, g_uMaxControlPoints>& STARTScreenControlPoints = { read = m_STARTScreenControlPoints, write = m_STARTScreenControlPoints };
	__property unsigned int STARTScreenControlPointIndex = { read = m_uSTARTScreenControlPointIndex, write = m_uSTARTScreenControlPointIndex };
	__property std::array<TControlPoint, g_uMaxControlPoints>& REPLAYScreenControlPoints = { read = m_REPLAYScreenControlPoints, write = m_REPLAYScreenControlPoints };
	__property unsigned int REPLAYScreenControlPointIndex = { read = m_uREPLAYScreenControlPointIndex, write = m_uREPLAYScreenControlPointIndex };
	__property REPLAYScreenAction REPLAYScreenPreferredAction = { read = m_REPLAYScreenPreferredAction, write = m_REPLAYScreenPreferredAction };
	__property unsigned int Delay = { read = m_uDelay, write = m_uDelay };
	__property unsigned int NumberOfBattles = { read = m_nNumberOfBattles, write = m_nNumberOfBattles };
	__property bool EndlessMode = { read = m_bEndlessMode, write = m_bEndlessMode };
	__property BattleInitiationMethod BattleInitiationPreferredMethod = { read = m_BattleInitiationMethod, write = m_BattleInitiationMethod };
	__property TPoint BattleInitiationWhereToClickPoint = { read = m_BIWhereToClickPoint, write = m_BIWhereToClickPoint };

private:
	SupportedGameModes m_GameMode;
	bool m_bProcessSTARTScreenFirst;
	std::array<TControlPoint, g_uMaxControlPoints> m_STARTScreenControlPoints;
	unsigned int m_uSTARTScreenControlPointIndex;
	std::array<TControlPoint, g_uMaxControlPoints> m_REPLAYScreenControlPoints;
	unsigned int m_uREPLAYScreenControlPointIndex;
	REPLAYScreenAction m_REPLAYScreenPreferredAction;
	unsigned int m_uDelay;
	unsigned int m_nNumberOfBattles;
	bool m_bEndlessMode;
	BattleInitiationMethod m_BattleInitiationMethod;
	TPoint m_BIWhereToClickPoint;
};

//Возможные действия с диалогами, прерывающими выполнение задачи
typedef enum tagPromptDialogAction
{
	pdaAccept = 0,
	pdaSkip,
	pdaAbort
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

    //Все настройки
	__property TGameModeSpecSettings& CampaignSettings = { read = m_CampaignSettings, write = m_CampaignSettings };
	__property TGameModeSpecSettings& DungeonsSettings = { read = m_DungeonsSettings, write = m_DungeonsSettings };
	__property TGameModeSpecSettings& FactionWarsSettings = { read = m_FactionWarsSettings, write = m_FactionWarsSettings };

	__property TSize RAIDWindowSize = { read = m_GameWindowSize, write = m_GameWindowSize };

	__property bool SaveResults = { read = m_bSaveResults, write = m_bSaveResults };
	__property ResultSavingMode ResultSavingMethod = { read = m_ResultSavingMode, write = m_ResultSavingMode };
	__property unsigned int ResultSavingPeriod = { read = m_uResultSavingPeriod, write = m_uResultSavingPeriod };
	__property String PathForResults = { read = m_strPathForResults, write = m_strPathForResults };
	__property bool DeletePreviousResults = { read = m_bDeletePreviousResults, write = m_bDeletePreviousResults };

	__property TaskEndAction TaskEndBehavior = { read = m_TaskEndAction, write = m_TaskEndAction };
	__property String TEBUserDefinedCommand = { read = m_strTEAUserDefinedCommand, write = m_strTEAUserDefinedCommand };
	__property bool ExitOnTaskEnding = { read = m_bExitOnTaskEnding, write = m_bExitOnTaskEnding };
	__property bool CloseGameOnTaskEnding = { read = m_bCloseGameOnTaskEnding, write = m_bCloseGameOnTaskEnding };

	__property unsigned int TriesBeforeForceTaskEnding = { read = m_uTriesBeforeForceTaskEnding, write = m_uTriesBeforeForceTaskEnding };
	__property unsigned int ScreenCheckingPeriod = { read = m_uScreenCheckingPeriod, write = m_uScreenCheckingPeriod };

	__property std::array<TControlPoint, g_uMaxControlPoints> EnergyDialogControlPoints = { read = m_EnergyDialogControlPoints, write = m_EnergyDialogControlPoints };
	__property unsigned int EnergyDialogControlPointIndex = { read = m_uEnergyDialogControlPointIndex, write = m_uEnergyDialogControlPointIndex };
	__property TPoint EnergyDialogGETButtonPoint = { read = m_EnergyDialogGETButtonPoint, write = m_EnergyDialogGETButtonPoint };
	__property PromptDialogAction EnergyDialogPreferredAction = { read = m_EnergyDialogAction, write = m_EnergyDialogAction };

	__property std::array<TControlPoint, g_uMaxControlPoints> SMDialogControlPoints = { read = m_SMDialogControlPoints, write = m_SMDialogControlPoints };
	__property unsigned int SMDialogControlPointIndex = { read = m_uSMDialogControlPointIndex, write = m_uSMDialogControlPointIndex };

	__property TPoint MainWindowPosition = { read = m_MainWindowPosition, write = m_MainWindowPosition };
	__property unsigned int RecentActivePage = { read = m_uRecentActivePageIndex, write = m_uRecentActivePageIndex };
	__property bool StayOnTop = { read = m_bStayOnTop, write = m_bStayOnTop };

	__property bool EnableLogging = { read = m_bEnableLogging, write = m_bEnableLogging };
	__property unsigned int MaxLogEntries = { read = m_uMaxLogEntries, write = m_uMaxLogEntries };

	__property String PathToPlariumPlay = { read = m_strPathToPlariumPlay, write = m_strPathToPlariumPlay };

private:
	static const String m_strSettingsFileName;

	static const String m_strSectionCampaign;
	static const String m_strSectionDungeons;
	static const String m_strSectionFactionWars;
	static const String m_strSectionCommon;
	static const String m_strSectionInternal;

	TGameModeSpecSettings m_CampaignSettings;
	TGameModeSpecSettings m_DungeonsSettings;
	TGameModeSpecSettings m_FactionWarsSettings;

	TSize m_GameWindowSize;

	bool m_bSaveResults;
	ResultSavingMode m_ResultSavingMode;
	unsigned int m_uResultSavingPeriod;
	String m_strPathForResults;
	bool m_bDeletePreviousResults;

	TaskEndAction m_TaskEndAction;
	String m_strTEAUserDefinedCommand;
	bool m_bExitOnTaskEnding;
	bool m_bCloseGameOnTaskEnding;

	unsigned int m_uTriesBeforeForceTaskEnding;
	unsigned int m_uScreenCheckingPeriod;

	std::array<TControlPoint, g_uMaxControlPoints> m_EnergyDialogControlPoints;
	unsigned int m_uEnergyDialogControlPointIndex;
	TPoint m_EnergyDialogGETButtonPoint;
	PromptDialogAction m_EnergyDialogAction;
	std::array<TControlPoint, g_uMaxControlPoints> m_SMDialogControlPoints;
	unsigned int m_uSMDialogControlPointIndex;

	TPoint m_MainWindowPosition;
	unsigned int m_uRecentActivePageIndex;
	bool m_bStayOnTop;

    bool m_bEnableLogging;
	unsigned int m_uMaxLogEntries;

	String m_strPathToPlariumPlay;

};

//---------------------------------------------------------------------------
#endif
