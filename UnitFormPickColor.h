//---------------------------------------------------------------------------

#ifndef UnitFormPickColorH
#define UnitFormPickColorH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
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
	TLabel *LabelXInfo;
	TLabel *LabelX;
	TLabel *LabelYInfo;
	TLabel *LabelY;
	TLabel *LabelColorInfo;
	TImage *ImageCapturedFrame;
	TPanel *PanelColor;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormActivate(TObject *Sender);
	void __fastcall ImageCapturedFrameMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall ImageCapturedFrameClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);

private:	// User declarations
	PCData m_Data;
public:		// User declarations
	__fastcall TFormPickColor(TComponent* Owner);

	PCData GetResults() { return m_Data; }
};
//---------------------------------------------------------------------------
extern PACKAGE TFormPickColor *FormPickColor;
//---------------------------------------------------------------------------
#endif
