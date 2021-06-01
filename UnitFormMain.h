//---------------------------------------------------------------------------

#ifndef UnitFormMainH
#define UnitFormMainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Mask.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.Buttons.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <System.Win.TaskbarCore.hpp>
#include <Vcl.Taskbar.hpp>

#include "UnitSettings.h"
#include <Vcl.ButtonGroup.hpp>
//---------------------------------------------------------------------------

typedef enum tagTaskState
{
	tsRunning = 1,
	tsPaused,
	tsStopped
} TaskState;

typedef struct tagTaskInfo
{
	TaskState CurrentState;
	TGameModeSpecSettings Settings;
	TDateTime StartTime;
	tagTaskInfo() { CurrentState = TaskState::tsStopped; }
} TaskInfo;

typedef enum tagTaskStoppingReason
{
	tsrUser = 0,
	tsrSuccessfulCompletion,
	tsrError
} TaskStoppingReason;

class TFormMain : public TForm
{
__published:	// IDE-managed Components
	TPageControl *PageControlURAIDASettings;
	TTabSheet *TabSheetCampaign;
	TTabSheet *TabSheetDungeons;
	TTabSheet *TabSheetFactionWars;
	TTabSheet *TabSheetCommon;
	TBevel *Bevel1;
	TTimer *TimerMain;
	TTrayIcon *TrayIconApp;
	TLabel *LabelCopyright1;
	TPopupMenu *PopupMenuTray;
	TMenuItem *MenuItemShowHideAutomatizer;
	TMenuItem *MenuItemExit;
	TScrollBox *ScrollBox1;
	TLabel *LabelGWWidth;
	TEdit *EditGWWidth;
	TUpDown *UpDownGWWidth;
	TLabel *LabelGWHeight;
	TEdit *EditGWHeight;
	TUpDown *UpDownGWHeight;
	TGroupBox *GroupBoxResults;
	TLabel *LabelSRPath;
	TCheckBox *CheckBoxSaveResults;
	TRadioButton *RadioButtonSRAtTheEndOfEachBattle;
	TRadioButton *RadioButtonSRPeriodically;
	TEdit *EditSRPeriodically;
	TUpDown *UpDownSRPeriodically;
	TEdit *EditSRPath;
	TButton *ButtonSRBrowsePath;
	TButton *ButtonClearAllResults;
	TLabel *LabelTaskEndAction;
	TComboBox *ComboBoxTaskEndAction;
	TCheckBox *CheckBoxTEAExit;
	TCheckBox *CheckBoxTEACloseTheGame;
	TGroupBox *GroupBoxErrorHandling;
	TLabel *LabelTriesBeforeForceTaskEnding;
	TEdit *EditTriesBeforeFTE;
	TUpDown *UpDownTriesBeforeFTE;
	TLabel *LabelScreenCheckingInterval;
	TEdit *EditScreenCheckingInterval;
	TUpDown *UpDownScreenCheckingInterval;
	TBevel *Bevel2;
	TLabel *LabelCopyright2;
	TScrollBox *ScrollBoxGMSpecSettings;
	TGroupBox *GroupBoxScreens;
	TLabel *LabelRSY;
	TLabel *LabelRSX;
	TLabel *LabelREPLAYScreen;
	TLabel *LabelRSColor;
	TLabel *LabelSSY;
	TLabel *LabelSSX;
	TLabel *LabelSSColor;
	TEdit *EditRSY;
	TUpDown *UpDownRSY;
	TUpDown *UpDownRSX;
	TEdit *EditRSX;
	TPanel *PanelRSColor;
	TBitBtn *BitBtnRSPickPoint;
	TEdit *EditSSY;
	TBitBtn *BitBtnSSPickPoint;
	TUpDown *UpDownSSY;
	TPanel *PanelSSColor;
	TUpDown *UpDownSSX;
	TEdit *EditSSX;
	TCheckBox *CheckBoxProcessSTARTScreen;
	TLinkLabel *LinkLabel1;
	TLinkLabel *LinkLabel2;
	TButton *ButtonApplyGWSize;
	TButton *ButtonUseCurrentGWSize;
	TProgressBar *ProgressBarBattle;
	TFileOpenDialog *FileOpenDialogGeneric;
	TPanel *PanelCover;
	TImage *ImageAppIcon;
	TMenuItem *N1;
	TMenuItem *MenuItemOpenResults;
	TGroupBox *GroupBoxTaskBreakers;
	TGroupBox *GroupBoxEnergyDialog;
	TLabel *LabelEDY;
	TLabel *LabelEDX;
	TLabel *LabelEDColor;
	TLabel *LabelEDCP;
	TEdit *EditEDY;
	TBitBtn *BitBtnEDPickPoint;
	TUpDown *UpDownEDY;
	TPanel *PanelEDColor;
	TUpDown *UpDownEDX;
	TEdit *EditEDX;
	TLabel *LabelEDAction;
	TRadioButton *RadioButtonEDAccept;
	TRadioButton *RadioButtonEDSkip;
	TGroupBox *GroupBoxMaintenanceDialog;
	TLabel *LabelSMY;
	TLabel *LabelSMX;
	TLabel *LabelSMColor;
	TLabel *LabelSMCP;
	TEdit *EditSMY;
	TBitBtn *BitBtnSMPickColor;
	TUpDown *UpDownSMY;
	TPanel *PanelSMColor;
	TUpDown *UpDownSMX;
	TEdit *EditSMX;
	TColorDialog *ColorDialogCPColor;
	TLabel *LabelPreferredGWSize;
	TLabel *LabelEDGETButton;
	TLabel *LabelEDGETX;
	TEdit *EditEDGETX;
	TUpDown *UpDownEDGETX;
	TLabel *LabelEDGETY;
	TEdit *EditEDGETY;
	TUpDown *UpDownEDGETY;
	TBitBtn *BitBtnEDGETPickPoint;
	TRadioButton *RadioButtonEDAbort;
	TGroupBox *GroupBoxTaskParameters;
	TEdit *EditBTMinutes;
	TUpDown *UpDownNumberOfBattles;
	TEdit *EditNumberOfBattles;
	TUpDown *UpDownBTSeconds;
	TEdit *EditBTSeconds;
	TLabel *LabelBTSeparator;
	TUpDown *UpDownBTMinutes;
	TLabel *LabelBT;
	TLabel *LabelNumberOfBattles;
	TLabel *LabelRSAction;
	TRadioButton *RadioButtonRSActionReplay;
	TRadioButton *RadioButtonRSActionNext;
	TTaskbar *TaskbarApp;
	TPanel *PanelBattles;
	TLabel *LabelBattlesCounter;
	TLabel *LabelSSColorTolerance;
	TEdit *EditSSColorTolerance;
	TUpDown *UpDownSSColorTolerance;
	TLabel *LabelRSColorTolerance;
	TEdit *EditRSColorTolerance;
	TUpDown *UpDownRSColorTolerance;
	TLabel *LabelEDColorTolerance;
	TEdit *EditEDColorTolerance;
	TUpDown *UpDownEDColorTolerance;
	TLabel *LabelSMColorTolerance;
	TEdit *EditSMColorTolerance;
	TUpDown *UpDownSMColorTolerance;
	TLinkLabel *LinkLabelReleases;
	TMenuItem *MenuItemMainWindow;
	TMenuItem *N2;
	TMenuItem *MenuItemStayOnTop;
	TMenuItem *MenuItemMoveToCenter;
	TCheckBox *CheckBoxClearOldResults;
	TBitBtn *BitBtnStopTask;
	TBitBtn *BitBtnRunTask;
	TImageList *ImageListRTButton;
	TButtonGroup *ButtonGroupSSCPIndex;
	TButtonGroup *ButtonGroupRSCPIndex;
	TButtonGroup *ButtonGroupEDCPIndex;
	TButtonGroup *ButtonGroupSMDCPIndex;
	TCheckBox *CheckBoxSSCPState;
	TCheckBox *CheckBoxRSCPState;
	TCheckBox *CheckBoxEDCPState;
	TCheckBox *CheckBoxSMDCPState;
	TMenuItem *MenuItemOpenLogFile;
	TBitBtn *BitBtnCalculations;
	void __fastcall TimerMainTimer(TObject *Sender);
	void __fastcall MenuItemShowHideAutomatizerClick(TObject *Sender);
	void __fastcall MenuItemExitClick(TObject *Sender);
	void __fastcall TrayIconAppDblClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BitBtnSSPickPointClick(TObject *Sender);
	void __fastcall ButtonApplyGWSizeClick(TObject *Sender);
	void __fastcall ButtonUseCurrentGWSizeClick(TObject *Sender);
	void __fastcall BitBtnRSPickPointClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall LinkLabel1Click(TObject *Sender);
	void __fastcall LinkLabel2Click(TObject *Sender);
	void __fastcall TrayIconAppBalloonClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall PageControlURAIDASettingsChange(TObject *Sender);
	void __fastcall PageControlURAIDASettingsChanging(TObject *Sender, bool &AllowChange);
	void __fastcall ButtonSRBrowsePathClick(TObject *Sender);
	void __fastcall ButtonClearAllResultsClick(TObject *Sender);
	void __fastcall MenuItemOpenResultsClick(TObject *Sender);
	void __fastcall CheckBoxSaveResultsClick(TObject *Sender);
	void __fastcall BitBtnEDPickPointClick(TObject *Sender);
	void __fastcall BitBtnSMPickColorClick(TObject *Sender);
	void __fastcall PanelSSColorClick(TObject *Sender);
	void __fastcall PanelRSColorClick(TObject *Sender);
	void __fastcall PanelEDColorClick(TObject *Sender);
	void __fastcall PanelSMColorClick(TObject *Sender);
	void __fastcall BitBtnEDGETPickPointClick(TObject *Sender);
	void __fastcall LinkLabelReleasesClick(TObject *Sender);
	void __fastcall MenuItemStayOnTopClick(TObject *Sender);
	void __fastcall MenuItemMoveToCenterClick(TObject *Sender);
	void __fastcall BitBtnRunTaskClick(TObject *Sender);
	void __fastcall BitBtnStopTaskClick(TObject *Sender);
	void __fastcall PopupMenuTrayPopup(TObject *Sender);
	void __fastcall ButtonGroupSSCPIndexButtonClicked(TObject *Sender, int Index);
	void __fastcall ButtonGroupRSCPIndexButtonClicked(TObject *Sender, int Index);
	void __fastcall ButtonGroupEDCPIndexButtonClicked(TObject *Sender, int Index);
	void __fastcall ButtonGroupSMDCPIndexButtonClicked(TObject *Sender, int Index);
	void __fastcall MenuItemOpenLogFileClick(TObject *Sender);
	void __fastcall BitBtnCalculationsClick(TObject *Sender);


private:	// User declarations
	TaskInfo m_ActiveTaskInfo;
	bool m_bForceStopTask;

	static const String m_strButtonRTRunCaption;
	static const String m_strButtonRTPauseCaption;
	static const String m_strButtonRTResumeCaption;

	void StartTask();
	void StopTask(TaskStoppingReason Reason);

	void SaveResult(unsigned int nBattleNumber, bool bError = false);

	void GetAppropriateGMSpecSettings(TGameModeSpecSettings& Result);
	void ApplyAppropriateGMSpecSettings(const TGameModeSpecSettings& Input);
	void UpdateGMSpecSettingsFrame();
	void SaveSettingsFromGMSpecSettingsFrame();
	void UpdateCommonSettingsFrame();
	void SaveSettingsFromCommonSettingsFrame();

	void UpdateNecessarySettings();
	void SaveNecessarySettings();

public:		// User declarations
	__fastcall TFormMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormMain *FormMain;
//---------------------------------------------------------------------------
#endif
