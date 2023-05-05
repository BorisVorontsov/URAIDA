//---------------------------------------------------------------------------
#include "URAIDAPCH.h"
#pragma hdrstop

#include "UnitSettings.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

const String TSettingsManager::m_strSettingsFileName = L"Settings.ini";

const String TSettingsManager::m_strSectionCampaign = L"Campaign";
const String TSettingsManager::m_strSectionDungeons = L"Dungeons";
const String TSettingsManager::m_strSectionFactionWars = L"Faction Wars";
const String TSettingsManager::m_strSectionCommon = L"Common Settings";
const String TSettingsManager::m_strSectionInternal = L"Internal Settings";

//---------------------------------------------------------------------------
TSettingsManager::TSettingsManager()
{
	m_CampaignSettings.GameMode = SupportedGameModes::gmCampaign;
	m_DungeonsSettings.GameMode = SupportedGameModes::gmDungeons;
	m_FactionWarsSettings.GameMode = SupportedGameModes::gmFactionWars;

	for (int i = 0; i < m_EnergyDialogControlPoints.size(); i++)
		m_EnergyDialogControlPoints[i].Name = String(L"EnergyDialogControlPoint") + IntToStr(i + 1);
	for (int i = 0; i < m_SMDialogControlPoints.size(); i++)
		m_SMDialogControlPoints[i].Name = String(L"SMDialogControlPoint") + IntToStr(i + 1);
}
//---------------------------------------------------------------------------
TSettingsManager::~TSettingsManager()
{
	//
}
//---------------------------------------------------------------------------
bool TSettingsManager::ReadINI()
{
	String strAppPath = ExtractFilePath(Application->ExeName);
	String strSettingsFile = strAppPath + m_strSettingsFileName;

	std::unique_ptr<TIniFile> pSettings(new TIniFile(strSettingsFile));

	//Кампания
	m_CampaignSettings.Load(pSettings.get(), m_strSectionCampaign);

	//Подземелья
	m_DungeonsSettings.Load(pSettings.get(), m_strSectionDungeons);

	//Войны фракций
	m_FactionWarsSettings.Load(pSettings.get(), m_strSectionFactionWars);

	//Общие настройки
	m_GameWindowSize.cx = pSettings->ReadInteger(m_strSectionCommon, L"GameWindowWidth", 1280);
	m_GameWindowSize.cy = pSettings->ReadInteger(m_strSectionCommon, L"GameWindowHeight", 720);

	m_bSaveResults = pSettings->ReadBool(m_strSectionCommon, L"SaveResults", true);
	m_ResultSavingMode = static_cast<ResultSavingMode>(pSettings->ReadInteger(m_strSectionCommon, L"ResultSavingMode", ResultSavingMode::rsmAtTheEndOfEachBattle));
	m_uResultSavingPeriod = pSettings->ReadInteger(m_strSectionCommon, L"ResultSavingPeriod", 60);
	m_strPathForResults = pSettings->ReadString(m_strSectionCommon, L"PathForResults", L"");
	m_bDeletePreviousResults = pSettings->ReadBool(m_strSectionCommon, L"DeletePreviousResults", false);

	m_TaskEndAction = static_cast<TaskEndAction>(pSettings->ReadInteger(m_strSectionCommon, L"TaskEndAction", TaskEndAction::teaDoNothing));
	m_strTEAUserDefinedCommand = pSettings->ReadString(m_strSectionCommon, L"TEAUserDefinedCommand", L"");
	m_bExitOnTaskEnding = pSettings->ReadBool(m_strSectionCommon, L"ExitOnTaskEnding", false);
	m_bCloseGameOnTaskEnding = pSettings->ReadBool(m_strSectionCommon, L"CloseGameOnTaskEnding", false);

	m_uTriesBeforeForceTaskEnding = pSettings->ReadInteger(m_strSectionCommon, L"TriesBeforeForceTaskEnding", 20);
	m_uScreenCheckingPeriod = pSettings->ReadInteger(m_strSectionCommon, L"ScreenCheckingPeriod", 5);

	for (auto& ControlPoint : m_EnergyDialogControlPoints)
		ControlPoint.Load(pSettings.get(), m_strSectionCommon);
	m_uEnergyDialogControlPointIndex = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogControlPointIndex", 0);
	m_EnergyDialogGETButtonPoint.x = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointX", 0);
	m_EnergyDialogGETButtonPoint.y = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointY", 0);
	m_EnergyDialogAction = static_cast<PromptDialogAction>(pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogAction", PromptDialogAction::pdaSkip));

	for (auto& ControlPoint : m_SMDialogControlPoints)
		ControlPoint.Load(pSettings.get(), m_strSectionCommon);
	m_uSMDialogControlPointIndex = pSettings->ReadInteger(m_strSectionCommon, L"SMDialogControlPointIndex", 0);

	//Внутренние
	m_MainWindowPosition.Left = pSettings->ReadInteger(m_strSectionInternal, L"MainWindowLeft", 100);
	m_MainWindowPosition.Top = pSettings->ReadInteger(m_strSectionInternal, L"MainWindowTop", 100);
	m_MainWindowPosition.Bottom = pSettings->ReadInteger(m_strSectionInternal, L"MainWindowHeight", 720);
	m_uRecentActivePageIndex = pSettings->ReadInteger(m_strSectionInternal, L"RecentActivePage", 0);
	m_bStayOnTop = pSettings->ReadBool(m_strSectionInternal, L"StayOnTop", false);
	m_bEnableLogging = pSettings->ReadBool(m_strSectionInternal, L"EnableLogging", false);
	m_uMaxLogEntries = pSettings->ReadInteger(m_strSectionInternal, L"MaxLogEntries", 1000);
	m_strPathToPlariumPlay = pSettings->ReadString(m_strSectionInternal, L"PathToPlariumPlay", L"");

	if (!FileExists(strSettingsFile))
		this->UpdateINI();

	return true;
}
//---------------------------------------------------------------------------
bool TSettingsManager::UpdateINI()
{
	String strAppPath = ExtractFilePath(Application->ExeName);
	String strSettingsFile = strAppPath + m_strSettingsFileName;

	std::unique_ptr<TIniFile> pSettings(new TIniFile(strSettingsFile));

	//Кампания
	m_CampaignSettings.Serialize(pSettings.get(), m_strSectionCampaign);

	//Подземелья
	m_DungeonsSettings.Serialize(pSettings.get(), m_strSectionDungeons);

	//Войны фракций
	m_FactionWarsSettings.Serialize(pSettings.get(), m_strSectionFactionWars);

	//Общие
	pSettings->WriteInteger(m_strSectionCommon, L"GameWindowWidth", m_GameWindowSize.cx);
	pSettings->WriteInteger(m_strSectionCommon, L"GameWindowHeight", m_GameWindowSize.cy);

	pSettings->WriteBool(m_strSectionCommon, L"SaveResults", m_bSaveResults);
	pSettings->WriteInteger(m_strSectionCommon, L"ResultSavingMode", m_ResultSavingMode);
	pSettings->WriteInteger(m_strSectionCommon, L"ResultSavingPeriod", m_uResultSavingPeriod);
	pSettings->WriteString(m_strSectionCommon, L"PathForResults", m_strPathForResults);
	pSettings->WriteBool(m_strSectionCommon, L"DeletePreviousResults", m_bDeletePreviousResults);

	pSettings->WriteInteger(m_strSectionCommon, L"TaskEndAction", m_TaskEndAction);
	pSettings->WriteString(m_strSectionCommon, L"TEAUserDefinedCommand", m_strTEAUserDefinedCommand);
	pSettings->WriteBool(m_strSectionCommon, L"ExitOnTaskEnding", m_bExitOnTaskEnding);
	pSettings->WriteBool(m_strSectionCommon, L"CloseGameOnTaskEnding", m_bCloseGameOnTaskEnding);

	pSettings->WriteInteger(m_strSectionCommon, L"TriesBeforeForceTaskEnding", m_uTriesBeforeForceTaskEnding);
	pSettings->WriteInteger(m_strSectionCommon, L"ScreenCheckingPeriod", m_uScreenCheckingPeriod);

	for (auto& ControlPoint : m_EnergyDialogControlPoints)
		ControlPoint.Serialize(pSettings.get(), m_strSectionCommon);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogControlPointIndex", m_uEnergyDialogControlPointIndex);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointX", m_EnergyDialogGETButtonPoint.x);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointY", m_EnergyDialogGETButtonPoint.y);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogAction", m_EnergyDialogAction);

	for (auto& ControlPoint : m_SMDialogControlPoints)
		ControlPoint.Serialize(pSettings.get(), m_strSectionCommon);
	pSettings->WriteInteger(m_strSectionCommon, L"SMDialogControlPointIndex", m_uSMDialogControlPointIndex);

	//Внутренние
	pSettings->WriteInteger(m_strSectionInternal, L"MainWindowLeft", m_MainWindowPosition.Left);
	pSettings->WriteInteger(m_strSectionInternal, L"MainWindowTop", m_MainWindowPosition.Top);
	pSettings->WriteInteger(m_strSectionInternal, L"MainWindowHeight", m_MainWindowPosition.Bottom);
	pSettings->WriteInteger(m_strSectionInternal, L"RecentActivePage", m_uRecentActivePageIndex);
	pSettings->WriteBool(m_strSectionInternal, L"StayOnTop", m_bStayOnTop);
	pSettings->WriteBool(m_strSectionInternal, L"EnableLogging", m_bEnableLogging);
	pSettings->WriteInteger(m_strSectionInternal, L"MaxLogEntries", m_uMaxLogEntries);
	pSettings->WriteString(m_strSectionInternal, L"PathToPlariumPlay", m_strPathToPlariumPlay);

	return true;
}
//---------------------------------------------------------------------------

