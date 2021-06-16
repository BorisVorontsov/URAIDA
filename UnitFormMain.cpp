//---------------------------------------------------------------------------

#include "URAIDAPCH.h"
#include <Mmsystem.h>
#include <powrprof.h>
#include <Jpeg.hpp>
#include <IOUtils.hpp>
#include <memory>
#include <array>
#pragma hdrstop

#include "UnitFormMain.h"
#include "UnitFormPickPoint.h"
#include "UnitFormCalculations.h"
#include "UnitCommon.h"
#include "UnitLogManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormMain *FormMain;

TSettingsManager *g_pSettingsManager;
TRAIDWorker *g_pRAIDWorker;
TLogManager *g_pLogManager;

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
		uBattleDelayInSeconds = (HIWORD(m_ActiveTaskInfo.Settings.Delay) * 60) + LOWORD(m_ActiveTaskInfo.Settings.Delay);

		if (!uBattleDelayInSeconds)
		{
			this->StopTask(TaskStoppingReason::tsrError);
			MessageBox(this->Handle, L"Время боя должно быть больше нуля!", L"Ошибка", MB_ICONSTOP);

			return;
        }
	}

	if ((nCycleCounter > m_ActiveTaskInfo.Settings.NumberOfBattles) && bScreenCheckPassed)
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
		if (nCycleCounter <= m_ActiveTaskInfo.Settings.NumberOfBattles)
		{
			uBattleTimeout = 0;
			LabelBattlesCounter->Caption = String(nCycleCounter) + L"/" +
				String(m_ActiveTaskInfo.Settings.NumberOfBattles);

			String strTrayHint;
			strTrayHint.sprintf(L"%s (%i/%i)", Application->Title.c_str(), nCycleCounter,
				m_ActiveTaskInfo.Settings.NumberOfBattles);
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
		g_pRAIDWorker->ValidateGameWindow();

		uScreenCheckingTiomeout = 0;

		//Цикл сравнения контрольных точек с кадром
		auto CompareControlPoints
		{
			[](std::array<TControlPoint, g_uMaxControlPoints>& ControlPoints)
			{
				unsigned int nEnabledCPs = 0;

				//Захватываем текущий кадр, над которым далее будет проводиться анализ
				if (!g_pRAIDWorker->CaptureFrame())
					return false;

				for (auto& ControlPoint : ControlPoints)
				{
					if (ControlPoint.Enabled)
					{
						nEnabledCPs++;
						if (!g_pRAIDWorker->ComparePixels(ControlPoint.Coordinates, ControlPoint.PixelColor,
							ControlPoint.Tolerance))
						{
							if (g_pSettingsManager->EnableLogging)
							{
								g_pLogManager->Append(L"Сравнение КТ провалено ( X: %i, Y: %i Цвет: %i Погрешность: %i )",
									ControlPoint.Coordinates.x, ControlPoint.Coordinates.y, ControlPoint.PixelColor,
									ControlPoint.Tolerance);
							}
							return false;
						}
					}
				}

				if (!nEnabledCPs)
				{
					if (g_pSettingsManager->EnableLogging)
						g_pLogManager->Append(L"Активных КТ не обнаружено, пропуск");

					return false;
				}

				if (g_pSettingsManager->EnableLogging)
					g_pLogManager->Append(L"Сравнение всех активных КТ завершено успешно");
				return true;
			}
		};

		//Прерыватель задачи: диалог пополнения энергии
		//Появляется при её нехватке после попытки запуска боя
		auto EnergyDialogTest
		{
			[=](bool& bExitTimer)
			{
				if (g_pSettingsManager->EnableLogging)
					g_pLogManager->Append(L"Цикл %i: начало тестирования КТ диалога пополнения энергии", nCycleCounter);

				if (CompareControlPoints(g_pSettingsManager->EnergyDialogControlPoints))
				{
					if (g_pSettingsManager->EnableLogging)
						g_pLogManager->Append(L"Выполнение действий для диалога пополнения энергии");

					switch (g_pSettingsManager->EnergyDialogPreferredAction)
					{
						case PromptDialogAction::pdaAccept:
							//Кликаем, куда задал пользователь (предполагается кнопка "Получить")
							g_pRAIDWorker->SendMouseClick(g_pSettingsManager->EnergyDialogGETButtonPoint);
							bExitTimer = false;
							break;
						case PromptDialogAction::pdaSkip:
							g_pRAIDWorker->SendMouseClick(TPoint(1, 1));
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

					return true;
				}

				return false;
			}
		};

		//Возможное обнаружение диалога пополнения энергии
		bool bExitTimer;
		if (EnergyDialogTest(bExitTimer))
		{
			if (bExitTimer)
				return;
		}

		//Прерыватель задачи: диалог работ на сервере
		if (g_pSettingsManager->EnableLogging)
			g_pLogManager->Append(L"Цикл %i: начало тестирования КТ диалога работ на сервере", nCycleCounter);

		if (CompareControlPoints(g_pSettingsManager->SMDialogControlPoints))
		{
			if (g_pSettingsManager->EnableLogging)
				g_pLogManager->Append(L"Выполнение действий для диалога работ на сервере");

			//Просто игнорируем
			g_pRAIDWorker->SendMouseClick(TPoint(1, 1));
		}

		auto DoOnSuccessOperations
		{
			[=]()
			{
				LabelBattlesCounter->Font->Color = clBlack;
				ProgressBarBattle->Style = TProgressBarStyle::pbstNormal;
				bScreenCheckPassed = true;
			}
		};

		//Если задан параметр "STARTScreenFirst", начинам с анализа на экран начала боя
		if ((nCycleCounter == 1) && m_ActiveTaskInfo.Settings.ProcessSTARTScreenFirst)
		{
			if (g_pSettingsManager->EnableLogging)
				g_pLogManager->Append(L"Цикл %i: начало тестирования КТ экрана НАЧАТЬ", nCycleCounter);

			if (CompareControlPoints(m_ActiveTaskInfo.Settings.STARTScreenControlPoints))
			{
				if (g_pSettingsManager->EnableLogging)
					g_pLogManager->Append(L"Выполнение действий для экрана НАЧАТЬ");

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
					DoOnSuccessOperations();
                }
			}
		}
		else //Если параметр поиска экрана начала боя не задан, или мы уже на втором или более цикле, переходим к анализу экрана "Повтор/Далее"
		{
			if (g_pSettingsManager->EnableLogging)
				g_pLogManager->Append(L"Цикл %i: начало тестирования КТ экрана ПОВТОР/ДАЛЕЕ", nCycleCounter);

			if (CompareControlPoints(m_ActiveTaskInfo.Settings.REPLAYScreenControlPoints))
			{
				//Отчёт, если задано создание скриншота по завершению каждого боя
				if ((nCycleCounter > 1) && g_pSettingsManager->SaveResults &&
					(g_pSettingsManager->ResultSavingMethod == ResultSavingMode::rsmAtTheEndOfEachBattle))
				{
					this->SaveResult(nCycleCounter - 1);
				}

				//Если мы прошли последний бой (счётчик nCycleCounter будет равен nNumberOfBattles + 1),
				//просто ждём экрана результатов
				if (nCycleCounter <= m_ActiveTaskInfo.Settings.NumberOfBattles)
				{
					if (g_pSettingsManager->EnableLogging)
						g_pLogManager->Append(L"Выполнение действий для экрана ПОВТОР/ДАЛЕЕ");

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
						DoOnSuccessOperations();
					}
				}
				else
                    bScreenCheckPassed = true;
			}
			else
				nScreenCheckFailures++;
		}

		//Если превысили число ошибочных попыток сравнения, завершаем всю задачу
		if (nScreenCheckFailures == g_pSettingsManager->TriesBeforeForceTaskEnding)
		{
			if (g_pSettingsManager->EnableLogging)
			{
				g_pLogManager->Append(L"Превышен лимит ошибочных тестирований экрана: %i/%i",
					nScreenCheckFailures, g_pSettingsManager->TriesBeforeForceTaskEnding);
            }

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

	auto FillButtonGroupControl
	{
		[this](TButtonGroup* pControl)
		{
			TGrpButtonItem *pNewItem;
			for (int i = 0; i < g_uMaxControlPoints; i++)
			{
				pNewItem = pControl->Items->Add();
				pNewItem->Index = i;
				pNewItem->Caption = IntToStr(i + 1);
			}
		}
	};

	FillButtonGroupControl(ButtonGroupSSCPIndex);
	FillButtonGroupControl(ButtonGroupRSCPIndex);
	FillButtonGroupControl(ButtonGroupEDCPIndex);
	FillButtonGroupControl(ButtonGroupSMDCPIndex);

	g_pRAIDWorker = new TRAIDWorker;

	PageControlURAIDASettings->ActivePageIndex = g_pSettingsManager->RecentActivePage;
	this->UpdateNecessarySettings();
	BitBtnRunTask->Caption = m_strButtonRTRunCaption;
	std::unique_ptr<TBitmap> pGlyph(new TBitmap());
	ImageListRTButton->GetBitmap(0, pGlyph.get());
	BitBtnRunTask->Glyph = pGlyph.get();

	//Вывод названия и версии программы
	String strAppVersion;
	GetFileVersion(Application->ExeName, strAppVersion, fvfMajorMinor);
	LabelCopyright1->Caption = Application->Title + L" вер." + strAppVersion + L" от";

	g_pLogManager = new TLogManager;
	if (g_pSettingsManager->EnableLogging)
	{
		String strLogFile, strExeName = Application->ExeName;
		strLogFile = IncludeTrailingPathDelimiter(ExtractFilePath(strExeName)) +
			TPath::GetFileNameWithoutExtension(strExeName) + L".log";
		g_pLogManager->LogFile = strLogFile;
		g_pLogManager->OpenLog();
		g_pLogManager->MaximumEntries = g_pSettingsManager->MaxLogEntries;
        MenuItemOpenLogFile->Visible = true;
    }
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnSSPickPointClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = false;

	if (FormPickPoint->Execute(this))
	{
		PickPointData Results = FormPickPoint->GetResults();

		UpDownSSX->Position = Results.XY.x;
		UpDownSSY->Position = Results.XY.y;
		PanelSSColor->Color = Results.Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonApplyGWSizeClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;

	g_pRAIDWorker->ResizeGameWindow(TSize(UpDownGWWidth->Position, UpDownGWHeight->Position));
}
//---------------------------------------------------------------------------
void TFormMain::GetAppropriateGMSpecSettings(TGameModeSpecSettings& Result)
{
	if (PageControlURAIDASettings->ActivePage == TabSheetCampaign)
	{
		Result = g_pSettingsManager->CampaignSettings;
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetDungeons)
	{
		Result = g_pSettingsManager->DungeonsSettings;
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetFactionWars)
	{
		Result = g_pSettingsManager->FactionWarsSettings;
	}
}
//---------------------------------------------------------------------------
void TFormMain::ApplyAppropriateGMSpecSettings(const TGameModeSpecSettings& Input)
{
	if (PageControlURAIDASettings->ActivePage == TabSheetCampaign)
	{
		g_pSettingsManager->CampaignSettings = Input;
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetDungeons)
	{
		g_pSettingsManager->DungeonsSettings = Input;
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetFactionWars)
	{
		g_pSettingsManager->FactionWarsSettings = Input;
	}
}
//---------------------------------------------------------------------------
void TFormMain::UpdateGMSpecSettingsFrame()
{
	//Чтение настроек

	TGameModeSpecSettings GMSpecSettings;
	this->GetAppropriateGMSpecSettings(GMSpecSettings);

	CheckBoxProcessSTARTScreen->Checked = GMSpecSettings.ProcessSTARTScreenFirst;
	ButtonGroupSSCPIndex->ItemIndex = GMSpecSettings.STARTScreenControlPointIndex;
	CheckBoxSSCPState->Checked = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Enabled;
	UpDownSSX->Position = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Coordinates.x;
	UpDownSSY->Position = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Coordinates.y;
	PanelSSColor->Color = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].PixelColor;
	UpDownSSColorTolerance->Position = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Tolerance;
	ButtonGroupRSCPIndex->ItemIndex = GMSpecSettings.REPLAYScreenControlPointIndex;
	CheckBoxRSCPState->Checked = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Enabled;
	UpDownRSX->Position = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Coordinates.x;
	UpDownRSY->Position = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Coordinates.y;
	PanelRSColor->Color = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].PixelColor;
	UpDownRSColorTolerance->Position = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Tolerance;

	switch (GMSpecSettings.REPLAYScreenPreferredAction)
	{
		case REPLAYScreenAction::rsaReplay:
			RadioButtonRSActionReplay->Checked = true;
			break;
		case REPLAYScreenAction::rsaGoNext:
			RadioButtonRSActionNext->Checked = true;
            break;
	}

	UpDownBTMinutes->Position = HIWORD(GMSpecSettings.Delay);
	UpDownBTSeconds->Position = LOWORD(GMSpecSettings.Delay);
	UpDownNumberOfBattles->Position = GMSpecSettings.NumberOfBattles;
}
//---------------------------------------------------------------------------
void TFormMain::SaveSettingsFromGMSpecSettingsFrame()
{
	TGameModeSpecSettings GMSpecSettings;
	this->GetAppropriateGMSpecSettings(GMSpecSettings);

	GMSpecSettings.ProcessSTARTScreenFirst = CheckBoxProcessSTARTScreen->Checked;
	GMSpecSettings.STARTScreenControlPointIndex = ButtonGroupSSCPIndex->ItemIndex;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Enabled = CheckBoxSSCPState->Checked;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Coordinates =
		TPoint(UpDownSSX->Position, UpDownSSY->Position);
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].PixelColor = PanelSSColor->Color;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Tolerance = UpDownSSColorTolerance->Position;
	GMSpecSettings.REPLAYScreenControlPointIndex = ButtonGroupRSCPIndex->ItemIndex;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Enabled = CheckBoxRSCPState->Checked;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Coordinates =
		TPoint(UpDownRSX->Position, UpDownRSY->Position);
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].PixelColor = PanelRSColor->Color;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Tolerance = UpDownRSColorTolerance->Position;

	if (RadioButtonRSActionReplay->Checked)
	{
		GMSpecSettings.REPLAYScreenPreferredAction = REPLAYScreenAction::rsaReplay;
	}
	else if (RadioButtonRSActionNext->Checked)
	{
		GMSpecSettings.REPLAYScreenPreferredAction = REPLAYScreenAction::rsaGoNext;
	}

	GMSpecSettings.Delay = MAKELONG(UpDownBTSeconds->Position, UpDownBTMinutes->Position);
	GMSpecSettings.NumberOfBattles = UpDownNumberOfBattles->Position;

	this->ApplyAppropriateGMSpecSettings(GMSpecSettings);
}
//---------------------------------------------------------------------------
void TFormMain::UpdateCommonSettingsFrame()
{
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

	ButtonGroupEDCPIndex->ItemIndex = g_pSettingsManager->EnergyDialogControlPointIndex;
	CheckBoxEDCPState->Checked = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Enabled;
	UpDownEDX->Position = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Coordinates.x;
	UpDownEDY->Position = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Coordinates.y;
	PanelEDColor->Color = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].PixelColor;
	UpDownEDColorTolerance->Position = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Tolerance;

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

	ButtonGroupSMDCPIndex->ItemIndex = g_pSettingsManager->SMDialogControlPointIndex;
	CheckBoxSMDCPState->Checked = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Enabled;
	UpDownSMX->Position = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Coordinates.x;
	UpDownSMY->Position = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Coordinates.y;
	PanelSMColor->Color = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].PixelColor;
	UpDownSMColorTolerance->Position = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Tolerance;
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
		TGameModeSpecSettings GMSpecSettings;

		fWidthCoeff = static_cast<float>(NewGWSize.cx) / static_cast<float>(CurrentGWSize.cx);
		fHeightCoeff = static_cast<float>(NewGWSize.cy) / static_cast<float>(CurrentGWSize.cy);

		auto GMSSShiftCoordinates
		{
			[fWidthCoeff, fHeightCoeff, &GMSpecSettings]()
			{
				for (auto& ControlPoint : GMSpecSettings.STARTScreenControlPoints)
				{
					ControlPoint.ShiftCoordinates(fWidthCoeff, fHeightCoeff);
				}
				for (auto& ControlPoint : GMSpecSettings.REPLAYScreenControlPoints)
				{
					ControlPoint.ShiftCoordinates(fWidthCoeff, fHeightCoeff);
				}
			}
		};

		GMSpecSettings = g_pSettingsManager->CampaignSettings;
		GMSSShiftCoordinates();
		g_pSettingsManager->CampaignSettings = GMSpecSettings;
		GMSpecSettings = g_pSettingsManager->DungeonsSettings;
		GMSSShiftCoordinates();
		g_pSettingsManager->DungeonsSettings = GMSpecSettings;
		GMSpecSettings = g_pSettingsManager->FactionWarsSettings;
		GMSSShiftCoordinates();
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

	g_pSettingsManager->EnergyDialogControlPointIndex = ButtonGroupEDCPIndex->ItemIndex;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Enabled = CheckBoxEDCPState->Checked;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Coordinates =
		TPoint(UpDownEDX->Position, UpDownEDY->Position);
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].PixelColor = PanelEDColor->Color;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Tolerance = UpDownEDColorTolerance->Position;
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

	g_pSettingsManager->SMDialogControlPointIndex = ButtonGroupSMDCPIndex->ItemIndex;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Enabled = CheckBoxSMDCPState->Checked;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Coordinates =
		TPoint(UpDownSMX->Position, UpDownSMY->Position);
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].PixelColor = PanelSMColor->Color;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Tolerance = UpDownSMColorTolerance->Position;
}
//---------------------------------------------------------------------------
void TFormMain::UpdateNecessarySettings()
{
	if ((PageControlURAIDASettings->ActivePage == TabSheetCampaign) ||
		(PageControlURAIDASettings->ActivePage == TabSheetDungeons) ||
		(PageControlURAIDASettings->ActivePage == TabSheetFactionWars))
	{
		if (ScrollBoxGMSpecSettings->Parent != PageControlURAIDASettings->ActivePage)
			ScrollBoxGMSpecSettings->Parent = PageControlURAIDASettings->ActivePage;
		this->UpdateGMSpecSettingsFrame();
	}
	else if (PageControlURAIDASettings->ActivePage == TabSheetCommon)
	{
		this->UpdateCommonSettingsFrame();
	}
}
//---------------------------------------------------------------------------
void TFormMain::SaveNecessarySettings()
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
void __fastcall TFormMain::ButtonUseCurrentGWSizeClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;

    TRect GameWindowSize;
	if (g_pRAIDWorker->GetGameWindowSize(GameWindowSize, true))
	{
		UpDownGWWidth->Position = GameWindowSize.Width();
		UpDownGWHeight->Position = GameWindowSize.Height();
    }
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnRSPickPointClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = false;

	if (FormPickPoint->Execute(this))
	{
		PickPointData Results = FormPickPoint->GetResults();

		UpDownRSX->Position = Results.XY.x;
		UpDownRSY->Position = Results.XY.y;
		PanelRSColor->Color = Results.Color;
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
	ShellExecute(NULL, L"OPEN", L"https://discord.gg/EJs4gRSW4G", NULL, NULL, SW_SHOWNORMAL);
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

	this->SaveNecessarySettings();

	g_pSettingsManager->StayOnTop = MenuItemStayOnTop->Checked;
	g_pSettingsManager->RecentActivePage = PageControlURAIDASettings->ActivePageIndex;
	g_pSettingsManager->MainWindowPosition = TPoint(this->Left, this->Top);

	g_pSettingsManager->UpdateINI();

	if (g_pSettingsManager->EnableLogging)
		g_pLogManager->CloseLog();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PageControlURAIDASettingsChange(TObject *Sender)
{
	this->UpdateNecessarySettings();
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::PageControlURAIDASettingsChanging(TObject *Sender, bool &AllowChange)
{
	this->SaveNecessarySettings();
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
bool TFormMain::CheckActivePageWithMessage()
{
	if (PageControlURAIDASettings->ActivePage == TabSheetCommon)
	{
		MessageBox(this->Handle, L"Сначала выберите режим игры", L"Предупреждение", MB_ICONEXCLAMATION);
		return false;
	}

	return true;
}
//---------------------------------------------------------------------------
void TFormMain::StartTask()
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;

	if (m_ActiveTaskInfo.CurrentState == TaskState::tsStopped)
	{
		if (!this->CheckActivePageWithMessage())
            return;

		//Запускаем таймер задачи по указанным параметрам
		//Параметры будут применены только при старте новой задачи
		this->SaveSettingsFromGMSpecSettingsFrame();
		TGameModeSpecSettings GMSpecSettings;
        this->GetAppropriateGMSpecSettings(GMSpecSettings);

		//Проверяем, не забыл ли пользователь включить хотя бы одну контрольную точку
		unsigned int nCPCount = 0;
		if (GMSpecSettings.ProcessSTARTScreenFirst)
		{
			for (auto& ControlPoint : GMSpecSettings.STARTScreenControlPoints)
			{
				if (ControlPoint.Enabled)
					nCPCount++;
			}
			if (!nCPCount)
			{
				MessageBox(this->Handle, L"Включите хотя бы одну контрольную точку экрана НАЧАТЬ!", L"Предупреждение",
					MB_ICONEXCLAMATION);
				return;
			}
		}
        nCPCount = 0;
		if ((GMSpecSettings.ProcessSTARTScreenFirst && GMSpecSettings.NumberOfBattles > 1) ||
			!GMSpecSettings.ProcessSTARTScreenFirst)
		{
			for (auto& ControlPoint : GMSpecSettings.REPLAYScreenControlPoints)
			{
				if (ControlPoint.Enabled)
					nCPCount++;
			}
			if (!nCPCount)
			{
				MessageBox(this->Handle, L"Включите хотя бы одну контрольную точку экрана ПОВТОР/ДАЛЕЕ!", L"Предупреждение",
					MB_ICONEXCLAMATION);
				return;
			}
		}

		m_ActiveTaskInfo.Settings = GMSpecSettings;

        //Очищаем папку результатов перед началом выполнения задачи, если указана такая опция
		if (g_pSettingsManager->ClearOldResults)
		{
			if (!g_pSettingsManager->PathForResults.IsEmpty() && DirectoryExists(g_pSettingsManager->PathForResults) &&
				!TDirectory::IsEmpty(g_pSettingsManager->PathForResults))
			{
				TDirectory::Delete(g_pSettingsManager->PathForResults, true);
			}
		};

		LabelBattlesCounter->Font->Color = clBlack;
		ProgressBarBattle->Position = 0;
		TaskbarApp->ProgressValue = 0;

		m_ActiveTaskInfo.StartTime = Now();

		if (g_pSettingsManager->EnableLogging)
		{
			String strPresetName;
			switch (m_ActiveTaskInfo.Settings.GameMode)
			{
				case SupportedGameModes::gmCampaign:
					strPresetName = L"::gmCampaign";
					break;
				case SupportedGameModes::gmDungeons:
					strPresetName = L"::gmDungeons";
					break;
				case SupportedGameModes::gmFactionWars:
					strPresetName = L"::gmFactionWars";
					break;
			}

			g_pLogManager->Append(L"Новая задача: %s", strPresetName.c_str());
		}
	}

	std::unique_ptr<TBitmap> pGlyph(new TBitmap());
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

	//Блокируем переход системы в экономичный режим и отключение экрана для корректной работы
	//автоматизатора и игры
	SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_DISPLAY_REQUIRED);

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
	std::unique_ptr<TBitmap> pGlyph(new TBitmap());
	ImageListRTButton->GetBitmap(0, pGlyph.get());
	BitBtnRunTask->Glyph = pGlyph.get();

	TrayIconApp->BalloonHint = L"";
	TrayIconApp->Hint = Application->Title;

	String strReason;
	switch (Reason)
	{
		case TaskStoppingReason::tsrUser:
		{
			LabelBattlesCounter->Font->Color = clGray;
			ProgressBarBattle->State = TProgressBarState::pbsNormal;
			TaskbarApp->ProgressState = TTaskBarProgressState::Normal;
			strReason = L"::tsrUser";
			break;
		}
		case TaskStoppingReason::tsrSuccessfulCompletion:
		{
			LabelBattlesCounter->Font->Color = clGreen;
			ProgressBarBattle->State = TProgressBarState::pbsNormal;
			TaskbarApp->ProgressState = TTaskBarProgressState::Normal;
			strReason = L"::tsrSuccessfulCompletion";
			break;
		}
		case TaskStoppingReason::tsrError:
		{
			LabelBattlesCounter->Font->Color = clRed;
			ProgressBarBattle->State = TProgressBarState::pbsError;
			TaskbarApp->ProgressState = TTaskBarProgressState::Error;
			strReason = L"::tsrError";
			break;
		}
	}

	if (g_pSettingsManager->EnableLogging)
	{
		g_pLogManager->Append(L"Остановка задачи, причина: %s", strReason.c_str());
		g_pLogManager->FlushToDisk();
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

	//Снимаем блокировку
	SetThreadExecutionState(ES_CONTINUOUS);

	PageControlURAIDASettings->Visible = true;
}
//---------------------------------------------------------------------------
void TFormMain::SaveResult(unsigned int nBattleNumber, bool bError)
{
	if (m_ActiveTaskInfo.CurrentState == TaskState::tsStopped)
		return;

	if (g_pSettingsManager->EnableLogging)
	{
		g_pLogManager->Append(L"Сохранение прогресса боя %i (%s)", nBattleNumber,
			(bError)?L"ошибка тестирования":L"кадр");
	}

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

	strFinalPath = IncludeTrailingPathDelimiter(strPathForResults) + strActiveTaskSubDir;
	CreateDir(strFinalPath);

	std::unique_ptr<TImage> pImage(new TImage(this));

	TSize FrameSize;
	g_pRAIDWorker->GetFrameSize(FrameSize);
	pImage->Width = FrameSize.cx;
	pImage->Height = FrameSize.cy;
	g_pRAIDWorker->DrawFrame(pImage->Canvas, FrameSize);

	//Если bError, записываем как причину аварийной остановки задачи
	if (bError)
	{
		std::array<TControlPoint, g_uMaxControlPoints> ControlPoints;
		if ((nBattleNumber == 1) && this->m_ActiveTaskInfo.Settings.ProcessSTARTScreenFirst)
		{
			ControlPoints = this->m_ActiveTaskInfo.Settings.STARTScreenControlPoints;
		}
		else
		{
			ControlPoints = this->m_ActiveTaskInfo.Settings.REPLAYScreenControlPoints;
		}
		for (auto& ControlPoint: ControlPoints)
		{
			if (ControlPoint.Enabled)
			{
				//Дополнительно указываем на скриншоте координату проверки в виде крестика
				const unsigned int uMarkerIndent = 10;

				auto DrawCrosshair
				{
					[&pImage, ControlPoint, uMarkerIndent]()
					{
						pImage->Canvas->MoveTo(ControlPoint.Coordinates.x - uMarkerIndent , ControlPoint.Coordinates.y);
						pImage->Canvas->LineTo(ControlPoint.Coordinates.x + uMarkerIndent, ControlPoint.Coordinates.y);
						pImage->Canvas->MoveTo(ControlPoint.Coordinates.x, ControlPoint.Coordinates.y - uMarkerIndent);
						pImage->Canvas->LineTo(ControlPoint.Coordinates.x, ControlPoint.Coordinates.y + uMarkerIndent);
					}
				};

				pImage->Canvas->Pen->Color = clRed;
				pImage->Canvas->Pen->Width = 5;
				DrawCrosshair();

				pImage->Canvas->Pen->Color = ControlPoint.PixelColor;
				pImage->Canvas->Pen->Width = 3;
				DrawCrosshair();
			}
		}

		//Сохраняем в формате PNG для возможности анализа цветов кадра
		std::unique_ptr<TPngImage> pPngImage(new TPngImage());
		pPngImage->Assign(pImage->Picture->Bitmap);

		strFileName.sprintf(L"Ошибка_%s.png", FormatDateTime(L"hh-mm", Now()).c_str());
		pPngImage->SaveToFile(IncludeTrailingPathDelimiter(strFinalPath) + strFileName);
	}
	else
	{
		//Экономим место форматом JPEG
		std::unique_ptr<TJPEGImage> pJPEGImage(new TJPEGImage());
		pJPEGImage->Assign(pImage->Picture->Bitmap);

		strFileName.sprintf(L"Бой%i_%s.jpeg", nBattleNumber, FormatDateTime(L"hh-mm", Now()).c_str());
		pJPEGImage->SaveToFile(IncludeTrailingPathDelimiter(strFinalPath) + strFileName);
	}
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
		PickPointData Results = FormPickPoint->GetResults();

		UpDownEDX->Position = Results.XY.x;
		UpDownEDY->Position = Results.XY.y;
		PanelEDColor->Color = Results.Color;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnEDGETPickPointClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = true;

	if (FormPickPoint->Execute(this))
	{
		PickPointData Results = FormPickPoint->GetResults();

		UpDownEDGETX->Position = Results.XY.x;
		UpDownEDGETY->Position = Results.XY.y;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnSMPickColorClick(TObject *Sender)
{
	FormPickPoint->OnlyCoordinates = false;

	if (FormPickPoint->Execute(this))
	{
		PickPointData Results = FormPickPoint->GetResults();

		UpDownSMX->Position = Results.XY.x;
		UpDownSMY->Position = Results.XY.y;
		PanelSMColor->Color = Results.Color;
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
	MenuItemOpenLogFile->Enabled = TFile::Exists(g_pLogManager->LogFile);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonGroupSSCPIndexButtonClicked(TObject *Sender, int Index)
{
	if (ButtonGroupSSCPIndex->ItemIndex == Index) return;

	TGameModeSpecSettings GMSpecSettings;
	this->GetAppropriateGMSpecSettings(GMSpecSettings);

	//Сохраняем параметры контрольной точки по старому индексу
	GMSpecSettings.STARTScreenControlPointIndex = ButtonGroupSSCPIndex->ItemIndex;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Enabled = CheckBoxSSCPState->Checked;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Coordinates =
		TPoint(UpDownSSX->Position, UpDownSSY->Position);
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].PixelColor = PanelSSColor->Color;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Tolerance = UpDownSSColorTolerance->Position;

	this->ApplyAppropriateGMSpecSettings(GMSpecSettings);

	//Обновляем индекс и вычитываем параметры по выбранному индексу
	GMSpecSettings.STARTScreenControlPointIndex = Index;

	CheckBoxSSCPState->Checked = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Enabled;
	UpDownSSX->Position = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Coordinates.x;
	UpDownSSY->Position = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Coordinates.y;
	PanelSSColor->Color = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].PixelColor;
	UpDownSSColorTolerance->Position = GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Tolerance;

	//ButtonGroupSSCPIndex->ItemIndex = Index;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonGroupRSCPIndexButtonClicked(TObject *Sender, int Index)
{
	if (ButtonGroupRSCPIndex->ItemIndex == Index) return;

	TGameModeSpecSettings GMSpecSettings;
	this->GetAppropriateGMSpecSettings(GMSpecSettings);

	//Сохраняем параметры контрольной точки по старому индексу
	GMSpecSettings.REPLAYScreenControlPointIndex = ButtonGroupRSCPIndex->ItemIndex;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Enabled = CheckBoxRSCPState->Checked;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Coordinates =
		TPoint(UpDownRSX->Position, UpDownRSY->Position);
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].PixelColor = PanelRSColor->Color;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Tolerance = UpDownRSColorTolerance->Position;

	this->ApplyAppropriateGMSpecSettings(GMSpecSettings);

	//Обновляем индекс и вычитываем параметры по выбранному индексу
	GMSpecSettings.REPLAYScreenControlPointIndex = Index;

	CheckBoxRSCPState->Checked = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Enabled;
	UpDownRSX->Position = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Coordinates.x;
	UpDownRSY->Position = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Coordinates.y;
	PanelRSColor->Color = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].PixelColor;
	UpDownRSColorTolerance->Position = GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Tolerance;

	//ButtonGroupRSCPIndex->ItemIndex = Index;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonGroupEDCPIndexButtonClicked(TObject *Sender, int Index)
{
	if (ButtonGroupEDCPIndex->ItemIndex == Index) return;

	//Сохраняем параметры контрольной точки по старому индексу
	g_pSettingsManager->EnergyDialogControlPointIndex = ButtonGroupEDCPIndex->ItemIndex;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Enabled = CheckBoxEDCPState->Checked;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Coordinates =
		TPoint(UpDownEDX->Position, UpDownEDY->Position);
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].PixelColor = PanelEDColor->Color;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Tolerance = UpDownEDColorTolerance->Position;

	//Обновляем по новому индексу
	g_pSettingsManager->EnergyDialogControlPointIndex = Index;

	CheckBoxEDCPState->Checked = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Enabled;
	UpDownEDX->Position = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Coordinates.x;
	UpDownEDY->Position = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Coordinates.y;
	PanelEDColor->Color = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].PixelColor;
	UpDownEDColorTolerance->Position = g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Tolerance;

	//ButtonGroupEDCPIndex->ItemIndex = Index;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::ButtonGroupSMDCPIndexButtonClicked(TObject *Sender, int Index)
{
	if (ButtonGroupSMDCPIndex->ItemIndex == Index) return;

	//Сохраняем параметры контрольной точки по старому индексу
	g_pSettingsManager->SMDialogControlPointIndex = ButtonGroupSMDCPIndex->ItemIndex;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Enabled = CheckBoxSMDCPState->Checked;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Coordinates =
		TPoint(UpDownSMX->Position, UpDownSMY->Position);
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].PixelColor = PanelSMColor->Color;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Tolerance = UpDownSMColorTolerance->Position;

	//Обновляем по новому индексу
	g_pSettingsManager->SMDialogControlPointIndex = Index;

	CheckBoxSMDCPState->Checked = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Enabled;
	UpDownSMX->Position = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Coordinates.x;
	UpDownSMY->Position = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Coordinates.y;
	PanelSMColor->Color = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].PixelColor;
	UpDownSMColorTolerance->Position = g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Tolerance;

	//ButtonGroupSMDCPIndex->ItemIndex = Index;
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::MenuItemOpenLogFileClick(TObject *Sender)
{
	ShellExecute(NULL, L"OPEN", g_pLogManager->LogFile.c_str(), NULL, NULL, SW_SHOWNORMAL);
}
//---------------------------------------------------------------------------

void __fastcall TFormMain::BitBtnCalculationsClick(TObject *Sender)
{
	if (!this->CheckActivePageWithMessage())
		return;

	if (m_ActiveTaskInfo.CurrentState != TaskState::tsStopped)
        return;

	FormCalculations->Execute(this, UpDownNumberOfBattles);
}
//---------------------------------------------------------------------------

