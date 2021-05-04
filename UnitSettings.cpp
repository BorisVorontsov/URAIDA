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

	std::shared_ptr<TIniFile> pSettings(new TIniFile(strSettingsFile));

	//Кампания
	m_CampaignSettings.GameMode = SupportedGameModes::gmCampaign;
	m_CampaignSettings.bProcessSTARTScreenFirst = pSettings->ReadBool(m_strSectionCampaign, L"ProcessSTARTScreenFirst", true);
	m_CampaignSettings.STARTScreenControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionCampaign, L"STARTScreenControlPointX", 0);
	m_CampaignSettings.STARTScreenControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionCampaign, L"STARTScreenControlPointY", 0);
	m_CampaignSettings.STARTScreenControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionCampaign, L"STARTScreenControlPointColor", clWhite));
	m_CampaignSettings.STARTScreenControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionCampaign, L"STARTScreenControlPointColorTolerance", 1);
	m_CampaignSettings.REPLAYScreenControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenControlPointX", 0);
	m_CampaignSettings.REPLAYScreenControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenControlPointY", 0);
	m_CampaignSettings.REPLAYScreenControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenControlPointColor", clWhite));
	m_CampaignSettings.REPLAYScreenControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenControlPointColorTolerance", 1);
	m_CampaignSettings.REPLAYScreenPreferredAction = static_cast<REPLAYScreenAction>(pSettings->ReadInteger(m_strSectionCampaign, L"REPLAYScreenAction", REPLAYScreenAction::rsaReplay));
	m_CampaignSettings.uDelay = pSettings->ReadInteger(m_strSectionCampaign, L"Delay", 10);
	m_CampaignSettings.nNumberOfBattles = pSettings->ReadInteger(m_strSectionCampaign, L"NumberOfBattles", 1);

	//Подземелья
	m_CampaignSettings.GameMode = SupportedGameModes::gmDungeons;
	m_DungeonsSettings.bProcessSTARTScreenFirst = pSettings->ReadBool(m_strSectionDungeons, L"ProcessSTARTScreenFirst", true);
	m_DungeonsSettings.STARTScreenControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionDungeons, L"STARTScreenControlPointX", 0);
	m_DungeonsSettings.STARTScreenControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionDungeons, L"STARTScreenControlPointY", 0);
	m_DungeonsSettings.STARTScreenControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionDungeons, L"STARTScreenControlPointColor", clWhite));
	m_DungeonsSettings.STARTScreenControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionDungeons, L"STARTScreenControlPointColorTolerance", 1);
	m_DungeonsSettings.REPLAYScreenControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenControlPointX", 0);
	m_DungeonsSettings.REPLAYScreenControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenControlPointY", 0);
	m_DungeonsSettings.REPLAYScreenControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenControlPointColor", clWhite));
	m_DungeonsSettings.REPLAYScreenControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenControlPointColorTolerance", 1);
	m_DungeonsSettings.REPLAYScreenPreferredAction = static_cast<REPLAYScreenAction>(pSettings->ReadInteger(m_strSectionDungeons, L"REPLAYScreenAction", REPLAYScreenAction::rsaReplay));
	m_DungeonsSettings.uDelay = pSettings->ReadInteger(m_strSectionDungeons, L"Delay", 10);
	m_DungeonsSettings.nNumberOfBattles = pSettings->ReadInteger(m_strSectionDungeons, L"NumberOfBattles", 1);

	//Войны фракций
	m_CampaignSettings.GameMode = SupportedGameModes::gmFactionWars;
	m_FactionWarsSettings.bProcessSTARTScreenFirst = pSettings->ReadBool(m_strSectionFactionWars, L"ProcessSTARTScreenFirst", true);
	m_FactionWarsSettings.STARTScreenControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionFactionWars, L"STARTScreenControlPointX", 0);
	m_FactionWarsSettings.STARTScreenControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionFactionWars, L"STARTScreenControlPointY", 0);
	m_FactionWarsSettings.STARTScreenControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionFactionWars, L"STARTScreenControlPointColor", clWhite));
	m_FactionWarsSettings.STARTScreenControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionFactionWars, L"STARTScreenControlPointColorTolerance", 1);
	m_FactionWarsSettings.REPLAYScreenControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointX", 0);
	m_FactionWarsSettings.REPLAYScreenControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointY", 0);
	m_FactionWarsSettings.REPLAYScreenControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointColor", clWhite));
	m_FactionWarsSettings.REPLAYScreenControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointColorTolerance", 1);
	m_FactionWarsSettings.REPLAYScreenPreferredAction = static_cast<REPLAYScreenAction>(pSettings->ReadInteger(m_strSectionFactionWars, L"REPLAYScreenAction", REPLAYScreenAction::rsaReplay));
	m_FactionWarsSettings.uDelay = pSettings->ReadInteger(m_strSectionFactionWars, L"Delay", 10);
	m_FactionWarsSettings.nNumberOfBattles = pSettings->ReadInteger(m_strSectionFactionWars, L"NumberOfBattles", 1);

	//Общие настройки
	m_GameWindowSize.cx = pSettings->ReadInteger(m_strSectionCommon, L"GameWindowWidth", 1280);
	m_GameWindowSize.cy = pSettings->ReadInteger(m_strSectionCommon, L"GameWindowHeight", 720);

	m_bSaveResults = pSettings->ReadBool(m_strSectionCommon, L"SaveResults", true);
	m_ResultSavingMode = static_cast<ResultSavingMode>(pSettings->ReadInteger(m_strSectionCommon, L"ResultSavingMode", ResultSavingMode::rsmAtTheEndOfEachBattle));
	m_uResultSavingPeriod = pSettings->ReadInteger(m_strSectionCommon, L"ResultSavingPeriod", 60);
	m_strPathForResults = pSettings->ReadString(m_strSectionCommon, L"PathForResults", L"");
	m_bClearOldResults = pSettings->ReadBool(m_strSectionCommon, L"ClearOldResults", false);

	m_TaskEndAction = static_cast<TaskEndAction>(pSettings->ReadInteger(m_strSectionCommon, L"TaskEndAction", TaskEndAction::teaDoNothing));
	m_bExitOnTaskEnding = pSettings->ReadBool(m_strSectionCommon, L"ExitOnTaskEnding", false);
	m_bCloseGameOnTaskEnding = pSettings->ReadBool(m_strSectionCommon, L"CloseGameOnTaskEnding", false);

	m_uTriesBeforeForceTaskEnding = pSettings->ReadInteger(m_strSectionCommon, L"TriesBeforeForceTaskEnding", 20);
	m_uScreenCheckingPeriod = pSettings->ReadInteger(m_strSectionCommon, L"ScreenCheckingPeriod", 5);

	m_EnergyDialogControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogControlPointX", 0);
	m_EnergyDialogControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogControlPointY", 0);
	m_EnergyDialogControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogControlPointColor", clWhite));
	m_EnergyDialogControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogControlPointColorTolerance", 1);
	m_EnergyDialogGETButtonPoint.x = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointX", 0);
	m_EnergyDialogGETButtonPoint.y = pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointY", 0);
	m_EnergyDialogAction = static_cast<PromptDialogAction>(pSettings->ReadInteger(m_strSectionCommon, L"EnergyDialogAction", PromptDialogAction::pdaSkip));

	m_SMDialogControlPoint.Coordinates.x = pSettings->ReadInteger(m_strSectionCommon, L"SMDialogControlPointX", 0);
	m_SMDialogControlPoint.Coordinates.y = pSettings->ReadInteger(m_strSectionCommon, L"SMDialogControlPointY", 0);
	m_SMDialogControlPoint.PixelColor = static_cast<TColor>(pSettings->ReadInteger(m_strSectionCommon, L"SMDialogControlPointColor", clWhite));
	m_SMDialogControlPoint.uTolerance = pSettings->ReadInteger(m_strSectionCommon, L"SMDialogControlPointColorTolerance", 1);

	//Внутренние
	m_MainWindowPosition.x = pSettings->ReadInteger(m_strSectionInternal, L"MainWindowLeft", 100);
	m_MainWindowPosition.y = pSettings->ReadInteger(m_strSectionInternal, L"MainWindowTop", 100);
	m_uRecentActivePageIndex = pSettings->ReadInteger(m_strSectionInternal, L"RecentActivePage", 0);
	m_bStayOnTop = pSettings->ReadInteger(m_strSectionInternal, L"StayOnTop", false);

	if (!FileExists(strSettingsFile))
		this->UpdateINI();

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
	pSettings->WriteInteger(m_strSectionCampaign, L"STARTScreenControlPointX", m_CampaignSettings.STARTScreenControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionCampaign, L"STARTScreenControlPointY", m_CampaignSettings.STARTScreenControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionCampaign, L"STARTScreenControlPointColor", m_CampaignSettings.STARTScreenControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionCampaign, L"STARTScreenControlPointColorTolerance", m_CampaignSettings.STARTScreenControlPoint.uTolerance);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenControlPointX", m_CampaignSettings.REPLAYScreenControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenControlPointY", m_CampaignSettings.REPLAYScreenControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenControlPointColor", m_CampaignSettings.REPLAYScreenControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenControlPointColorTolerance", m_CampaignSettings.REPLAYScreenControlPoint.uTolerance);
	pSettings->WriteInteger(m_strSectionCampaign, L"REPLAYScreenAction", m_CampaignSettings.REPLAYScreenPreferredAction);
	pSettings->WriteInteger(m_strSectionCampaign, L"Delay", m_CampaignSettings.uDelay);
	pSettings->WriteInteger(m_strSectionCampaign, L"NumberOfBattles", m_CampaignSettings.nNumberOfBattles);

	//Подземелья
	pSettings->WriteBool(m_strSectionDungeons, L"ProcessSTARTScreenFirst", m_DungeonsSettings.bProcessSTARTScreenFirst);
	pSettings->WriteInteger(m_strSectionDungeons, L"STARTScreenControlPointX", m_DungeonsSettings.STARTScreenControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionDungeons, L"STARTScreenControlPointY", m_DungeonsSettings.STARTScreenControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionDungeons, L"STARTScreenControlPointColor", m_DungeonsSettings.STARTScreenControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionDungeons, L"STARTScreenControlPointColorTolerance", m_DungeonsSettings.STARTScreenControlPoint.uTolerance);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenControlPointX", m_DungeonsSettings.REPLAYScreenControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenControlPointY", m_DungeonsSettings.REPLAYScreenControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenControlPointColor", m_DungeonsSettings.REPLAYScreenControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenControlPointColorTolerance", m_DungeonsSettings.REPLAYScreenControlPoint.uTolerance);
	pSettings->WriteInteger(m_strSectionDungeons, L"REPLAYScreenAction", m_DungeonsSettings.REPLAYScreenPreferredAction);
	pSettings->WriteInteger(m_strSectionDungeons, L"Delay", m_DungeonsSettings.uDelay);
	pSettings->WriteInteger(m_strSectionDungeons, L"NumberOfBattles", m_DungeonsSettings.nNumberOfBattles);

	//Войны фракций
	pSettings->WriteBool(m_strSectionFactionWars, L"ProcessSTARTScreenFirst", m_FactionWarsSettings.bProcessSTARTScreenFirst);
	pSettings->WriteInteger(m_strSectionFactionWars, L"STARTScreenControlPointX", m_FactionWarsSettings.STARTScreenControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionFactionWars, L"STARTScreenControlPointY", m_FactionWarsSettings.STARTScreenControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionFactionWars, L"STARTScreenControlPointColor", m_FactionWarsSettings.STARTScreenControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionFactionWars, L"STARTScreenControlPointColorTolerance", m_FactionWarsSettings.STARTScreenControlPoint.uTolerance);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointX", m_FactionWarsSettings.REPLAYScreenControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointY", m_FactionWarsSettings.REPLAYScreenControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointColor", m_FactionWarsSettings.REPLAYScreenControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenControlPointColorTolerance", m_FactionWarsSettings.REPLAYScreenControlPoint.uTolerance);
	pSettings->WriteInteger(m_strSectionFactionWars, L"REPLAYScreenAction", m_FactionWarsSettings.REPLAYScreenPreferredAction);
	pSettings->WriteInteger(m_strSectionFactionWars, L"Delay", m_FactionWarsSettings.uDelay);
	pSettings->WriteInteger(m_strSectionFactionWars, L"NumberOfBattles", m_FactionWarsSettings.nNumberOfBattles);

	//Общие
	pSettings->WriteInteger(m_strSectionCommon, L"GameWindowWidth", m_GameWindowSize.cx);
	pSettings->WriteInteger(m_strSectionCommon, L"GameWindowHeight", m_GameWindowSize.cy);

	pSettings->WriteBool(m_strSectionCommon, L"SaveResults", m_bSaveResults);
	pSettings->WriteInteger(m_strSectionCommon, L"ResultSavingMode", m_ResultSavingMode);
	pSettings->WriteInteger(m_strSectionCommon, L"ResultSavingPeriod", m_uResultSavingPeriod);
	pSettings->WriteString(m_strSectionCommon, L"PathForResults", m_strPathForResults);
	pSettings->WriteBool(m_strSectionCommon, L"ClearOldResults", m_bClearOldResults);

	pSettings->WriteInteger(m_strSectionCommon, L"TaskEndAction", m_TaskEndAction);
	pSettings->WriteBool(m_strSectionCommon, L"ExitOnTaskEnding", m_bExitOnTaskEnding);
	pSettings->WriteBool(m_strSectionCommon, L"CloseGameOnTaskEnding", m_bCloseGameOnTaskEnding);

	pSettings->WriteInteger(m_strSectionCommon, L"TriesBeforeForceTaskEnding", m_uTriesBeforeForceTaskEnding);
	pSettings->WriteInteger(m_strSectionCommon, L"ScreenCheckingPeriod", m_uScreenCheckingPeriod);

	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogControlPointX", m_EnergyDialogControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogControlPointY", m_EnergyDialogControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogControlPointColor", m_EnergyDialogControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogControlPointColorTolerance", m_EnergyDialogControlPoint.uTolerance);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointX", m_EnergyDialogGETButtonPoint.x);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogGETButtonPointY", m_EnergyDialogGETButtonPoint.y);
	pSettings->WriteInteger(m_strSectionCommon, L"EnergyDialogAction", m_EnergyDialogAction);

	pSettings->WriteInteger(m_strSectionCommon, L"SMDialogControlPointX", m_SMDialogControlPoint.Coordinates.x);
	pSettings->WriteInteger(m_strSectionCommon, L"SMDialogControlPointY", m_SMDialogControlPoint.Coordinates.y);
	pSettings->WriteInteger(m_strSectionCommon, L"SMDialogControlPointColor", m_SMDialogControlPoint.PixelColor);
	pSettings->WriteInteger(m_strSectionCommon, L"SMDialogControlPointColorTolerance", m_SMDialogControlPoint.uTolerance);

	//Внутренние
	pSettings->WriteInteger(m_strSectionInternal, L"MainWindowLeft", m_MainWindowPosition.x);
	pSettings->WriteInteger(m_strSectionInternal, L"MainWindowTop", m_MainWindowPosition.y);
	pSettings->WriteInteger(m_strSectionInternal, L"RecentActivePage", m_uRecentActivePageIndex);
	pSettings->WriteInteger(m_strSectionInternal, L"StayOnTop", m_bStayOnTop);

    return true;
}
//---------------------------------------------------------------------------

