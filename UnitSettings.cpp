//---------------------------------------------------------------------------
#include "URAIDAPCH.h"
#include <memory>
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
	//TODO
}
//---------------------------------------------------------------------------
TSettingsManager::~TSettingsManager()
{
	//TODO
}
//---------------------------------------------------------------------------
bool TSettingsManager::ReadINI()
{
	String strAppPath = ExtractFilePath(Application->ExeName);
	String strSettingsFile = strAppPath + m_strSettingsFileName;

	if (!FileExists(strSettingsFile))
		this->UpdateINI();

	std::shared_ptr<TIniFile> pSettings(new TIniFile(strSettingsFile));

	//Кампания
	m_CampaignSettings.GameMode = SupportedGameModes::gmCampaign;
	m_CampaignSettings.bProcessSTARTScreenFirst = pSettings->ReadBool(m_strSectionCampaign, L"ProcessSTARTScreenFirst", true);
	m_CampaignSettings.STARTScreenControlPoint.x = pSettings->ReadInteger(m_strSectionCampaign, L"STARTScreenControlPointX", 0);
	m_CampaignSettings.STARTScreenControlPoint.y = pSettings->ReadInteger(m_strSectionCampaign, L"STARTScreenControlPointY", 0);
	m_CampaignSettings.STARTScreenControlPointColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionCampaign, L"STARTScreenControlPointColor", clWhite));
	m_CampaignSettings.REPLAYScreenControlPoint.x = pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenControlPointX", 0);
	m_CampaignSettings.REPLAYScreenControlPoint.y = pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenControlPointY", 0);
	m_CampaignSettings.REPLAYScreenControlPointColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenControlPointColor", clWhite));
	m_CampaignSettings.uDelay = pSettings->ReadInteger(m_strSectionCampaign, L"Delay", 10);
	m_CampaignSettings.nNumberOfBattles = pSettings->ReadInteger(m_strSectionCampaign, L"NumberOfBattles", 1);

	//Подземелья
	m_CampaignSettings.GameMode = SupportedGameModes::gmDungeons;
	m_DungeonsSettings.bProcessSTARTScreenFirst = pSettings->ReadBool(m_strSectionDungeons, L"ProcessSTARTScreenFirst", true);
	m_DungeonsSettings.STARTScreenControlPoint.x = pSettings->ReadInteger(m_strSectionDungeons, L"STARTScreenControlPointX", 0);
	m_DungeonsSettings.STARTScreenControlPoint.y = pSettings->ReadInteger(m_strSectionDungeons, L"STARTScreenControlPointY", 0);
	m_DungeonsSettings.STARTScreenControlPointColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionDungeons, L"STARTScreenControlPointColor", clWhite));
	m_DungeonsSettings.REPLAYScreenControlPoint.x = pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenControlPointX", 0);
	m_DungeonsSettings.REPLAYScreenControlPoint.y = pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenControlPointY", 0);
	m_DungeonsSettings.REPLAYScreenControlPointColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenControlPointColor", clWhite));
	m_DungeonsSettings.uDelay = pSettings->ReadInteger(m_strSectionDungeons, L"Delay", 10);
	m_DungeonsSettings.nNumberOfBattles = pSettings->ReadInteger(m_strSectionDungeons, L"NumberOfBattles", 1);

	//Войны фракций
	m_CampaignSettings.GameMode = SupportedGameModes::gmFactionWars;
	m_FactionWarsSettings.bProcessSTARTScreenFirst = pSettings->ReadBool(m_strSectionFactionWars, L"ProcessSTARTScreenFirst", true);
	m_FactionWarsSettings.STARTScreenControlPoint.x = pSettings->ReadInteger(m_strSectionFactionWars, L"STARTScreenControlPointX", 0);
	m_FactionWarsSettings.STARTScreenControlPoint.y = pSettings->ReadInteger(m_strSectionFactionWars, L"STARTScreenControlPointY", 0);
	m_FactionWarsSettings.STARTScreenControlPointColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionFactionWars, L"STARTScreenControlPointColor", clWhite));
	m_FactionWarsSettings.REPLAYScreenControlPoint.x = pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointX", 0);
	m_FactionWarsSettings.REPLAYScreenControlPoint.y = pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointY", 0);
	m_FactionWarsSettings.REPLAYScreenControlPointColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointColor", clWhite));
	m_FactionWarsSettings.uDelay = pSettings->ReadInteger(m_strSectionFactionWars, L"Delay", 10);
	m_FactionWarsSettings.nNumberOfBattles = pSettings->ReadInteger(m_strSectionFactionWars, L"NumberOfBattles", 1);

    //Общие настройки
	m_bAutoResizeWindow = pSettings->ReadBool(m_strSectionCommon, L"AutoResizeWindow", true);
	m_GameWindowSize.cx = pSettings->ReadInteger(m_strSectionCommon, L"GameWindowWidth", 1280);
	m_GameWindowSize.cy = pSettings->ReadInteger(m_strSectionCommon, L"GameWindowHeight", 720);

	m_bSaveResults = pSettings->ReadBool(m_strSectionCommon, L"SaveResults", true);
	m_ResultSavingMode = static_cast<ResultSavingMode>(pSettings->ReadInteger(m_strSectionCommon, L"ResultSavingMode", ResultSavingMode::rsmAtTheEndOfEachBattle));
	m_uResultSavingPeriod = pSettings->ReadInteger(m_strSectionCommon, L"ResultSavingPeriod", 60);
	m_strPathForResults = pSettings->ReadString(m_strSectionCommon, L"PathForResults", L"");

	m_TaskEndAction = static_cast<TaskEndAction>(pSettings->ReadInteger(m_strSectionCommon, L"TaskEndAction", TaskEndAction::teaShowNotification));
	m_bExitOnTaskEnding = pSettings->ReadBool(m_strSectionCommon, L"ExitOnTaskEnding", false);
	m_bCloseGameOnTaskEnding = pSettings->ReadBool(m_strSectionCommon, L"CloseGameOnTaskEnding", false);

	m_uTriesBeforeForceTaskEnding = pSettings->ReadInteger(m_strSectionCommon, L"TriesBeforeForceTaskEnding", 20);
	m_uScreenCheckingPeriod = pSettings->ReadInteger(m_strSectionCommon, L"ScreenCheckingPeriod", 5);

	m_uRecentActivePageIndex = pSettings->ReadInteger(m_strSectionInternal, L"RecentActivePage", 0);

	return true;
}
//---------------------------------------------------------------------------
bool TSettingsManager::UpdateINI()
{
	String strAppPath = ExtractFilePath(Application->ExeName);
	String strSettingsFile = strAppPath + m_strSettingsFileName;

	std::shared_ptr<TIniFile> pSettings(new TIniFile(strSettingsFile));

	//Кампания
	pSettings->WriteBool(m_strSectionCampaign, L"ProcessSTARTScreenFirst", m_CampaignSettings.bProcessSTARTScreenFirst);
	pSettings->WriteInteger(m_strSectionCampaign, L"STARTScreenControlPointX", m_CampaignSettings.STARTScreenControlPoint.x);
	pSettings->WriteInteger(m_strSectionCampaign, L"STARTScreenControlPointY", m_CampaignSettings.STARTScreenControlPoint.y);
	pSettings->WriteInteger(m_strSectionCampaign, L"STARTScreenControlPointColor", m_CampaignSettings.STARTScreenControlPointColor);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenControlPointX", m_CampaignSettings.REPLAYScreenControlPoint.x);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenControlPointY", m_CampaignSettings.REPLAYScreenControlPoint.y);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenControlPointColor", m_CampaignSettings.REPLAYScreenControlPointColor);
	pSettings->WriteInteger(m_strSectionCampaign, L"Delay", m_CampaignSettings.uDelay);
	pSettings->WriteInteger(m_strSectionCampaign, L"NumberOfBattles", m_CampaignSettings.nNumberOfBattles);

	//Подземелья
	pSettings->WriteBool(m_strSectionDungeons, L"ProcessSTARTScreenFirst", m_DungeonsSettings.bProcessSTARTScreenFirst);
	pSettings->WriteInteger(m_strSectionDungeons, L"STARTScreenControlPointX", m_DungeonsSettings.STARTScreenControlPoint.x);
	pSettings->WriteInteger(m_strSectionDungeons, L"STARTScreenControlPointY", m_DungeonsSettings.STARTScreenControlPoint.y);
	pSettings->WriteInteger(m_strSectionDungeons, L"STARTScreenControlPointColor", m_DungeonsSettings.STARTScreenControlPointColor);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenControlPointX", m_DungeonsSettings.REPLAYScreenControlPoint.x);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenControlPointY", m_DungeonsSettings.REPLAYScreenControlPoint.y);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenControlPointColor", m_DungeonsSettings.REPLAYScreenControlPointColor);
	pSettings->WriteInteger(m_strSectionDungeons, L"Delay", m_DungeonsSettings.uDelay);
	pSettings->WriteInteger(m_strSectionDungeons, L"NumberOfBattles", m_DungeonsSettings.nNumberOfBattles);

	//Войны фракций
	pSettings->WriteBool(m_strSectionFactionWars, L"ProcessSTARTScreenFirst", m_FactionWarsSettings.bProcessSTARTScreenFirst);
	pSettings->WriteInteger(m_strSectionFactionWars, L"STARTScreenControlPointX", m_FactionWarsSettings.STARTScreenControlPoint.x);
	pSettings->WriteInteger(m_strSectionFactionWars, L"STARTScreenControlPointY", m_FactionWarsSettings.STARTScreenControlPoint.y);
	pSettings->WriteInteger(m_strSectionFactionWars, L"STARTScreenControlPointColor", m_FactionWarsSettings.STARTScreenControlPointColor);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointX", m_FactionWarsSettings.REPLAYScreenControlPoint.x);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointY", m_FactionWarsSettings.REPLAYScreenControlPoint.y);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointColor", m_FactionWarsSettings.REPLAYScreenControlPointColor);
	pSettings->WriteInteger(m_strSectionFactionWars, L"Delay", m_FactionWarsSettings.uDelay);
	pSettings->WriteInteger(m_strSectionFactionWars, L"NumberOfBattles", m_FactionWarsSettings.nNumberOfBattles);

    //Общие
	pSettings->WriteBool(m_strSectionCommon, L"AutoResizeWindow", m_bAutoResizeWindow);
	pSettings->WriteInteger(m_strSectionCommon, L"GameWindowWidth", m_GameWindowSize.cx);
	pSettings->WriteInteger(m_strSectionCommon, L"GameWindowHeight", m_GameWindowSize.cy);

	pSettings->WriteBool(m_strSectionCommon, L"SaveResults", m_bSaveResults);
	pSettings->WriteInteger(m_strSectionCommon, L"ResultSavingMode", m_ResultSavingMode);
	pSettings->WriteInteger(m_strSectionCommon, L"ResultSavingPeriod", m_uResultSavingPeriod);
	pSettings->WriteString(m_strSectionCommon, L"PathForResults", m_strPathForResults);

	pSettings->WriteInteger(m_strSectionCommon, L"TaskEndAction", m_TaskEndAction);
	pSettings->WriteBool(m_strSectionCommon, L"ExitOnTaskEnding", m_bExitOnTaskEnding);
	pSettings->WriteBool(m_strSectionCommon, L"CloseGameOnTaskEnding", m_bCloseGameOnTaskEnding);

	pSettings->WriteInteger(m_strSectionCommon, L"TriesBeforeForceTaskEnding", m_uTriesBeforeForceTaskEnding);
	pSettings->WriteInteger(m_strSectionCommon, L"ScreenCheckingPeriod", m_uScreenCheckingPeriod);

	pSettings->WriteInteger(m_strSectionInternal, L"RecentActivePage", m_uRecentActivePageIndex);

    return true;
}
//---------------------------------------------------------------------------

