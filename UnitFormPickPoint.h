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
#include <Vcl.Buttons.hpp>
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
	TImage *ImageCapturedFrame;
	TPanel *PanelMenu;
	TLabel *LabelSource;
	TFileOpenDialog *FileOpenDialog1;
	TPanel *PanelCPInfo;
	TLabel *LabelColorInfo;
	TLabel *LabelYInfo;
	TLabel *LabelXInfo;
	TLabel *LabelColorInRGB;
	TLabel *LabelY;
	TLabel *LabelX;
	TPanel *PanelColor;
	TBevel *Bevel1;
	TBevel *Bevel2;
	TBitBtn *BitBtnSourceFromGame;
	TBitBtn *BitBtnSourceFromFile;
	void __fastcall ImageCapturedFrameClick(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall ImageCapturedFrameMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall PanelCPInfoMouseMove(TObject *Sender, TShiftState Shift, int X,
          int Y);
	void __fastcall BitBtnSourceFromGameClick(TObject *Sender);
	void __fastcall BitBtnSourceFromFileClick(TObject *Sender);

private:	// User declarations
	PickPointData m_Data;
	bool m_bCancelled;
	bool m_bOnlyCoordinates;

	void ResizeAndAlignWindow();
public:		// User declarations
	__fastcall TFormPickPoint(TComponent* Owner);

	__property bool OnlyCoordinates = { read = m_bOnlyCoordinates, write = m_bOnlyCoordinates };

	bool Execute(TForm* pParent);
	PickPointData GetResults() { return m_Data; }
};
//---------------------------------------------------------------------------
extern PACKAGE TFormPickPoint *FormPickPoint;
//---------------------------------------------------------------------------
#endif
