//---------------------------------------------------------------------------

#ifndef UnitFormPickPointH
#define UnitFormPickPointH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
typedef struct tagPickPointData
{
	TPoint XY;
	TColor Color;
} PickPointData;

class TFormPickPoint : public TForm
{
__published:	// IDE-managed Components
	TPanel *PanelCapturedFrame;
	TLabel *LabelColorInfo;
	TLabel *LabelX;
	TLabel *LabelXInfo;
	TLabel *LabelY;
	TLabel *LabelYInfo;
	TPanel *PanelColor;
	TLabel *LabelColorInRGB;
	TImage *ImageCapturedFrame;
	TPanel *PanelMenu;
	TButton *ButtonSourceFromFile;
	TButton *ButtonSourceFromGame;
	TLabel *LabelSource;
	TFileOpenDialog *FileOpenDialog1;
	void __fastcall ImageCapturedFrameClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall ButtonSourceFromGameClick(TObject *Sender);
	void __fastcall ButtonSourceFromFileClick(TObject *Sender);
	void __fastcall ImageCapturedFrameMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall FormShow(TObject *Sender);

private:	// User declarations
	PickPointData m_Data;
	bool m_bCancelled;
	bool m_bOnlyCoordinates;

	void ResizeAndAlignWindow();
public:		// User declarations
	__fastcall TFormPickPoint(TComponent* Owner);

	__property bool OnlyCoordinates = { read = m_bOnlyCoordinates, write = m_bOnlyCoordinates };

	bool Execute();
	PickPointData GetResults() { return m_Data; }
};
//---------------------------------------------------------------------------
extern PACKAGE TFormPickPoint *FormPickPoint;
//---------------------------------------------------------------------------
#endif