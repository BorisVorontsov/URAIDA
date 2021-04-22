//---------------------------------------------------------------------------

#ifndef UnitFormPickColorH
#define UnitFormPickColorH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
//---------------------------------------------------------------------------
typedef struct tagPCData
{
	TPoint XY;
	TColor Color;
	bool bCancelled;
} PCData;

class TFormPickColor : public TForm
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
	PCData m_Data;

	void ResizeAndAlignWindow();
public:		// User declarations
	__fastcall TFormPickColor(TComponent* Owner);

	PCData GetResults() { return m_Data; }
};
//---------------------------------------------------------------------------
extern PACKAGE TFormPickColor *FormPickColor;
//---------------------------------------------------------------------------
#endif
