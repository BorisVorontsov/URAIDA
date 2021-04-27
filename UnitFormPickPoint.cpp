//---------------------------------------------------------------------------

#include "URAIDAPCH.h"
#include <Jpeg.hpp>
#include <pngimage.hpp>
#include <memory>
#pragma hdrstop

#include "UnitFormPickPoint.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormPickPoint *FormPickPoint;

//---------------------------------------------------------------------------
__fastcall TFormPickPoint::TFormPickPoint(TComponent* Owner)
	: TForm(Owner)
{

}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::ImageCapturedFrameClick(TObject *Sender)
{
	m_Data.Color = (!m_bOnlyCoordinates)?(ImageCapturedFrame->Canvas->Pixels[m_Data.XY.x][m_Data.XY.y]):clWhite;
    m_bCancelled = false;
	this->Close();
}
//---------------------------------------------------------------------------
bool TFormPickPoint::Execute()
{
	if (m_bOnlyCoordinates)
	{
		LabelColorInfo->Visible = false;
		PanelColor->Visible = false;
		LabelColorInRGB->Visible = false;
		PanelCPInfo->Height = PanelCPInfo->Constraints->MinHeight;
	}
	else
	{
		LabelColorInfo->Visible = true;
		PanelColor->Visible = true;
		LabelColorInRGB->Visible = true;
		PanelCPInfo->Height = PanelCPInfo->Constraints->MaxHeight;
	}

	this->ShowModal();

	return !m_bCancelled;
}
//---------------------------------------------------------------------------
void __fastcall TFormPickPoint::FormKeyPress(TObject *Sender, System::WideChar &Key)
{
	if (Key == VK_ESCAPE)
	{
		m_bCancelled = true;
		this->Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::ButtonSourceFromGameClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;
	PanelMenu->Visible = false;

	g_pRAIDWorker->ValidateGameWindow();

	this->ResizeAndAlignWindow();

	ImageCapturedFrame->Picture->Bitmap->SetSize(this->ClientWidth, this->ClientHeight);
	g_pRAIDWorker->CaptureFrame(ImageCapturedFrame->Canvas, TSize(this->ClientWidth, this->ClientHeight));
}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::ButtonSourceFromFileClick(TObject *Sender)
{
	if (FileOpenDialog1->Execute(this->Handle))
	{
		std::shared_ptr<TBitmap> pBitmap(new TBitmap());
		switch (FileOpenDialog1->FileTypeIndex)
		{
			case 1:
			{
				std::shared_ptr<TJPEGImage> pJPEGImage(new TJPEGImage());
				pJPEGImage->LoadFromFile(FileOpenDialog1->FileName);
				pJPEGImage->DIBNeeded();
				pBitmap->Assign(pJPEGImage.get());
				break;
			}
			case 2:
			{
				std::shared_ptr<TPngImage> pPngImage(new TPngImage());
				pPngImage->LoadFromFile(FileOpenDialog1->FileName);
				pBitmap->Assign(pPngImage.get());
				break;
			}
			case 3:
				pBitmap->LoadFromFile(FileOpenDialog1->FileName);
				break;
		}

		if (!pBitmap->Empty)
		{
			PanelMenu->Visible = false;

			this->ResizeAndAlignWindow();

			//Дополнительная проверка размеров изображения и настройки игрового окна
			//Важно, что бы они совпадали для корректных координат
			TSize GWSizeFromSettings = g_pSettingsManager->RAIDWindowSize;
			if ((pBitmap->Width != GWSizeFromSettings.cx) || (pBitmap->Height != GWSizeFromSettings.cy))
			{
				ImageCapturedFrame->Canvas->StretchDraw(TRect(0, 0, GWSizeFromSettings.cx, GWSizeFromSettings.cy), pBitmap.get());
			}
			else
				ImageCapturedFrame->Picture->Bitmap->Assign(pBitmap.get());
		}
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::ImageCapturedFrameMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y)
{
	LabelX->Caption = IntToStr(static_cast<int>(this->m_Data.XY.x = X));
	LabelY->Caption = IntToStr(static_cast<int>(this->m_Data.XY.y = Y));

	if (!m_bOnlyCoordinates)
	{
		TColor PixelColor = ImageCapturedFrame->Canvas->Pixels[m_Data.XY.x][m_Data.XY.y];
		String strRGBValues;
		strRGBValues.sprintf(L"R: %i G: %i B: %i", GetRValue(PixelColor), GetGValue(PixelColor), GetBValue(PixelColor));
		PanelColor->Color = PixelColor;
		LabelColorInRGB->Caption = strRGBValues;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::FormShow(TObject *Sender)
{
	this->ClientWidth = this->Constraints->MinWidth;
	this->ClientHeight = this->Constraints->MinHeight;

	this->Left = (Screen->Width - this->ClientWidth) / 2;
	this->Top = (Screen->Height - this->ClientHeight) / 2;

	PanelMenu->Visible = true;
}
//---------------------------------------------------------------------------
void TFormPickPoint::ResizeAndAlignWindow()
{
	TSize GWSizeFromSettings = g_pSettingsManager->RAIDWindowSize;

	this->Left = (Screen->Width - GWSizeFromSettings.cx) / 2;
	this->Top = (Screen->Height - GWSizeFromSettings.cy) / 2;
	this->ClientWidth = GWSizeFromSettings.cx;
	this->ClientHeight = GWSizeFromSettings.cy;
}
//---------------------------------------------------------------------------

