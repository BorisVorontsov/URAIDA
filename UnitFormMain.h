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
	GameModeSpecSettings Settings;
	TDateTime StartTime;
	tagTaskInfo() { ZeroMemory(this, sizeof(*this)); }
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
	TButton *ButtonRunTask;
	TButton *ButtonStopTask;
	TTabSheet *TabSheetCommon;
	TBevel *Bevel1;
	TTimer *TimerMain;
	TLabel *LabelBattlesCounter;
	TTrayIcon *TrayIconApp;
	TLabel *LabelCopyright1;
	TPopupMenu *PopupMenuTray;
	TMenuItem *ShowHideAutomatizer1;
	TMenuItem *Exit1;
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
	TLabel *LabelBT;
	TLabel *LabelNumberofBattles;
	TGroupBox *GroupBoxScreens;
	TLabel *LabelRSY;
	TLabel *LabelRSX;
	TLabel *LabelREPLAYScreen;
	TLabel *LabelRSColor;
	TLabel *LabelSSY;
	TLabel *LabelSSX;
	TLabel *LabelSSColor;
	TEdit *EditRSY;
	TUpDown *UpDownRSYPos;
	TUpDown *UpDownRSXPos;
	TEdit *EditRSX;
	TPanel *PanelRSColor;
	TBitBtn *BitBtnRSPickColor;
	TEdit *EditSSY;
	TBitBtn *BitBtnSSPickColor;
	TUpDown *UpDownSSYPos;
	TPanel *PanelSSColor;
	TUpDown *UpDownSSXPos;
	TEdit *EditSSX;
	TCheckBox *CheckBoxProcessSTARTScreen;
	TEdit *EditNumberofBattles;
	TUpDown *UpDownNumberofBattles;
	TLinkLabel *LinkLabel1;
	TLinkLabel *LinkLabel2;
	TBevel *Bevel3;
	TButton *ButtonApplyGWSize;
	TButton *ButtonUseCurrentGWSize;
	TProgressBar *ProgressBarBattle;
	TFileOpenDialog *FileOpenDialogGeneric;
	TEdit *EditBTMinutes;
	TUpDown *UpDownBTMinutes;
	TLabel *LabelBTSeparator;
	TEdit *EditBTSeconds;
	TUpDown *UpDownBTSeconds;
	TPanel *PanelCover;
	TImage *ImageAppIcon;
	TMenuItem *N1;
	TMenuItem *OpenResults1;
	TGroupBox *GroupBoxTaskBreakers;
	TGroupBox *GroupBoxEnergyDialog;
	TLabel *LabelEDY;
	TLabel *LabelEDX;
	TLabel *LabelEDColor;
	TLabel *LabelEDABCP;
	TEdit *EditEDY;
	TBitBtn *BitBtnEDPickColor;
	TUpDown *UpDownEDYPos;
	TPanel *PanelEDColor;
	TUpDown *UpDownEDXPos;
	TEdit *EditEDX;
	TLabel *LabelEDAction;
	TRadioButton *RadioButtonEDAccept;
	TRadioButton *RadioButtonEDSkip;
	TGroupBox *GroupBoxMaintenanceDialog;
	TLabel *LabelSMY;
	TLabel *LabelSMX;
	TLabel *LabelSMColor;
	TLabel *LabelSMCP;
	TLabel *LabelSMAction;
	TEdit *EditSMY;
	TBitBtn *BitBtnSMPickColor;
	TUpDown *UpDownSMYPos;
	TPanel *PanelSMColor;
	TUpDown *UpDownSMXPos;
	TEdit *EditSMX;
	TRadioButton *RadioButtonSMAccept;
	TRadioButton *RadioButtonSMSkip;
	TLabel *LabelColorTolerance;
	TEdit *EditColorTolerance;
	TUpDown *UpDownColorTolerance;
	TColorDialog *ColorDialogCPColor;
	TLabel *LabelPreferredGWSize;
	TButton *Button1;
	void __fastcall ButtonRunTaskClick(TObject *Sender);
	void __fastcall ButtonStopTaskClick(TObject *Sender);
	void __fastcall TimerMainTimer(TObject *Sender);
	void __fastcall ShowHideAutomatizer1Click(TObject *Sender);
	void __fastcall Exit1Click(TObject *Sender);
	void __fastcall TrayIconAppDblClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BitBtnSSPickColorClick(TObject *Sender);
	void __fastcall ButtonApplyGWSizeClick(TObject *Sender);
	void __fastcall ButtonUseCurrentGWSizeClick(TObject *Sender);
	void __fastcall BitBtnRSPickColorClick(TObject *Sender);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall LinkLabel1Click(TObject *Sender);
	void __fastcall LinkLabel2Click(TObject *Sender);
	void __fastcall TrayIconAppBalloonClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall PageControlURAIDASettingsChange(TObject *Sender);
	void __fastcall PageControlURAIDASettingsChanging(TObject *Sender, bool &AllowChange);
	void __fastcall ButtonSRBrowsePathClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall ButtonClearAllResultsClick(TObject *Sender);
	void __fastcall OpenResults1Click(TObject *Sender);
	void __fastcall CheckBoxSaveResultsClick(TObject *Sender);
	void __fastcall BitBtnEDPickColorClick(TObject *Sender);
	void __fastcall BitBtnSMPickColorClick(TObject *Sender);
	void __fastcall PanelSSColorClick(TObject *Sender);
	void __fastcall PanelRSColorClick(TObject *Sender);
	void __fastcall PanelEDColorClick(TObject *Sender);
	void __fastcall PanelSMColorClick(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);


private:	// User declarations
	TaskInfo m_ActiveTaskInfo;
	bool m_bForceStopTask;

	static const String m_strButtonRTRunCaption;
	static const String m_strButtonRTPauseCaption;
	static const String m_strButtonRTResumeCaption;

	void StartTask();
	void StopTask(TaskStoppingReason Reason);

	void SaveResult(unsigned int nBattleNumber = -1);

	void UpdateGMSpecSettingsFrame(TTabSheet *pPage);
	GameModeSpecSettings SaveSettingsFromGMSpecSettingsFrame();
	void UpdateCommonSettingsFrame();
	void SaveSettingsFromCommonSettingsFrame();

public:		// User declarations
	__fastcall TFormMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormMain *FormMain;
//---------------------------------------------------------------------------
#endif
