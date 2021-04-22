//---------------------------------------------------------------------------

#include "URAIDAPCH.h"
#include <Mmsystem.h>
#include <powrprof.h>
#include <Jpeg.hpp>
#include <IOUtils.hpp>
#include <memory>
#pragma hdrstop

#include "UnitFormMain.h"
#include "UnitFormPickColor.h"
#include "UnitCommon.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMain *FormMain;

TSettingsManager *g_pSettingsManager;
TRAIDWorker *g_pRAIDWorker;

const String TFormMain::m_strButtonRTRunCaption = L"Run";
const String TFormMain::m_strButtonRTPauseCaption = L"Pause";
const String TFormMain::m_strButtonRTResumeCaption = L"Resume";

//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner)
{
	AdjustPrivilege(SE_SHUTDOWN_NAME);

	m_ActiveTaskInfo.CurrentState = TaskState::tsStopped;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::ButtonRunTaskClick(TObject *Sender)
{
	m_bForceStopTask = false;
	this->StartTask();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::ButtonStopTaskClick(TObject *Sender)
{
	m_bForceStopTask = true;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::TimerMainTimer(TObject *Sender)
{
	//А вдруг?
	if (!g_pRAIDWorker->IsGameRunning())
	{
		this->StopTask(TaskStoppingReason::tsrError);
		MessageBox(this->Handle, L"Unable to find game window!", L"Error", MB_ICONSTOP);

		return;
	}

    static unsigned int nCycleCounter = 0;
	static unsigned int uBattleTimeout = 0;
	static unsigned int uBattleDelayInSeconds = 0;
	static unsigned int nScreenCheckFailures = 0;
	static bool bScreenCheckPassed = false;
	static unsigned int uScreenCheckingTiomeout = 0;
	static unsigned int uResultSavingTimeout = 0;

	//Сохранение скриншота (отчёта) боя с заданным периодом
	if (g_pSettingsManager->SaveResults &&
		(g_pSettingsManager->ResultSavingMethod == ResultSavingMode::rsmPeriodically) &&
		(uResultSavingTimeout == g_pSettingsManager->ResultSavingPeriod))
	{
		this->SaveResult(nCycleCounter);
        uResultSavingTimeout = 0;
	}

	if (!nCycleCounter)
	{
		//Кешируем значение времени задачи в секундах один раз, в начале первого боя
		uBattleDelayInSeconds = (HIWORD(m_ActiveTaskInfo.Settings.uDelay) * 60) + LOWORD(m_ActiveTaskInfo.Settings.uDelay);

		if (!uBattleDelayInSeconds)
		{
			this->StopTask(TaskStoppingReason::tsrError);
			MessageBox(this->Handle, L"Battle time should be greater than 0!", L"Error", MB_ICONSTOP);

			return;
        }
	}

	if ((nCycleCounter > m_ActiveTaskInfo.Settings.nNumberOfBattles) && bScreenCheckPassed)
	{
		//Успешное завершение задачи
		nCycleCounter = 0;
		this->StopTask(TaskStoppingReason::tsrSuccessfulCompletion);

		return;
	}
	else if (m_bForceStopTask)
	{
		//Выходим из задачи
		nCycleCounter = 0;
		this->StopTask(TaskStoppingReason::tsrUser);

        return;
    }

    //Обновляем прогресс текущего боя
	ProgressBarBattle->Position = 100.0f * (static_cast<float>(uBattleTimeout) / static_cast<float>(uBattleDelayInSeconds));

    //Начало каждого боя
	if (!nCycleCounter || ((uBattleTimeout == uBattleDelayInSeconds) && bScreenCheckPassed))
	{
		nCycleCounter++;
		if (nCycleCounter <= m_ActiveTaskInfo.Settings.nNumberOfBattles)
		{
			uBattleTimeout = 0;
			LabelBattlesCounter->Caption = String(nCycleCounter) + L"/" +
				String(m_ActiveTaskInfo.Settings.nNumberOfBattles);

			String strTrayHint;
			strTrayHint.sprintf(L"%s (%i/%i)", Application->Title.c_str(), nCycleCounter,
				m_ActiveTaskInfo.Settings.nNumberOfBattles);
			TrayIconApp->Hint = strTrayHint;
		}
		LabelBattlesCounter->Font->Color = clSilver;

		nScreenCheckFailures = 0;
		bScreenCheckPassed = false;
		uScreenCheckingTiomeout = 0;
		uResultSavingTimeout = 0;
	}

	//Тут 90% магии
	//Если мы сюда попали не потому, что это самое начало, а потому, что были ошибки, ждём до заданного периода
	if (!nScreenCheckFailures || (nScreenCheckFailures && (uScreenCheckingTiomeout == g_pSettingsManager->ScreenCheckingPeriod)))
	{
		uScreenCheckingTiomeout = 0;

		//Если задан этот параметр, сначала проверяем кнопку "Начать", подразумевая, что перед нами экран начала боя
		bool bSTARTMenuPassed = false;
		if (nCycleCounter == 1 && m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst)
		{
			if (g_pRAIDWorker->ComparePixels(m_ActiveTaskInfo.Settings.STARTScreenControlPoint, m_ActiveTaskInfo.Settings.STARTScreenControlPointColor))
			{
				//Отправляем клавишу Enter окну игры, на экране начала боя это равносильно нажатию "Начать"
				g_pRAIDWorker->SendKey(VK_RETURN);
				LabelBattlesCounter->Font->Color = clBlack;
				bScreenCheckPassed = true;
			}
		}

		//Проверяем прерыватели выполнения задачи
		if (g_pRAIDWorker->ComparePixels(g_pSettingsManager->EnergyDialogControlPoint, g_pSettingsManager->EnergyDialogControlPointColor))
		{
			switch (g_pSettingsManager->EnergyDialogPreferredAction)
			{
				case PromptDialogAction::pdaAccept:
				{
					TPoint AcceptButtonPos;
					//
					g_pRAIDWorker->SendMouseClick(AcceptButtonPos);
					break;
				}
				case PromptDialogAction::pdaSkip:
					g_pRAIDWorker->SendKey(VK_ESCAPE);
					break;
			}
		}

		//Если мы прошли первый бой или если не удалось найти кнопку "Начать", подразумеваем, что перед нами экран
		//результатов боя и пытаемся нажать кнопку "Повтор"
		if (!m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst || (m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst && nCycleCounter > 1)
			|| (m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst && !bScreenCheckPassed))
		{
			if (g_pRAIDWorker->ComparePixels(m_ActiveTaskInfo.Settings.REPLAYScreenControlPoint, m_ActiveTaskInfo.Settings.REPLAYScreenControlPointColor))
			{
				//Отчёт, если задано создание скриншота по завершению каждого боя
				if ((nCycleCounter > 1) && g_pSettingsManager->SaveResults &&
					(g_pSettingsManager->ResultSavingMethod == ResultSavingMode::rsmAtTheEndOfEachBattle))
				{
					this->SaveResult(nCycleCounter - 1);
				}

				//Если мы прошли последний бой (счётчик nCycleCounter будет равен nNumberOfBattles + 1),
				//просто ждём экрана результатов
				if (nCycleCounter <= m_ActiveTaskInfo.Settings.nNumberOfBattles)
				{
					//[R] на экране результатов боя эквивалентно нажатию кнопки "Повтор"
					g_pRAIDWorker->SendKey('R');
					LabelBattlesCounter->Font->Color = clBlack;
				}

				bScreenCheckPassed = true;
			}
			else
				nScreenCheckFailures++;
		}

		//Если превысили число ошибочных попыток сравнения, завершаем всю задачу
		if (nScreenCheckFailures == g_pSettingsManager->TriesBeforeForceTaskEnding)
		{
            nCycleCounter = 0;
			this->StopTask(TaskStoppingReason::tsrError);

			return;
		}
	}

	if (bScreenCheckPassed)
	{
		uBattleTimeout++;
		uResultSavingTimeout++;
	}
	else
		uScreenCheckingTiomeout++;
}

//---------------------------------------------------------------------------
void __fastcall TFormMain::ShowHideAutomatizer1Click(TObject *Sender)
{
	this->Visible = !this->Visible;
	if (this->WindowState != wsNormal)
		this->WindowState = wsNormal;
	if (this->Visible)
		SetForegroundWindow(this->Handle);
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::OpenResults1Click(TObject *Sender)
{
	if (!g_pSettingsManager->PathForResults.IsEmpty() && !DirectoryExists(g_pSettingsManager->PathForResults))
	{
		MessageBox(this->Handle, L"There is no results!", L"Error", MB_ICONSTOP);
		return;
	}

	String strPath;
	if (!g_pSettingsManager->PathForResults.IsEmpty())
	{
		strPath = g_pSettingsManager->PathForResults;
	}
	else
	{
		strPath = IncludeTrailingPathDelimiter(ExtractFilePath(Application->ExeName)) +
			g_strDefaultResultsFolder;
		if (!DirectoryExists(strPath))
			TDirectory::CreateDirectory(strPath);
	}

	ShellExecute(NULL, L"OPEN", strPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::Exit1Click(TObject *Sender)
{
	this->Close();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::TrayIconAppDblClick(TObject *Sender)
{
	this->ShowHideAutomatizer1Click(this);
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::FormCreate(TObject *Sender)
{
	g_pSettingsManager = new TSettingsManager;
	g_pSettingsManager->ReadINI();

	g_pRAIDWorker = new TRAIDWorker;

	PageControlURAIDASettings->ActivePageIndex = g_pSettingsManager->RecentActivePage;
	this->PageControlURAIDASettingsChange(this);
	ButtonRunTask->Caption = m_strButtonRTRunCaption;

	//Вывод названия и версии программы
	String strAppVersion;
	GetFileVersion(Application->ExeName, strAppVersion, fvfMajorMinor);
	LabelCopyright1->Caption = Application->Title + L" v." + strAppVersion + L" by";
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnSSPickPointClick(TObject *Sender)
{
	FormPickColor->ShowModal();

	PCData PCResults;
	PCResults = FormPickColor->GetResults();

	if (!PCResults.bCancelled)
	{
		UpDownSSXPos->Position = PCResults.XY.x;
		UpDownSSYPos->Position = PCResults.XY.y;
		PanelSSColor->Color = PCResults.Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonApplyGWSizeClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;

	g_pRAIDWorker->ResizeGameWindow(g_pSettingsManager->RAIDWindowSize);
}
//---------------------------------------------------------------------------
void TFormMain::UpdateGMSpecSettingsFrame(TTabSheet *pPage)
{
	GameModeSpecSettings GMSpecSettings;

	if (pPage == TabSheetCampaign)
	{
		GMSpecSettings = g_pSettingsManager->CampaignSettings;
	}
	else if (pPage == TabSheetDungeons)
	{
		GMSpecSettings = g_pSettingsManager->DungeonsSettings;
	}
	else if (pPage == TabSheetFactionWars)
	{
		GMSpecSettings = g_pSettingsManager->FactionWarsSettings;
	}

	CheckBoxProcessSTARTScreen->Checked = GMSpecSettings.bProcessSTARTScreenFirst;
	UpDownSSXPos->Position = GMSpecSettings.STARTScreenControlPoint.x;
	UpDownSSYPos->Position = GMSpecSettings.STARTScreenControlPoint.y;
	PanelSSColor->Color = GMSpecSettings.STARTScreenControlPointColor;
	UpDownRSXPos->Position = GMSpecSettings.REPLAYScreenControlPoint.x;
	UpDownRSYPos->Position = GMSpecSettings.REPLAYScreenControlPoint.y;
	PanelRSColor->Color = GMSpecSettings.REPLAYScreenControlPointColor;
	UpDownBTMinutes->Position = HIWORD(GMSpecSettings.uDelay);
	UpDownBTSeconds->Position = LOWORD(GMSpecSettings.uDelay);
	UpDownNumberofBattles->Position = GMSpecSettings.nNumberOfBattles;
}
//---------------------------------------------------------------------------
GameModeSpecSettings TFormMain::SaveSettingsFromGMSpecSettingsFrame()
{
	GameModeSpecSettings GMSpecSettings;
	GMSpecSettings.bProcessSTARTScreenFirst = CheckBoxProcessSTARTScreen->Checked;
	GMSpecSettings.STARTScreenControlPoint.x = UpDownSSXPos->Position;
	GMSpecSettings.STARTScreenControlPoint.y = UpDownSSYPos->Position;
	GMSpecSettings.STARTScreenControlPointColor = PanelSSColor->Color;
	GMSpecSettings.REPLAYScreenControlPoint.x = UpDownRSXPos->Position;
	GMSpecSettings.REPLAYScreenControlPoint.y = UpDownRSYPos->Position;
	GMSpecSettings.REPLAYScreenControlPointColor = PanelRSColor->Color;
	GMSpecSettings.uDelay = MAKELONG(UpDownBTSeconds->Position, UpDownBTMinutes->Position);
	GMSpecSettings.nNumberOfBattles = UpDownNumberofBattles->Position;

	if (PageControlURAIDASettings->ActivePage == TabSheetCampaign)
	{
		GMSpecSettings.GameMode = SupportedGameModes::gmCampaign;
		g_pSettingsManager->CampaignSettings = GMSpecSettings;
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetDungeons)
	{
		GMSpecSettings.GameMode = SupportedGameModes::gmDungeons;
		g_pSettingsManager->DungeonsSettings = GMSpecSettings;
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetFactionWars)
	{
		GMSpecSettings.GameMode = SupportedGameModes::gmFactionWars;
		g_pSettingsManager->FactionWarsSettings = GMSpecSettings;
	}

	//Запись в файл
	g_pSettingsManager->UpdateINI();

	return GMSpecSettings;
}
//---------------------------------------------------------------------------
void TFormMain::UpdateCommonSettingsFrame()
{
	UpDownGWWidth->Position = g_pSettingsManager->RAIDWindowSize.cx;
	UpDownGWHeight->Position = g_pSettingsManager->RAIDWindowSize.cy;
	CheckBoxSaveResults->Checked = g_pSettingsManager->SaveResults;
	OpenResults1->Enabled = CheckBoxSaveResults->Checked;

	switch (g_pSettingsManager->ResultSavingMethod)
	{
		case ResultSavingMode::rsmAtTheEndOfEachBattle:
			RadioButtonSRAtTheEndOfEachBattle->Checked = true;
			break;
		case ResultSavingMode::rsmPeriodically:
			RadioButtonSRPeriodically->Checked = true;
			break;
	}

	UpDownSRPeriodically->Position = g_pSettingsManager->ResultSavingPeriod;
	EditSRPath->Text = g_pSettingsManager->PathForResults;

	ComboBoxTaskEndAction->ItemIndex = static_cast<int>(g_pSettingsManager->TaskEndBehavior);
	CheckBoxTEAExit->Checked = g_pSettingsManager->ExitOnTaskEnding;
	CheckBoxTEACloseTheGame->Checked = g_pSettingsManager->CloseGameOnTaskEnding;

	UpDownTriesBeforeFTE->Position = g_pSettingsManager->TriesBeforeForceTaskEnding;
	UpDownScreenCheckingInterval->Position = g_pSettingsManager->ScreenCheckingPeriod;
	UpDownColorTolerance->Position = g_pSettingsManager->ColorTolerance;

	UpDownEDXPos->Position = g_pSettingsManager->EnergyDialogControlPoint.x;
	UpDownEDYPos->Position = g_pSettingsManager->EnergyDialogControlPoint.y;
	PanelEDColor->Color = g_pSettingsManager->EnergyDialogControlPointColor;

	switch (g_pSettingsManager->EnergyDialogPreferredAction)
	{
		case PromptDialogAction::pdaAccept:
			RadioButtonEDAccept->Checked = true;
			break;
		case PromptDialogAction::pdaSkip:
			RadioButtonEDSkip->Checked = true;
			break;
	}

	UpDownSMXPos->Position = g_pSettingsManager->SMDialogControlPoint.x;
	UpDownSMYPos->Position = g_pSettingsManager->SMDialogControlPoint.y;
	PanelSMColor->Color = g_pSettingsManager->SMDialogControlPointColor;

	switch (g_pSettingsManager->SMDialogPreferredAction)
	{
		case PromptDialogAction::pdaAccept:
			RadioButtonSMAccept->Checked = true;
			break;
		case PromptDialogAction::pdaSkip:
			RadioButtonSMSkip->Checked = true;
			break;
	}
}
//---------------------------------------------------------------------------
void TFormMain::SaveSettingsFromCommonSettingsFrame()
{
	TSize NewGWSize(UpDownGWWidth->Position, UpDownGWHeight->Position);
	TSize CurrentGWSize = g_pSettingsManager->RAIDWindowSize;

	//При сохранении размеров окна игры пересчитываем координаты всех контрольных точек,
	//если новые размеры отличаются от прежних
	if (NewGWSize != CurrentGWSize)
	{
		float fWidthCoeff, fHeightCoeff;

		if (NewGWSize.cx > CurrentGWSize.cx)
			fWidthCoeff = static_cast<float>(CurrentGWSize.cx) / static_cast<float>(NewGWSize.cx);
		else
			fWidthCoeff = static_cast<float>(NewGWSize.cx) / static_cast<float>(CurrentGWSize.cx);

		if (NewGWSize.cy > CurrentGWSize.cy)
			fHeightCoeff = static_cast<float>(CurrentGWSize.cy) / static_cast<float>(NewGWSize.cy);
		else
			fHeightCoeff = static_cast<float>(NewGWSize.cy) / static_cast<float>(CurrentGWSize.cy);

		GameModeSpecSettings GMSpecSettings;
		GMSpecSettings = g_pSettingsManager->CampaignSettings;
		GMSpecSettings.ShiftCoordinates(fWidthCoeff, fHeightCoeff);
		g_pSettingsManager->CampaignSettings = GMSpecSettings;
		GMSpecSettings = g_pSettingsManager->DungeonsSettings;
		GMSpecSettings.ShiftCoordinates(fWidthCoeff, fHeightCoeff);
		g_pSettingsManager->DungeonsSettings = GMSpecSettings;
		GMSpecSettings = g_pSettingsManager->FactionWarsSettings;
		GMSpecSettings.ShiftCoordinates(fWidthCoeff, fHeightCoeff);
		g_pSettingsManager->FactionWarsSettings = GMSpecSettings;

		this->UpDownEDXPos->Position *= fWidthCoeff;
		this->UpDownEDYPos->Position *= fHeightCoeff;
		this->UpDownSMXPos->Position *= fWidthCoeff;
		this->UpDownSMYPos->Position *= fHeightCoeff;

		g_pSettingsManager->RAIDWindowSize = NewGWSize;
	}

	g_pSettingsManager->SaveResults = CheckBoxSaveResults->Checked;

	if (RadioButtonSRAtTheEndOfEachBattle->Checked)
	{
		g_pSettingsManager->ResultSavingMethod = ResultSavingMode::rsmAtTheEndOfEachBattle;
	}
	else if (RadioButtonSRPeriodically->Checked)
	{
		g_pSettingsManager->ResultSavingMethod = ResultSavingMode::rsmPeriodically;
	}

	g_pSettingsManager->ResultSavingPeriod = UpDownSRPeriodically->Position;
	g_pSettingsManager->PathForResults = EditSRPath->Text;

	g_pSettingsManager->TaskEndBehavior = static_cast<TaskEndAction>(ComboBoxTaskEndAction->ItemIndex);
	g_pSettingsManager->ExitOnTaskEnding = CheckBoxTEAExit->Checked;
	g_pSettingsManager->CloseGameOnTaskEnding = CheckBoxTEACloseTheGame->Checked;

	g_pSettingsManager->TriesBeforeForceTaskEnding = UpDownTriesBeforeFTE->Position;
	g_pSettingsManager->ScreenCheckingPeriod = UpDownScreenCheckingInterval->Position;
	g_pSettingsManager->ColorTolerance = UpDownColorTolerance->Position;

	g_pSettingsManager->EnergyDialogControlPoint = TPoint(UpDownEDXPos->Position, UpDownEDYPos->Position);
	g_pSettingsManager->EnergyDialogControlPointColor = PanelEDColor->Color;

	if (RadioButtonEDAccept->Checked)
	{
		g_pSettingsManager->EnergyDialogPreferredAction = PromptDialogAction::pdaAccept;
	}
	else if (RadioButtonEDSkip->Checked)
	{
		g_pSettingsManager->EnergyDialogPreferredAction = PromptDialogAction::pdaSkip;
	}

	g_pSettingsManager->SMDialogControlPoint = TPoint(UpDownSMXPos->Position, UpDownSMYPos->Position);
	g_pSettingsManager->SMDialogControlPointColor = PanelSMColor->Color;

	if (RadioButtonSMAccept->Checked)
	{
		g_pSettingsManager->SMDialogPreferredAction = PromptDialogAction::pdaAccept;
	}
	else if (RadioButtonSMSkip->Checked)
	{
		g_pSettingsManager->SMDialogPreferredAction = PromptDialogAction::pdaSkip;
	}

	//Запись в файл
	g_pSettingsManager->UpdateINI();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::ButtonUseCurrentGWSizeClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;

	TRect GameWindowSize = g_pRAIDWorker->GetGameWindowSize(true);
	UpDownGWWidth->Position = GameWindowSize.Width();
	UpDownGWHeight->Position = GameWindowSize.Height();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnRSPickPointClick(TObject *Sender)
{
	FormPickColor->ShowModal();

	PCData PCResults;
	PCResults = FormPickColor->GetResults();

	if (!PCResults.bCancelled)
	{
		UpDownRSXPos->Position = PCResults.XY.x;
		UpDownRSYPos->Position = PCResults.XY.y;
		PanelRSColor->Color = PCResults.Color;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormResize(TObject *Sender)
{
	if (this->WindowState == wsMinimized)
        this->Hide();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::LinkLabel1Click(TObject *Sender)
{
	ShellExecute(NULL, L"OPEN", L"https://paypal.me/BorisVorontsov", NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::LinkLabel2Click(TObject *Sender)
{
	String strAddress;
	String strFullAppVersion;
	GetFileVersion(Application->ExeName, strFullAppVersion, fvfMajorMinorReleaseBuild);
	strAddress.sprintf(L"mailto:borisvorontsov@yandex.ru?subject=%s v%s feedback", Application->Title.c_str(),
		strFullAppVersion.c_str());

	ShellExecute(NULL, NULL, strAddress.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::TrayIconAppBalloonClick(TObject *Sender)
{
	if (g_pSettingsManager->SaveResults)
	{
		this->OpenResults1Click(this);
	}
	else
	{
		this->Show();
		if (this->WindowState != wsNormal)
			this->WindowState = wsNormal;
		SetForegroundWindow(this->Handle);
    }
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::FormShow(TObject *Sender)
{
	this->UpdateGMSpecSettingsFrame(PageControlURAIDASettings->ActivePage);
	this->UpdateCommonSettingsFrame();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::FormClose(TObject *Sender, TCloseAction &Action)
{
	this->SaveSettingsFromGMSpecSettingsFrame();
	this->SaveSettingsFromCommonSettingsFrame();

	g_pSettingsManager->RecentActivePage = PageControlURAIDASettings->ActivePageIndex;
    g_pSettingsManager->UpdateINI();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PageControlURAIDASettingsChange(TObject *Sender)
{
	if (PageControlURAIDASettings->ActivePage != TabSheetCommon)
	{
		if (ScrollBoxGMSpecSettings->Parent != PageControlURAIDASettings->ActivePage)
			ScrollBoxGMSpecSettings->Parent = PageControlURAIDASettings->ActivePage;
		this->UpdateGMSpecSettingsFrame(PageControlURAIDASettings->ActivePage);
	}
	else /*TabSheetCommon*/
	{
		this->UpdateCommonSettingsFrame();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PageControlURAIDASettingsChanging(TObject *Sender, bool &AllowChange)
{
	if (PageControlURAIDASettings->ActivePage != TabSheetCommon)
	{
		this->SaveSettingsFromGMSpecSettingsFrame();
	}
	else
	{
		this->SaveSettingsFromCommonSettingsFrame();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonSRBrowsePathClick(TObject *Sender)
{
	String strPath;
	String strInitialDir = (EditSRPath->Text.IsEmpty())?ExtractFilePath(Application->ExeName):EditSRPath->Text;
	FileOpenDialogGeneric->Options = TFileDialogOptions() << fdoPickFolders << fdoPathMustExist;
	FileOpenDialogGeneric->Title = L"Select directory for results";
	FileOpenDialogGeneric->DefaultFolder = strInitialDir;
    //BrowseForFolderDialog
	if (FileOpenDialogGeneric->Execute(this->Handle))
	{
		EditSRPath->Text = FileOpenDialogGeneric->FileName;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonClearAllResultsClick(TObject *Sender)
{
	if (g_pSettingsManager->PathForResults.IsEmpty() || !DirectoryExists(g_pSettingsManager->PathForResults))
	{
		MessageBox(this->Handle, L"Path error!", L"Error", MB_ICONSTOP);
		return;
	}

	if (!TDirectory::IsEmpty(g_pSettingsManager->PathForResults))
	{
		if (MessageBox(this->Handle, L"Are you sure you want to delete all results?", L"Warning",
			MB_YESNO | MB_DEFBUTTON2 | MB_ICONEXCLAMATION) == IDNO)
		{
			return;
		}
	}

	TDirectory::Delete(g_pSettingsManager->PathForResults, true);
}
//---------------------------------------------------------------------------
void TFormMain::StartTask()
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;

	if (m_ActiveTaskInfo.CurrentState == TaskState::tsStopped)
	{
		if (PageControlURAIDASettings->ActivePage == TabSheetCommon) {
			MessageBox(this->Handle, L"Select game mode first", L"Warning", MB_ICONEXCLAMATION);
			return;
		}

		g_pRAIDWorker->ValidateGameWindow();

		//Запускаем таймер задачи по указанным параметрам
		//Параметры будут применены только при старте новой задачи
		m_ActiveTaskInfo.Settings = this->SaveSettingsFromGMSpecSettingsFrame();

		ProgressBarBattle->Position = 0;
		LabelBattlesCounter->Font->Color = clBlack;

		m_ActiveTaskInfo.StartTime = Now();
	}

	switch (m_ActiveTaskInfo.CurrentState)
	{
		case TaskState::tsStopped:
		case TaskState::tsPaused:
			m_ActiveTaskInfo.CurrentState = TaskState::tsRunning;
			ButtonRunTask->Caption = m_strButtonRTPauseCaption;

			TimerMain->Enabled = true;
			break;
		case TaskState::tsRunning:
			m_ActiveTaskInfo.CurrentState = TaskState::tsPaused;
			ButtonRunTask->Caption = m_strButtonRTResumeCaption;

			TimerMain->Enabled = false;
			break;
	}

	PageControlURAIDASettings->Visible = false;
}
//---------------------------------------------------------------------------
void TFormMain::StopTask(TaskStoppingReason Reason)
{
	if (m_ActiveTaskInfo.CurrentState == TaskState::tsStopped)
		return;

	TimerMain->Enabled = false;
	ProgressBarBattle->Position = 100;

	m_ActiveTaskInfo.CurrentState = TaskState::tsStopped;
	ButtonRunTask->Caption = m_strButtonRTRunCaption;

    TrayIconApp->Hint = Application->Title;

	switch (Reason)
	{
		case TaskStoppingReason::tsrUser:
			LabelBattlesCounter->Font->Color = clGray;
			break;
		case TaskStoppingReason::tsrSuccessfulCompletion:
			LabelBattlesCounter->Font->Color = clGreen;
			break;
		case TaskStoppingReason::tsrError:
			LabelBattlesCounter->Font->Color = clRed;
			break;
	}

    //Если это не пользователь нажал кнопку..
	if (Reason != TaskStoppingReason::tsrUser)
	{
		//Выполняем сценарий завершения задачи
		if (g_pSettingsManager->CloseGameOnTaskEnding)
			g_pRAIDWorker->CloseGame();

		switch (g_pSettingsManager->TaskEndBehavior)
		{
			case TaskEndAction::teaDoNothing:
				break;
			case TaskEndAction::teaShowNotification:
				TrayIconApp->BalloonTitle = Application->Title;
				if (Reason == TaskStoppingReason::tsrSuccessfulCompletion)
				{
					TrayIconApp->BalloonHint = L"Task completed successfully";
				}
				else if (Reason == TaskStoppingReason::tsrError)
				{
					TrayIconApp->BalloonHint = L"Force task termination due to errors";
				}
				TrayIconApp->ShowBalloonHint();
				break;
			case TaskEndAction::teaPlayAlert:
				for (int i = 0; i < 3; i++)
				{
					PlaySound(reinterpret_cast<LPWSTR>(SND_ALIAS_SYSTEMASTERISK), NULL, SND_SYNC | SND_ALIAS_ID);
					Wait(1000);
				}
				break;
			case TaskEndAction::teaGoToSleep:
				SetSuspendState(FALSE, FALSE, FALSE);
				break;
			case TaskEndAction::teaTurnOffPC:
				ExitWindowsEx(EWX_SHUTDOWN, SHTDN_REASON_MAJOR_APPLICATION | SHTDN_REASON_MINOR_MAINTENANCE);
				break;
		}

		if (g_pSettingsManager->ExitOnTaskEnding)
			this->Close();
	}

	PageControlURAIDASettings->Visible = true;
}
//---------------------------------------------------------------------------
void TFormMain::SaveResult(unsigned int nBattleNumber)
{
	if (m_ActiveTaskInfo.CurrentState == TaskState::tsStopped)
		return;

	String strPathForResults = g_pSettingsManager->PathForResults;
	String strGameMode, strActiveTaskSubDir, strFinalPath;
	String strFileName;
	//Проверяем/формируем путь к результатам
	if (strPathForResults.IsEmpty())
	{
		strPathForResults = IncludeTrailingPathDelimiter(ExtractFilePath(Application->ExeName)) +
			g_strDefaultResultsFolder;
		if (!DirectoryExists(strPathForResults))
			TDirectory::CreateDirectory(strPathForResults);
	}
	else if (!DirectoryExists(strPathForResults))
	{
		TDirectory::CreateDirectory(strPathForResults);
	}

	//Формируем имя подпапки для текущей задачи
	switch (m_ActiveTaskInfo.Settings.GameMode)
	{
		case SupportedGameModes::gmCampaign:
			strGameMode = L"Campaign";
			break;
		case SupportedGameModes::gmDungeons:
			strGameMode = L"Dungeons";
			break;
		case SupportedGameModes::gmFactionWars:
			strGameMode = L"FactionWars";
			break;
	}
	strActiveTaskSubDir.sprintf(L"%s_%s", strGameMode.c_str(),
		FormatDateTime(L"dd_mmmm_yyyy_hh-mm", m_ActiveTaskInfo.StartTime).c_str());

	//Формируем имя файла JPEG
	if (nBattleNumber != -1)
	{
		strFileName.sprintf(L"Battle%i_%s.jpeg", nBattleNumber, FormatDateTime(L"hh-mm", Now()).c_str());
	}
	else
	{
		strFileName.sprintf(L"Battle_%s.jpeg", FormatDateTime(L"hh-mm", Now()).c_str());
	}

	strFinalPath = IncludeTrailingPathDelimiter(strPathForResults) + strActiveTaskSubDir;
	CreateDir(strFinalPath);

	std::shared_ptr<TImage> pImage(new TImage(this));
	std::shared_ptr<TJPEGImage> pJPEGImage(new TJPEGImage());

	TRect FrameSize = g_pRAIDWorker->GetGameWindowSize(true);
	pImage->Width = FrameSize.Right;
	pImage->Height = FrameSize.Bottom;
	g_pRAIDWorker->CaptureFrame(pImage->Canvas, TSize(FrameSize.Right, FrameSize.Bottom));
	pJPEGImage->Assign(pImage->Picture->Bitmap);

	pJPEGImage->SaveToFile(IncludeTrailingPathDelimiter(strFinalPath) + strFileName);
}

//---------------------------------------------------------------------------

void __fastcall TFormMain::CheckBoxSaveResultsClick(TObject *Sender)
{
	OpenResults1->Enabled = CheckBoxSaveResults->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnEDPickPointClick(TObject *Sender)
{
	FormPickColor->ShowModal();

	PCData PCResults;
	PCResults = FormPickColor->GetResults();

	if (!PCResults.bCancelled)
	{
		UpDownEDXPos->Position = PCResults.XY.x;
		UpDownEDYPos->Position = PCResults.XY.y;
		PanelEDColor->Color = PCResults.Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnSMPickColorClick(TObject *Sender)
{
	FormPickColor->ShowModal();

	PCData PCResults;
	PCResults = FormPickColor->GetResults();

	if (!PCResults.bCancelled)
	{
		UpDownSMXPos->Position = PCResults.XY.x;
		UpDownSMYPos->Position = PCResults.XY.y;
		PanelSMColor->Color = PCResults.Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PanelSSColorClick(TObject *Sender)
{
	ColorDialogCPColor->Color = PanelSSColor->Color;
	if (ColorDialogCPColor->Execute(this->Handle))
	{
		PanelSSColor->Color = ColorDialogCPColor->Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PanelRSColorClick(TObject *Sender)
{
	ColorDialogCPColor->Color = PanelRSColor->Color;
	if (ColorDialogCPColor->Execute(this->Handle))
	{
		PanelRSColor->Color = ColorDialogCPColor->Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PanelEDColorClick(TObject *Sender)
{
	ColorDialogCPColor->Color = PanelEDColor->Color;
	if (ColorDialogCPColor->Execute(this->Handle))
	{
		PanelEDColor->Color = ColorDialogCPColor->Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PanelSMColorClick(TObject *Sender)
{
	ColorDialogCPColor->Color = PanelSMColor->Color;
	if (ColorDialogCPColor->Execute(this->Handle))
	{
		PanelSMColor->Color = ColorDialogCPColor->Color;
	}
}
//---------------------------------------------------------------------------


