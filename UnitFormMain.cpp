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

const String TFormMain::m_strButtonRTRunCaption = L"�����";
const String TFormMain::m_strButtonRTPauseCaption = L"�����";
const String TFormMain::m_strButtonRTResumeCaption = L"�����";

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

	//� �����?
	if (!g_pRAIDWorker->IsGameRunning())
	{
        nCycleCounter = 0;
		this->StopTask(TaskStoppingReason::tsrError);
		MessageBox(this->Handle, L"�� ������ ����� ���� ����!", L"������", MB_ICONSTOP);

		return;
	}

	//���������� ��������� (������) ��� � �������� ��������
	if (g_pSettingsManager->SaveResults &&
		(g_pSettingsManager->ResultSavingMethod == ResultSavingMode::rsmPeriodically) &&
		(uResultSavingTimeout == g_pSettingsManager->ResultSavingPeriod))
	{
		this->SaveResult(nCycleCounter);
		uResultSavingTimeout = 0;
	}

	if (!nCycleCounter)
	{
		//�������� �������� ������� ������ � �������� ���� ���, � ������ ������� ���
		uBattleDelayInSeconds = (HIWORD(m_ActiveTaskInfo.Settings.Delay) * 60) + LOWORD(m_ActiveTaskInfo.Settings.Delay);

		if (!uBattleDelayInSeconds)
		{
			this->StopTask(TaskStoppingReason::tsrError);
			MessageBox(this->Handle, L"����� ��� ������ ���� ������ ����!", L"������", MB_ICONSTOP);

			return;
        }
	}

	if ((nCycleCounter > m_ActiveTaskInfo.Settings.NumberOfBattles) && bScreenCheckPassed)
	{
		//�������� ���������� ������
		nCycleCounter = 0;
		this->StopTask(TaskStoppingReason::tsrSuccessfulCompletion);

		return;
	}
	else if (m_bForceStopTask)
	{
		//������� �� ������
		nCycleCounter = 0;
		this->StopTask(TaskStoppingReason::tsrUser);

        return;
    }

	if (bScreenCheckPassed)
	{
		//��������� �������� �������� ���
		ProgressBarBattle->Position = 100.0f * (static_cast<float>(uBattleTimeout) /
			static_cast<float>(uBattleDelayInSeconds));
		TaskbarApp->ProgressValue = ProgressBarBattle->Position;
	}

    //������ ������� ���
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

	//��� 90% �����
	//���� �� ���� ������ �� ������, ��� ��� ����� ������, � ������, ��� ���� ������, ��� �� ��������� �������
	if ((!nScreenCheckFailures || (nScreenCheckFailures && (uScreenCheckingTiomeout == g_pSettingsManager->ScreenCheckingPeriod))) &&
		!bScreenCheckPassed)
	{
		g_pRAIDWorker->ValidateGameWindow();

		uScreenCheckingTiomeout = 0;

		//���� ��������� ����������� ����� � ������
		auto CompareControlPoints
		{
			[](std::array<TControlPoint, g_uMaxControlPoints>& ControlPoints)
			{
				unsigned int nEnabledCPs = 0;

				//����������� ������� ����, ��� ������� ����� ����� ����������� ������
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
								g_pLogManager->Append(L"��������� �� ��������� ( X: %i, Y: %i ����: %i �����������: %i )",
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
						g_pLogManager->Append(L"�������� �� �� ����������, �������");

					return false;
				}

				if (g_pSettingsManager->EnableLogging)
					g_pLogManager->Append(L"��������� ���� �������� �� ��������� �������");
				return true;
			}
		};

		//����������� ������: ������ ���������� �������
		//���������� ��� � �������� ����� ������� ������� ���
		auto EnergyDialogTest
		{
			[=](bool& bExitTimer)
			{
				if (g_pSettingsManager->EnableLogging)
					g_pLogManager->Append(L"���� %i: ������ ������������ �� ������� ���������� �������", nCycleCounter);

				if (CompareControlPoints(g_pSettingsManager->EnergyDialogControlPoints))
				{
					if (g_pSettingsManager->EnableLogging)
						g_pLogManager->Append(L"���������� �������� ��� ������� ���������� �������");

					switch (g_pSettingsManager->EnergyDialogPreferredAction)
					{
						case PromptDialogAction::pdaAccept:
							//�������, ���� ����� ������������ (�������������� ������ "��������")
							g_pRAIDWorker->SendMouseClick(g_pSettingsManager->EnergyDialogGETButtonPoint);
							bExitTimer = false;
							break;
						case PromptDialogAction::pdaSkip:
							g_pRAIDWorker->SendMouseClick(TPoint(1, 1));
							bExitTimer = false;
							break;
						case PromptDialogAction::pdaAbort:
						{
							//���� ������, ������ ����������� ������
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

		//��������� ����������� ������� ���������� �������
		bool bExitTimer;
		if (EnergyDialogTest(bExitTimer))
		{
			if (bExitTimer)
				return;
		}

		//����������� ������: ������ ����� �� �������
		if (g_pSettingsManager->EnableLogging)
			g_pLogManager->Append(L"���� %i: ������ ������������ �� ������� ����� �� �������", nCycleCounter);

		if (CompareControlPoints(g_pSettingsManager->SMDialogControlPoints))
		{
			if (g_pSettingsManager->EnableLogging)
				g_pLogManager->Append(L"���������� �������� ��� ������� ����� �� �������");

			//������ ����������
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

		//���� ����� �������� "STARTScreenFirst", ������� � ������� �� ����� ������ ���
		if ((nCycleCounter == 1) && m_ActiveTaskInfo.Settings.ProcessSTARTScreenFirst)
		{
			if (g_pSettingsManager->EnableLogging)
				g_pLogManager->Append(L"���� %i: ������ ������������ �� ������ ������", nCycleCounter);

			if (CompareControlPoints(m_ActiveTaskInfo.Settings.STARTScreenControlPoints))
			{
				if (g_pSettingsManager->EnableLogging)
					g_pLogManager->Append(L"���������� �������� ��� ������ ������");

				//���������� ������� Enter ���� ����, �� ������ ������ ��� ��� ����������� ������� "������"
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
		else //���� �������� ������ ������ ������ ��� �� �����, ��� �� ��� �� ������ ��� ����� �����, ��������� � ������� ������ "������/�����"
		{
			if (g_pSettingsManager->EnableLogging)
				g_pLogManager->Append(L"���� %i: ������ ������������ �� ������ ������/�����", nCycleCounter);

			if (CompareControlPoints(m_ActiveTaskInfo.Settings.REPLAYScreenControlPoints))
			{
				//�����, ���� ������ �������� ��������� �� ���������� ������� ���
				if ((nCycleCounter > 1) && g_pSettingsManager->SaveResults &&
					(g_pSettingsManager->ResultSavingMethod == ResultSavingMode::rsmAtTheEndOfEachBattle))
				{
					this->SaveResult(nCycleCounter - 1);
				}

				//���� �� ������ ��������� ��� (������� nCycleCounter ����� ����� nNumberOfBattles + 1),
				//������ ��� ������ �����������
				if (nCycleCounter <= m_ActiveTaskInfo.Settings.NumberOfBattles)
				{
					if (g_pSettingsManager->EnableLogging)
						g_pLogManager->Append(L"���������� �������� ��� ������ ������/�����");

					if (m_ActiveTaskInfo.Settings.REPLAYScreenPreferredAction == REPLAYScreenAction::rsaReplay)
					{
						//[R] �� ������ ����������� ��� ������������ ������� ������ "������"
						g_pRAIDWorker->SendKey('R');
					}
					else if (m_ActiveTaskInfo.Settings.REPLAYScreenPreferredAction == REPLAYScreenAction::rsaGoNext)
					{
						//���������������� ������� ������� � ������� ����� ��������� ��� ���������� ������,
						//������������ ������� ������ "�����"
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

		//���� ��������� ����� ��������� ������� ���������, ��������� ��� ������
		if (nScreenCheckFailures == g_pSettingsManager->TriesBeforeForceTaskEnding)
		{
			if (g_pSettingsManager->EnableLogging)
			{
				g_pLogManager->Append(L"�������� ����� ��������� ������������ ������: %i/%i",
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
		MessageBox(this->Handle, L"���������� �����������!", L"������", MB_ICONSTOP);
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

	//����� �������� � ������ ���������
	String strAppVersion;
	GetFileVersion(Application->ExeName, strAppVersion, fvfMajorMinor);
	LabelCopyright1->Caption = Application->Title + L" ���." + strAppVersion + L" ��";

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
	//������ ��������

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

	//��� ���������� �������� ���� ���� ������������� ���������� ���� ����������� �����,
	//���� ����� ������� ���������� �� �������
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
		if (MessageBox(this->Handle, L"������� ������ �� ���������. �� ����� �����?", L"��������",
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
	FileOpenDialogGeneric->Title = L"�������� ���������� ��� ���������� �����������";
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
		MessageBox(this->Handle, L"���� �� ������!", L"������", MB_ICONSTOP);
		return;
	}

	if (!TDirectory::IsEmpty(g_pSettingsManager->PathForResults))
	{
		if (MessageBox(this->Handle, L"�� �������, ��� ������ ������� ��� ����������?", L"��������������",
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
		MessageBox(this->Handle, L"������� �������� ����� ����", L"��������������", MB_ICONEXCLAMATION);
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

		//��������� ������ ������ �� ��������� ����������
		//��������� ����� ��������� ������ ��� ������ ����� ������
		this->SaveSettingsFromGMSpecSettingsFrame();
		TGameModeSpecSettings GMSpecSettings;
        this->GetAppropriateGMSpecSettings(GMSpecSettings);

		//���������, �� ����� �� ������������ �������� ���� �� ���� ����������� �����
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
				MessageBox(this->Handle, L"�������� ���� �� ���� ����������� ����� ������ ������!", L"��������������",
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
				MessageBox(this->Handle, L"�������� ���� �� ���� ����������� ����� ������ ������/�����!", L"��������������",
					MB_ICONEXCLAMATION);
				return;
			}
		}

		m_ActiveTaskInfo.Settings = GMSpecSettings;

        //������� ����� ����������� ����� ������� ���������� ������, ���� ������� ����� �����
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

			g_pLogManager->Append(L"����� ������: %s", strPresetName.c_str());
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

	//��������� ������� ������� � ����������� ����� � ���������� ������ ��� ���������� ������
	//�������������� � ����
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
		g_pLogManager->Append(L"��������� ������, �������: %s", strReason.c_str());
		g_pLogManager->FlushToDisk();
	}

	//���� ��� �� ������������ ����� ������..
	if (Reason != TaskStoppingReason::tsrUser)
	{
		//��������� �������� ���������� ������
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
					TrayIconApp->BalloonHint = L"������ ������� ���������";
					TrayIconApp->BalloonFlags = TBalloonFlags::bfInfo;
				}
				else if (Reason == TaskStoppingReason::tsrError)
				{
					TrayIconApp->BalloonHint = L"�������������� ���������� ������ ��-�� ������";
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

	//������� ����������
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
		g_pLogManager->Append(L"���������� ��������� ��� %i (%s)", nBattleNumber,
			(bError)?L"������ ������������":L"����");
	}

	String strPathForResults = g_pSettingsManager->PathForResults;
	String strGameMode, strActiveTaskSubDir, strFinalPath;
	String strFileName;
	//���������/��������� ���� � �����������
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

	//��������� ��� �������� ��� ������� ������
	switch (m_ActiveTaskInfo.Settings.GameMode)
	{
		case SupportedGameModes::gmCampaign:
			strGameMode = L"��������";
			break;
		case SupportedGameModes::gmDungeons:
			strGameMode = L"����������";
			break;
		case SupportedGameModes::gmFactionWars:
			strGameMode = L"������������";
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

	//���� bError, ���������� ��� ������� ��������� ��������� ������
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
				//������������� ��������� �� ��������� ���������� �������� � ���� ��������
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

		//��������� � ������� PNG ��� ����������� ������� ������ �����
		std::unique_ptr<TPngImage> pPngImage(new TPngImage());
		pPngImage->Assign(pImage->Picture->Bitmap);

		strFileName.sprintf(L"������_%s.png", FormatDateTime(L"hh-mm", Now()).c_str());
		pPngImage->SaveToFile(IncludeTrailingPathDelimiter(strFinalPath) + strFileName);
	}
	else
	{
		//�������� ����� �������� JPEG
		std::unique_ptr<TJPEGImage> pJPEGImage(new TJPEGImage());
		pJPEGImage->Assign(pImage->Picture->Bitmap);

		strFileName.sprintf(L"���%i_%s.jpeg", nBattleNumber, FormatDateTime(L"hh-mm", Now()).c_str());
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

	//��������� ��������� ����������� ����� �� ������� �������
	GMSpecSettings.STARTScreenControlPointIndex = ButtonGroupSSCPIndex->ItemIndex;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Enabled = CheckBoxSSCPState->Checked;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Coordinates =
		TPoint(UpDownSSX->Position, UpDownSSY->Position);
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].PixelColor = PanelSSColor->Color;
	GMSpecSettings.STARTScreenControlPoints[GMSpecSettings.STARTScreenControlPointIndex].Tolerance = UpDownSSColorTolerance->Position;

	this->ApplyAppropriateGMSpecSettings(GMSpecSettings);

	//��������� ������ � ���������� ��������� �� ���������� �������
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

	//��������� ��������� ����������� ����� �� ������� �������
	GMSpecSettings.REPLAYScreenControlPointIndex = ButtonGroupRSCPIndex->ItemIndex;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Enabled = CheckBoxRSCPState->Checked;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Coordinates =
		TPoint(UpDownRSX->Position, UpDownRSY->Position);
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].PixelColor = PanelRSColor->Color;
	GMSpecSettings.REPLAYScreenControlPoints[GMSpecSettings.REPLAYScreenControlPointIndex].Tolerance = UpDownRSColorTolerance->Position;

	this->ApplyAppropriateGMSpecSettings(GMSpecSettings);

	//��������� ������ � ���������� ��������� �� ���������� �������
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

	//��������� ��������� ����������� ����� �� ������� �������
	g_pSettingsManager->EnergyDialogControlPointIndex = ButtonGroupEDCPIndex->ItemIndex;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Enabled = CheckBoxEDCPState->Checked;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Coordinates =
		TPoint(UpDownEDX->Position, UpDownEDY->Position);
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].PixelColor = PanelEDColor->Color;
	g_pSettingsManager->EnergyDialogControlPoints[g_pSettingsManager->EnergyDialogControlPointIndex].Tolerance = UpDownEDColorTolerance->Position;

	//��������� �� ������ �������
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

	//��������� ��������� ����������� ����� �� ������� �������
	g_pSettingsManager->SMDialogControlPointIndex = ButtonGroupSMDCPIndex->ItemIndex;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Enabled = CheckBoxSMDCPState->Checked;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Coordinates =
		TPoint(UpDownSMX->Position, UpDownSMY->Position);
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].PixelColor = PanelSMColor->Color;
	g_pSettingsManager->SMDialogControlPoints[g_pSettingsManager->SMDialogControlPointIndex].Tolerance = UpDownSMColorTolerance->Position;

	//��������� �� ������ �������
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

