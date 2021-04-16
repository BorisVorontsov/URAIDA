//---------------------------------------------------------------------------

#include "URAIDAPCH.h"
#pragma hdrstop

#include "UnitFormPickColor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormPickColor *FormPickColor;

//---------------------------------------------------------------------------
__fastcall TFormPickColor::TFormPickColor(TComponent* Owner)
	: TForm(Owner)
{

}
//---------------------------------------------------------------------------
void __fastcall TFormPickColor::FormShow(TObject *Sender)
{
	TRect RAIDWindowSize = g_pRAIDWorker->GetGameWindowSize(true);
	this->Width = RAIDWindowSize.Right;
	this->Height = RAIDWindowSize.Bottom;
}
//---------------------------------------------------------------------------

void __fastcall TFormPickColor::FormActivate(TObject *Sender)
{
	TSize FormSize;
	FormSize.cx = this->ClientWidth;
	FormSize.cy = this->ClientHeight;

	g_pRAIDWorker->CaptureFrame(ImageCapturedFrame->Canvas, FormSize);
}
//---------------------------------------------------------------------------

void __fastcall TFormPickColor::ImageCapturedFrameMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y)
{
	LabelX->Caption = IntToStr(static_cast<int>(this->m_Data.XY.x = X));
	LabelY->Caption = IntToStr(static_cast<int>(this->m_Data.XY.y = Y));

	PanelColor->Color = ImageCapturedFrame->Canvas->Pixels[m_Data.XY.x][m_Data.XY.y];
}
//---------------------------------------------------------------------------

void __fastcall TFormPickColor::ImageCapturedFrameClick(TObject *Sender)
{
	m_Data.Color = ImageCapturedFrame->Canvas->Pixels[m_Data.XY.x][m_Data.XY.y];
	m_Data.bCancelled = false;
	this->Close();
}
//---------------------------------------------------------------------------

void __fastcall TFormPickColor::FormKeyPress(TObject *Sender, System::WideChar &Key)

{
	if (Key == VK_ESCAPE)
	{
		m_Data.bCancelled = true;
		this->Close();
	}
}
//---------------------------------------------------------------------------

