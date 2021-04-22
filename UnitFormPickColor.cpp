//---------------------------------------------------------------------------

#include "URAIDAPCH.h"
#include <Jpeg.hpp>
#include <pngimage.hpp>
#include <memory>
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

void __fastcall TFormPickColor::ButtonSourceFromGameClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;
	PanelMenu->Visible = false;

	g_pRAIDWorker->ValidateGameWindow();

	this->ResizeAndAlignWindow();

	g_pRAIDWorker->CaptureFrame(ImageCapturedFrame->Canvas, TSize(this->ClientWidth, this->ClientHeight));
}
//---------------------------------------------------------------------------

void __fastcall TFormPickColor::ButtonSourceFromFileClick(TObject *Sender)
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

void __fastcall TFormPickColor::ImageCapturedFrameMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y)
{
	LabelX->Caption = IntToStr(static_cast<int>(this->m_Data.XY.x = X));
	LabelY->Caption = IntToStr(static_cast<int>(this->m_Data.XY.y = Y));

	TColor PixelColor = ImageCapturedFrame->Canvas->Pixels[m_Data.XY.x][m_Data.XY.y];
	String strRGBValues;
	strRGBValues.sprintf(L"R: %i G: %i B: %i", GetRValue(PixelColor), GetGValue(PixelColor), GetBValue(PixelColor));
	PanelColor->Color = PixelColor;
	LabelColorInRGB->Caption = strRGBValues;
}
//---------------------------------------------------------------------------

void __fastcall TFormPickColor::FormShow(TObject *Sender)
{
	this->ClientWidth = this->Constraints->MinWidth;
	this->ClientHeight = this->Constraints->MinHeight;

	this->Left = (Screen->Width - this->ClientWidth) / 2;
	this->Top = (Screen->Height - this->ClientHeight) / 2;

	PanelMenu->Visible = true;
}
//---------------------------------------------------------------------------
void TFormPickColor::ResizeAndAlignWindow()
{
	TRect RAIDWindowSize = g_pRAIDWorker->GetGameWindowSize(true);

	this->Left = (Screen->Width - RAIDWindowSize.Right) / 2;
	this->Top = (Screen->Height - RAIDWindowSize.Bottom) / 2;
	this->ClientWidth = RAIDWindowSize.Right;
	this->ClientHeight = RAIDWindowSize.Bottom;
}
//---------------------------------------------------------------------------


