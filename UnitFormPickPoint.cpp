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
bool TFormPickPoint::Execute(TForm* pParent)
{
	if (m_bOnlyCoordinates)
	{
		LabelColorInfo->Visible = false;
		PanelColor->Visible = false;
		LabelColorInRGB->Visible = false;
	}
	else
	{
		LabelColorInfo->Visible = true;
		PanelColor->Visible = true;
	}

	this->PopupParent = nullptr; // остыль дл€ обхода проблемы заблокированного модального окна с формой OnTop
	this->PopupParent = pParent;
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

void __fastcall TFormPickPoint::ImageCapturedFrameMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y)
{
	LabelX->Caption = IntToStr(static_cast<int>(this->m_Data.XY.x = X));
	LabelY->Caption = IntToStr(static_cast<int>(this->m_Data.XY.y = Y));

	if (!m_bOnlyCoordinates)
	{
		TColor PixelColor = ImageCapturedFrame->Canvas->Pixels[m_Data.XY.x][m_Data.XY.y];
		String strRGBValues;
		strRGBValues.sprintf(L"(R: %i G: %i B: %i)", GetRValue(PixelColor), GetGValue(PixelColor), GetBValue(PixelColor));
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

    PanelCPInfo->Visible = false;
	PanelMenu->Visible = true;
}
//---------------------------------------------------------------------------
void TFormPickPoint::ResizeAndAlignWindow()
{
	TSize GWSizeFromSettings = g_pSettingsManager->RAIDWindowSize;

	this->ClientWidth = GWSizeFromSettings.cx;
	this->ClientHeight = GWSizeFromSettings.cy + PanelCPInfo->Height;
	this->Left = (Screen->Width - this->ClientWidth) / 2;
	this->Top = (Screen->Height - this->ClientHeight) / 2;
}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::PanelCPInfoMouseMove(TObject *Sender, TShiftState Shift,
          int X, int Y)
{
	if (Shift.Contains(ssLeft))
	{
		ReleaseCapture();
		SendMessage(this->Handle, WM_NCLBUTTONDOWN, HTCAPTION, 0);
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::BitBtnSourceFromGameClick(TObject *Sender)
{
	if (!g_pRAIDWorker->CheckGameStateWithMessage(this)) return;
	PanelMenu->Visible = false;
	PanelCPInfo->Visible = true;

	g_pRAIDWorker->ValidateGameWindow();

	this->ResizeAndAlignWindow();

	//≈сли надо выводить и помещаемс€ по ширине, делаем видимым..
	if (!m_bOnlyCoordinates)
	{
		LabelColorInRGB->Visible = (PanelCPInfo->ClientWidth > LabelColorInRGB->BoundsRect.Right);
	}
	else
	{
		LabelColorInRGB->Visible = false;
	}

	ImageCapturedFrame->Picture->Bitmap->SetSize(ImageCapturedFrame->Width, ImageCapturedFrame->Height);
	g_pRAIDWorker->CaptureFrame();
	g_pRAIDWorker->DrawFrame(ImageCapturedFrame->Canvas, ImageCapturedFrame->BoundsRect.Size);
}
//---------------------------------------------------------------------------

void __fastcall TFormPickPoint::BitBtnSourceFromFileClick(TObject *Sender)
{
	if (FileOpenDialog1->Execute(this->Handle))
	{
		std::unique_ptr<TBitmap> pBitmap(new TBitmap());
		switch (FileOpenDialog1->FileTypeIndex)
		{
			case 1:
			{
				std::unique_ptr<TJPEGImage> pJPEGImage(new TJPEGImage());
				pJPEGImage->LoadFromFile(FileOpenDialog1->FileName);
				pJPEGImage->DIBNeeded();
				pBitmap->Assign(pJPEGImage.get());
				break;
			}
			case 2:
			{
				std::unique_ptr<TPngImage> pPngImage(new TPngImage());
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
			PanelCPInfo->Visible = true;

			this->ResizeAndAlignWindow();

			//ƒополнительна€ проверка размеров изображени€ и настройки игрового окна
			//¬ажно, что бы они совпадали дл€ корректных координат
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

