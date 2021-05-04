//---------------------------------------------------------------------------

#include "URAIDAPCH.h"
#include <Mmsystem.h>
#include <powrprof.h>
#include <Jpeg.hpp>
#include <IOUtils.hpp>
#include <memory>
#pragma hdrstop

#include "UnitFormMain.h"
#include "UnitFormPickPoint.h"
#include "UnitCommon.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMain *FormMain;

TSettingsManager *g_pSettingsManager;
TRAIDWorker *g_pRAIDWorker;

const String TFormMain::m_strButtonRTRunCaption = L"Старт";
const String TFormMain::m_strButtonRTPauseCaption = L"Пауза";
const String TFormMain::m_strButtonRTResumeCaption = L"Далее";

//---------------------------------------------------------------------------
__fastcall TFormMain::TFormMain(TComponent* Owner)
	: TForm(Owner)
{
	AdjustPrivilege(SE_SHUTDOWN_NAME);

	m_ActiveTaskInfo.CurrentState = TaskState::tsStopped;
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::TimerMainTimer(TObject *Sender)
{
	static unsigned int nCycleCounter = 0;
	static unsigned int uBattleTimeout = 0;
	static unsigned int uBattleDelayInSeconds = 0;
	static unsigned int nScreenCheckFailures = 0;
	static bool bScreenCheckPassed = false;
	static unsigned int uScreenCheckingTiomeout = 0;
	static unsigned int uResultSavingTimeout = 0;

	//А вдруг?
	if (!g_pRAIDWorker->IsGameRunning())
	{
        nCycleCounter = 0;
		this->StopTask(TaskStoppingReason::tsrError);
		MessageBox(this->Handle, L"Не удаётся найти окно игры!", L"Ошибка", MB_ICONSTOP);

		return;
	}

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
			MessageBox(this->Handle, L"Время боя должно быть больше нуля!", L"Ошибка", MB_ICONSTOP);

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

	if (bScreenCheckPassed)
	{
		//Обновляем прогресс текущего боя
		ProgressBarBattle->Position = 100.0f * (static_cast<float>(uBattleTimeout) /
			static_cast<float>(uBattleDelayInSeconds));
		TaskbarApp->ProgressValue = ProgressBarBattle->Position;
	}

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
			TrayIconApp->BalloonHint = L"";
			TrayIconApp->Hint = strTrayHint;
		}
		LabelBattlesCounter->Font->Color = clSilver;
		ProgressBarBattle->Style = TProgressBarStyle::pbstMarquee;

		nScreenCheckFailures = 0;
		bScreenCheckPassed = false;
		uScreenCheckingTiomeout = 0;
		uResultSavingTimeout = 0;
	}

	//Тут 90% магии
	//Если мы сюда попали не потому, что это самое начало, а потому, что были ошибки, ждём до заданного периода
	if ((!nScreenCheckFailures || (nScreenCheckFailures && (uScreenCheckingTiomeout == g_pSettingsManager->ScreenCheckingPeriod))) &&
		!bScreenCheckPassed)
	{
		bool bEnergyDialogProcessed;

		g_pRAIDWorker->ValidateGameWindow();

		uScreenCheckingTiomeout = 0;

		//Прерыватель задачи: диалог пополнения энергии
		//Появляется при её нехватке после попытки запуска боя
		//Вызываем после попыток запустить бой
		auto EnergyDialogTest {
			[=, &bEnergyDialogProcessed](bool& bExitTimer)
			{
				bEnergyDialogProcessed = false;

				if (g_pRAIDWorker->ComparePixels(g_pSettingsManager->EnergyDialogControlPoint,
					g_pSettingsManager->EnergyDialogControlPointColor, g_pSettingsManager->EnergyDialogControlPointColorTolerance))
				{
					switch (g_pSettingsManager->EnergyDialogPreferredAction)
					{
						case PromptDialogAction::pdaAccept:
							//Кликаем, куда задал пользователь (предполагается кнопка "Получить")
							g_pRAIDWorker->SendMouseClick(g_pSettingsManager->EnergyDialogGETButtonPoint);
							bExitTimer = false;
							break;
						case PromptDialogAction::pdaSkip:
							g_pRAIDWorker->SendKey(VK_ESCAPE);
							bExitTimer = false;
							break;
						case PromptDialogAction::pdaAbort:
						{
							//Если задано, просто заканчиваем задачу
							nCycleCounter = 0;
							this->StopTask(TaskStoppingReason::tsrUser);
							bExitTimer = true;
							break;
						}
					}

					bEnergyDialogProcessed = true;
				}

				return bEnergyDialogProcessed;
			}
		};

		//Прерыватель задачи: диалог работ на сервере
		if (g_pRAIDWorker->ComparePixels(g_pSettingsManager->SMDialogControlPoint,
			g_pSettingsManager->SMDialogControlPointColor, g_pSettingsManager->SMDialogControlPointColorTolerance))
		{
			//Просто игнорируем
			g_pRAIDWorker->SendKey(VK_ESCAPE);
		}

		//Если задан этот параметр, сначала проверяем кнопку "Начать", подразумевая, что перед нами экран начала боя
		if (nCycleCounter == 1 && m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst)
		{
			if (g_pRAIDWorker->ComparePixels(m_ActiveTaskInfo.Settings.STARTScreenControlPoint.Coordinates,
				m_ActiveTaskInfo.Settings.STARTScreenControlPoint.PixelColor, m_ActiveTaskInfo.Settings.STARTScreenControlPoint.uTolerance))
			{
				//Отправляем клавишу Enter окну игры, на экране начала боя это равносильно нажатию "Начать"
				g_pRAIDWorker->SendKey(VK_RETURN);

				bool bExitTimer;
				if (EnergyDialogTest(bExitTimer))
				{
					if (bExitTimer)
						return;
				}
				else
				{
					LabelBattlesCounter->Font->Color = clBlack;
					ProgressBarBattle->Style = TProgressBarStyle::pbstNormal;
					bScreenCheckPassed = true;
                }
			}
		}

		//Если мы прошли первый бой или если не удалось найти кнопку "Начать", подразумеваем, что перед нами экран
		//результатов боя и пытаемся нажать кнопку "Повтор"
		if (!bEnergyDialogProcessed && (!m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst || (m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst && nCycleCounter > 1)
			|| (m_ActiveTaskInfo.Settings.bProcessSTARTScreenFirst && !bScreenCheckPassed)))
		{
			if (g_pRAIDWorker->ComparePixels(m_ActiveTaskInfo.Settings.REPLAYScreenControlPoint.Coordinates,
				m_ActiveTaskInfo.Settings.REPLAYScreenControlPoint.PixelColor, m_ActiveTaskInfo.Settings.REPLAYScreenControlPoint.uTolerance))
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
					if (m_ActiveTaskInfo.Settings.REPLAYScreenPreferredAction == REPLAYScreenAction::rsaReplay)
					{
						//[R] на экране результатов боя эквивалентно нажатию кнопки "Повтор"
						g_pRAIDWorker->SendKey('R');
					}
					else if (m_ActiveTaskInfo.Settings.REPLAYScreenPreferredAction == REPLAYScreenAction::rsaGoNext)
					{
						//Последовательное нажатие пробела и клавиши ввода запускает бой следующего уровня,
						//эквивалентно нажатию кнопки "Далее"
						g_pRAIDWorker->SendKey(VK_SPACE);
						g_pRAIDWorker->SendKey(VK_RETURN);
					}

					bool bExitTimer;
					if (EnergyDialogTest(bExitTimer))
					{
						if (bExitTimer)
							return;
					}
					else
					{
						LabelBattlesCounter->Font->Color = clBlack;
						ProgressBarBattle->Style = TProgressBarStyle::pbstNormal;
					}
				}

				if (!bEnergyDialogProcessed)
					bScreenCheckPassed = true;
			}
			else
				nScreenCheckFailures++;
		}

		//Если превысили число ошибочных попыток сравнения, завершаем всю задачу
		if (nScreenCheckFailures == g_pSettingsManager->TriesBeforeForceTaskEnding)
		{
			if (g_pSettingsManager->SaveResults)
			{
				this->SaveResult(nCycleCounter, true);
			}

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
void __fastcall TFormMain::MenuItemShowHideAutomatizerClick(TObject *Sender)
{
	this->Visible = !this->Visible;
	if (this->WindowState != wsNormal)
		this->WindowState = wsNormal;
	if (this->Visible)
		SetForegroundWindow(this->Handle);
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::MenuItemOpenResultsClick(TObject *Sender)
{
	if (!g_pSettingsManager->PathForResults.IsEmpty() && !DirectoryExists(g_pSettingsManager->PathForResults))
	{
		MessageBox(this->Handle, L"Результаты отсутствуют!", L"Ошибка", MB_ICONSTOP);
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
void __fastcall TFormMain::MenuItemExitClick(TObject *Sender)
{
	this->Close();
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::TrayIconAppDblClick(TObject *Sender)
{
	this->MenuItemShowHideAutomatizerClick(this);
}
//---------------------------------------------------------------------------
void __fastcall TFormMain::FormCreate(TObject *Sender)
{
	g_pSettingsManager = new TSettingsManager;
	g_pSettingsManager->ReadINI();

	this->Left = g_pSettingsManager->MainWindowPosition.x;
	this->Top = g_pSettingsManager->MainWindowPosition.y;

	if (g_pSettingsManager->StayOnTop)
	{
		MenuItemStayOnTop->Checked = true;
		this->FormStyle = TFormStyle::fsStayOnTop;
	}

	g_pRAIDWorker = new TRAIDWorker;

	PageControlURAIDASettings->ActivePageIndex = g_pSettingsManager->RecentActivePage;
	this->PageControlURAIDASettingsChange(this);
	BitBtnRunTask->Caption = m_strButtonRTRunCaption;
	std::shared_ptr<TBitmap> pGlyph(new TBitmap());
	ImageListRTButton->GetBitmap(0, pGlyph.get());
	BitBtnRunTask->Glyph = pGlyph.get();

	//Вывод названия и версии программы
	String strAppVersion;
	GetFileVersion(Application->ExeName, strAppVersion, fvfMajorMinor);
	LabelCopyright1->Caption = Application->Title + L" вер." + strAppVersion + L" от";
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnSSPickPointClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = false;

	if (FormPickPoint->Execute(this))
	{
		PickPointData PPResults = FormPickPoint->GetResults();

		UpDownSSX->Position = PPResults.XY.x;
		UpDownSSY->Position = PPResults.XY.y;
		PanelSSColor->Color = PPResults.Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonApplyGWSizeClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;

	g_pRAIDWorker->ResizeGameWindow(TSize(UpDownGWWidth->Position, UpDownGWHeight->Position));
}
//---------------------------------------------------------------------------
void TFormMain::UpdateGMSpecSettingsFrame(TTabSheet *pPage)
{
	//Чтение настроек
	g_pSettingsManager->ReadINI();

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
	UpDownSSX->Position = GMSpecSettings.STARTScreenControlPoint.Coordinates.x;
	UpDownSSY->Position = GMSpecSettings.STARTScreenControlPoint.Coordinates.y;
	PanelSSColor->Color = GMSpecSettings.STARTScreenControlPoint.PixelColor;
	UpDownSSColorTolerance->Position = GMSpecSettings.STARTScreenControlPoint.uTolerance;
	UpDownRSX->Position = GMSpecSettings.REPLAYScreenControlPoint.Coordinates.x;
	UpDownRSY->Position = GMSpecSettings.REPLAYScreenControlPoint.Coordinates.y;
	PanelRSColor->Color = GMSpecSettings.REPLAYScreenControlPoint.PixelColor;
	UpDownRSColorTolerance->Position = GMSpecSettings.REPLAYScreenControlPoint.uTolerance;

	switch (GMSpecSettings.REPLAYScreenPreferredAction)
	{
		case REPLAYScreenAction::rsaReplay:
			RadioButtonRSActionReplay->Checked = true;
			break;
		case REPLAYScreenAction::rsaGoNext:
			RadioButtonRSActionNext->Checked = true;
            break;
	}

	UpDownBTMinutes->Position = HIWORD(GMSpecSettings.uDelay);
	UpDownBTSeconds->Position = LOWORD(GMSpecSettings.uDelay);
	UpDownNumberofBattles->Position = GMSpecSettings.nNumberOfBattles;
}
//---------------------------------------------------------------------------
GameModeSpecSettings TFormMain::SaveSettingsFromGMSpecSettingsFrame()
{
	GameModeSpecSettings GMSpecSettings;
	GMSpecSettings.bProcessSTARTScreenFirst = CheckBoxProcessSTARTScreen->Checked;
	GMSpecSettings.STARTScreenControlPoint.Coordinates.x = UpDownSSX->Position;
	GMSpecSettings.STARTScreenControlPoint.Coordinates.y = UpDownSSY->Position;
	GMSpecSettings.STARTScreenControlPoint.PixelColor = PanelSSColor->Color;
	GMSpecSettings.STARTScreenControlPoint.uTolerance = UpDownSSColorTolerance->Position;
	GMSpecSettings.REPLAYScreenControlPoint.Coordinates.x = UpDownRSX->Position;
	GMSpecSettings.REPLAYScreenControlPoint.Coordinates.y = UpDownRSY->Position;
	GMSpecSettings.REPLAYScreenControlPoint.PixelColor = PanelRSColor->Color;
	GMSpecSettings.REPLAYScreenControlPoint.uTolerance = UpDownRSColorTolerance->Position;

	if (RadioButtonRSActionReplay->Checked)
	{
		GMSpecSettings.REPLAYScreenPreferredAction = REPLAYScreenAction::rsaReplay;
	}
	else if (RadioButtonRSActionNext->Checked)
	{
		GMSpecSettings.REPLAYScreenPreferredAction = REPLAYScreenAction::rsaGoNext;
	}

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
	//Чтение настроек
	g_pSettingsManager->ReadINI();

	UpDownGWWidth->Position = g_pSettingsManager->RAIDWindowSize.cx;
	UpDownGWHeight->Position = g_pSettingsManager->RAIDWindowSize.cy;
	CheckBoxSaveResults->Checked = g_pSettingsManager->SaveResults;
	MenuItemOpenResults->Enabled = CheckBoxSaveResults->Checked;

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
	CheckBoxClearOldResults->Checked = g_pSettingsManager->ClearOldResults;

	ComboBoxTaskEndAction->ItemIndex = static_cast<int>(g_pSettingsManager->TaskEndBehavior);
	CheckBoxTEAExit->Checked = g_pSettingsManager->ExitOnTaskEnding;
	CheckBoxTEACloseTheGame->Checked = g_pSettingsManager->CloseGameOnTaskEnding;

	UpDownTriesBeforeFTE->Position = g_pSettingsManager->TriesBeforeForceTaskEnding;
	UpDownScreenCheckingInterval->Position = g_pSettingsManager->ScreenCheckingPeriod;

	UpDownEDX->Position = g_pSettingsManager->EnergyDialogControlPoint.x;
	UpDownEDY->Position = g_pSettingsManager->EnergyDialogControlPoint.y;
	PanelEDColor->Color = g_pSettingsManager->EnergyDialogControlPointColor;
	UpDownEDColorTolerance->Position = g_pSettingsManager->EnergyDialogControlPointColorTolerance;

	UpDownEDGETX->Position = g_pSettingsManager->EnergyDialogGETButtonPoint.x;
	UpDownEDGETY->Position = g_pSettingsManager->EnergyDialogGETButtonPoint.y;

	switch (g_pSettingsManager->EnergyDialogPreferredAction)
	{
		case PromptDialogAction::pdaAccept:
			RadioButtonEDAccept->Checked = true;
			break;
		case PromptDialogAction::pdaSkip:
			RadioButtonEDSkip->Checked = true;
			break;
		case PromptDialogAction::pdaAbort:
			RadioButtonEDAbort->Checked = true;
			break;
	}

	UpDownSMX->Position = g_pSettingsManager->SMDialogControlPoint.x;
	UpDownSMY->Position = g_pSettingsManager->SMDialogControlPoint.y;
	PanelSMColor->Color = g_pSettingsManager->SMDialogControlPointColor;
	UpDownSMColorTolerance->Position = g_pSettingsManager->SMDialogControlPointColorTolerance;
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

		this->UpDownEDX->Position *= fWidthCoeff;
		this->UpDownEDY->Position *= fHeightCoeff;
		this->UpDownSMX->Position *= fWidthCoeff;
		this->UpDownSMY->Position *= fHeightCoeff;

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
	g_pSettingsManager->ClearOldResults = CheckBoxClearOldResults->Checked;

	g_pSettingsManager->TaskEndBehavior = static_cast<TaskEndAction>(ComboBoxTaskEndAction->ItemIndex);
	g_pSettingsManager->ExitOnTaskEnding = CheckBoxTEAExit->Checked;
	g_pSettingsManager->CloseGameOnTaskEnding = CheckBoxTEACloseTheGame->Checked;

	g_pSettingsManager->TriesBeforeForceTaskEnding = UpDownTriesBeforeFTE->Position;
	g_pSettingsManager->ScreenCheckingPeriod = UpDownScreenCheckingInterval->Position;

	g_pSettingsManager->EnergyDialogControlPoint = TPoint(UpDownEDX->Position, UpDownEDY->Position);
	g_pSettingsManager->EnergyDialogControlPointColor = PanelEDColor->Color;
	g_pSettingsManager->EnergyDialogControlPointColorTolerance = UpDownEDColorTolerance->Position;
	g_pSettingsManager->EnergyDialogGETButtonPoint = TPoint(UpDownEDGETX->Position, UpDownEDGETY->Position);

	if (RadioButtonEDAccept->Checked)
	{
		g_pSettingsManager->EnergyDialogPreferredAction = PromptDialogAction::pdaAccept;
	}
	else if (RadioButtonEDSkip->Checked)
	{
		g_pSettingsManager->EnergyDialogPreferredAction = PromptDialogAction::pdaSkip;
	}
	else if (RadioButtonEDAbort->Checked)
	{
		g_pSettingsManager->EnergyDialogPreferredAction = PromptDialogAction::pdaAbort;
	}

	g_pSettingsManager->SMDialogControlPoint = TPoint(UpDownSMX->Position, UpDownSMY->Position);
	g_pSettingsManager->SMDialogControlPointColor = PanelSMColor->Color;
	g_pSettingsManager->SMDialogControlPointColorTolerance = UpDownSMColorTolerance->Position;

	//Запись в файл
	g_pSettingsManager->UpdateINI();
}
//---------------------------------------------------------------------------
void TFormMain::UpdateApplicationFrame()
{
	//
}
//---------------------------------------------------------------------------
void TFormMain::SaveSettingsFromApplicationFrame()
{
	//
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
	FormPickPoint->OnlyCoordinates = false;

	if (FormPickPoint->Execute(this))
	{
		PickPointData PPResults = FormPickPoint->GetResults();

		UpDownRSX->Position = PPResults.XY.x;
		UpDownRSY->Position = PPResults.XY.y;
		PanelRSColor->Color = PPResults.Color;
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
	strAddress.sprintf(L"mailto:borisvorontsov@yandex.ru?subject=%s вер.%s через обратную связь",
		Application->Title.c_str(), strFullAppVersion.c_str());

	ShellExecute(NULL, NULL, strAddress.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::TrayIconAppBalloonClick(TObject *Sender)
{
	if (g_pSettingsManager->SaveResults)
	{
		this->MenuItemOpenResultsClick(this);
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
	if (this->m_ActiveTaskInfo.CurrentState != TaskState::tsStopped)
	{
		if (MessageBox(this->Handle, L"Текущая задача не завершена. Всё равно выйти?", L"Внимание",
			MB_ICONEXCLAMATION | MB_YESNO | MB_DEFBUTTON2) == IDNO)
		{
			Action = TCloseAction::caNone;
			return;
		}
	}

	this->SaveSettingsFromGMSpecSettingsFrame();
	this->SaveSettingsFromCommonSettingsFrame();

	g_pSettingsManager->StayOnTop = MenuItemStayOnTop->Checked;
	g_pSettingsManager->RecentActivePage = PageControlURAIDASettings->ActivePageIndex;
	g_pSettingsManager->MainWindowPosition = TPoint(this->Left, this->Top);

    g_pSettingsManager->UpdateINI();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PageControlURAIDASettingsChange(TObject *Sender)
{
	if ((PageControlURAIDASettings->ActivePage == TabSheetCampaign) ||
		(PageControlURAIDASettings->ActivePage == TabSheetDungeons) ||
		(PageControlURAIDASettings->ActivePage == TabSheetFactionWars))
	{
		if (ScrollBoxGMSpecSettings->Parent != PageControlURAIDASettings->ActivePage)
			ScrollBoxGMSpecSettings->Parent = PageControlURAIDASettings->ActivePage;
		this->UpdateGMSpecSettingsFrame(PageControlURAIDASettings->ActivePage);
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetCommon)
	{
		this->UpdateCommonSettingsFrame();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PageControlURAIDASettingsChanging(TObject *Sender, bool &AllowChange)
{
	if ((PageControlURAIDASettings->ActivePage == TabSheetCampaign) ||
		(PageControlURAIDASettings->ActivePage == TabSheetDungeons) ||
		(PageControlURAIDASettings->ActivePage == TabSheetFactionWars))
	{
		this->SaveSettingsFromGMSpecSettingsFrame();
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetCommon)
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
	FileOpenDialogGeneric->Title = L"Выберите директорию для сохранения результатов";
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
		MessageBox(this->Handle, L"Путь не найден!", L"Ошибка", MB_ICONSTOP);
		return;
	}

	if (!TDirectory::IsEmpty(g_pSettingsManager->PathForResults))
	{
		if (MessageBox(this->Handle, L"Вы уверены, что хотите удалить все результаты?", L"Предупреждение",
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
			MessageBox(this->Handle, L"Сначала выберите режим игры", L"Предупреждение", MB_ICONEXCLAMATION);
			return;
		}

        //Очищаем папку результатов перед началом выполнения задачи, если указана такая опция
		if (g_pSettingsManager->ClearOldResults)
		{
			if (!g_pSettingsManager->PathForResults.IsEmpty() && DirectoryExists(g_pSettingsManager->PathForResults) &&
				!TDirectory::IsEmpty(g_pSettingsManager->PathForResults))
			{
				TDirectory::Delete(g_pSettingsManager->PathForResults, true);
			}
		}

		//Запускаем таймер задачи по указанным параметрам
		//Параметры будут применены только при старте новой задачи
		m_ActiveTaskInfo.Settings = this->SaveSettingsFromGMSpecSettingsFrame();

		LabelBattlesCounter->Font->Color = clBlack;
		ProgressBarBattle->Position = 0;
		TaskbarApp->ProgressValue = 0;

		m_ActiveTaskInfo.StartTime = Now();
	}

	std::shared_ptr<TBitmap> pGlyph(new TBitmap());
	switch (m_ActiveTaskInfo.CurrentState)
	{
		case TaskState::tsStopped:
		case TaskState::tsPaused:
			m_ActiveTaskInfo.CurrentState = TaskState::tsRunning;
			BitBtnRunTask->Caption = m_strButtonRTPauseCaption;
			ImageListRTButton->GetBitmap(1, pGlyph.get());
			BitBtnRunTask->Glyph = pGlyph.get();
			ProgressBarBattle->State = TProgressBarState::pbsNormal;
			TaskbarApp->ProgressState = TTaskBarProgressState::Normal;

			TimerMain->Enabled = true;
			break;
		case TaskState::tsRunning:
			m_ActiveTaskInfo.CurrentState = TaskState::tsPaused;
			BitBtnRunTask->Caption = m_strButtonRTResumeCaption;
			ImageListRTButton->GetBitmap(2, pGlyph.get());
			BitBtnRunTask->Glyph = pGlyph.get();
			ProgressBarBattle->State = TProgressBarState::pbsPaused;
			TaskbarApp->ProgressState = TTaskBarProgressState::Paused;

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
    ProgressBarBattle->Style = TProgressBarStyle::pbstNormal;
	ProgressBarBattle->Position = 100;
	TaskbarApp->ProgressValue = 100;

	m_ActiveTaskInfo.CurrentState = TaskState::tsStopped;
	BitBtnRunTask->Caption = m_strButtonRTRunCaption;
	std::shared_ptr<TBitmap> pGlyph(new TBitmap());
	ImageListRTButton->GetBitmap(0, pGlyph.get());
	BitBtnRunTask->Glyph = pGlyph.get();

	TrayIconApp->BalloonHint = L"";
	TrayIconApp->Hint = Application->Title;

	switch (Reason)
	{
		case TaskStoppingReason::tsrUser:
		{
			LabelBattlesCounter->Font->Color = clGray;
			ProgressBarBattle->State = TProgressBarState::pbsNormal;
			TaskbarApp->ProgressState = TTaskBarProgressState::Normal;
			break;
		}
		case TaskStoppingReason::tsrSuccessfulCompletion:
		{
			LabelBattlesCounter->Font->Color = clGreen;
			ProgressBarBattle->State = TProgressBarState::pbsNormal;
			TaskbarApp->ProgressState = TTaskBarProgressState::Normal;
			break;
		}
		case TaskStoppingReason::tsrError:
		{
			LabelBattlesCounter->Font->Color = clRed;
			ProgressBarBattle->State = TProgressBarState::pbsError;
			TaskbarApp->ProgressState = TTaskBarProgressState::Error;
			break;
		}
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
					TrayIconApp->BalloonHint = L"Задача успешно завершена";
					TrayIconApp->BalloonFlags = TBalloonFlags::bfInfo;
				}
				else if (Reason == TaskStoppingReason::tsrError)
				{
					TrayIconApp->BalloonHint = L"Принудительное завершение задачи из-за ошибок";
                    TrayIconApp->BalloonFlags = TBalloonFlags::bfError;
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
void TFormMain::SaveResult(unsigned int nBattleNumber, bool bError)
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
			strGameMode = L"Кампания";
			break;
		case SupportedGameModes::gmDungeons:
			strGameMode = L"Подземелья";
			break;
		case SupportedGameModes::gmFactionWars:
			strGameMode = L"ВойныФракций";
			break;
	}
	strActiveTaskSubDir.sprintf(L"%s_%s", strGameMode.c_str(),
		FormatDateTime(L"dd_mmmm_yyyy_hh-mm", m_ActiveTaskInfo.StartTime).c_str());

	//Формируем имя файла JPEG
	if (!bError)
	{
		strFileName.sprintf(L"Бой%i_%s.jpeg", nBattleNumber, FormatDateTime(L"hh-mm", Now()).c_str());
	}
	else
	{
		//Если bError, записываем как причину аварийной остановки задачи
		strFileName.sprintf(L"Ошибка_%s.jpeg", FormatDateTime(L"hh-mm", Now()).c_str());
	}

	strFinalPath = IncludeTrailingPathDelimiter(strPathForResults) + strActiveTaskSubDir;
	CreateDir(strFinalPath);

	std::shared_ptr<TImage> pImage(new TImage(this));
	std::shared_ptr<TJPEGImage> pJPEGImage(new TJPEGImage());

	TRect FrameSize = g_pRAIDWorker->GetGameWindowSize(true);
	pImage->Width = FrameSize.Right;
	pImage->Height = FrameSize.Bottom;
	g_pRAIDWorker->CaptureFrame(pImage->Canvas, TSize(FrameSize.Right, FrameSize.Bottom));

	if (bError)
	{
		//Дополнительно указываем на скриншоте координату проверки в виде красного крестика
		const unsigned int uMarkerIndent = 10;
		TPoint ControlPoint;
		if (nBattleNumber > 1)
		{
			ControlPoint = this->m_ActiveTaskInfo.Settings.REPLAYScreenControlPoint.Coordinates;
		}
		else
		{
			ControlPoint = this->m_ActiveTaskInfo.Settings.STARTScreenControlPoint.Coordinates;
		}

		pImage->Canvas->Pen->Color = clRed;
		pImage->Canvas->Pen->Width = 3;
		pImage->Canvas->MoveTo(ControlPoint.x - uMarkerIndent , ControlPoint.y);
		pImage->Canvas->LineTo(ControlPoint.x + uMarkerIndent, ControlPoint.y);
		pImage->Canvas->MoveTo(ControlPoint.x, ControlPoint.y - uMarkerIndent);
		pImage->Canvas->LineTo(ControlPoint.x, ControlPoint.y + uMarkerIndent);
	}

	pJPEGImage->Assign(pImage->Picture->Bitmap);
	pJPEGImage->SaveToFile(IncludeTrailingPathDelimiter(strFinalPath) + strFileName);
}

//---------------------------------------------------------------------------

void __fastcall TFormMain::CheckBoxSaveResultsClick(TObject *Sender)
{
	MenuItemOpenResults->Enabled = CheckBoxSaveResults->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnEDPickPointClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = false;

	if (FormPickPoint->Execute(this))
	{
		PickPointData PPResults = FormPickPoint->GetResults();

		UpDownEDX->Position = PPResults.XY.x;
		UpDownEDY->Position = PPResults.XY.y;
		PanelEDColor->Color = PPResults.Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnEDGETPickPointClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = true;

	if (FormPickPoint->Execute(this))
	{
		PickPointData PPResults = FormPickPoint->GetResults();

		UpDownEDGETX->Position = PPResults.XY.x;
		UpDownEDGETY->Position = PPResults.XY.y;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnSMPickColorClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = false;

	if (FormPickPoint->Execute(this))
	{
		PickPointData PPResults = FormPickPoint->GetResults();

		UpDownSMX->Position = PPResults.XY.x;
		UpDownSMY->Position = PPResults.XY.y;
		PanelSMColor->Color = PPResults.Color;
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

void __fastcall TFormMain::LinkLabelReleasesClick(TObject *Sender)
{
	ShellExecute(NULL, L"OPEN", L"https://github.com/BorisVorontsov/URAIDA/releases/", NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::MenuItemStayOnTopClick(TObject *Sender)
{
	MenuItemStayOnTop->Checked = !MenuItemStayOnTop->Checked;
	this->FormStyle = (MenuItemStayOnTop->Checked)?TFormStyle::fsStayOnTop:TFormStyle::fsNormal;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::MenuItemMoveToCenterClick(TObject *Sender)
{
	this->Left = (Screen->Width - this->Width) / 2;
	this->Top = (Screen->Height - this->Height) / 2;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnRunTaskClick(TObject *Sender)
{
	m_bForceStopTask = false;
	this->StartTask();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnStopTaskClick(TObject *Sender)
{
	m_bForceStopTask = true;
	if (m_ActiveTaskInfo.CurrentState == TaskState::tsPaused)
		this->TimerMainTimer(this);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PopupMenuTrayPopup(TObject *Sender)
{
    MenuItemMoveToCenter->Enabled = this->Visible;
}
//---------------------------------------------------------------------------

