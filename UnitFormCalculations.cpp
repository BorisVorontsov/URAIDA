//---------------------------------------------------------------------------
#include "URAIDAPCH.h"
#pragma hdrstop

#include "UnitFormCalculations.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFormCalculations *FormCalculations;
//---------------------------------------------------------------------------
__fastcall TFormCalculations::TFormCalculations(TComponent* Owner)
	: TForm(Owner), m_pReference(nullptr)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormCalculations::FormShow(TObject *Sender)
{
	this->Left = (Screen->Width - this->ClientWidth) / 2;
	this->Top = (Screen->Height - this->ClientHeight) / 2;

	this->BitBtnCalcTaskCostClick(this);
    this->BitBtnCalcNumberOfBattlesClick(this);
}
//---------------------------------------------------------------------------
void TFormCalculations::Execute(TForm* pParent, TUpDown* pReference)
{
	this->PopupParent = nullptr; // остыль дл€ обхода проблемы заблокированного модального окна с формой OnTop
	this->PopupParent = pParent;
	m_pReference = pReference;
	BitBtnCopyFromReference->Enabled = (pReference);
	BitBtnChangeReference->Enabled = (pReference);
	this->ShowModal();
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::FormKeyPress(TObject *Sender, System::WideChar &Key)
{
	if (Key == VK_ESCAPE)
	{
		this->Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::FormCreate(TObject *Sender)
{
	for (int i = 0; i < 24; i++)
		ComboBoxBattleCost->AddItem(IntToStr(i + 1), nullptr);
	ComboBoxBattleCost->ItemIndex = 0;

	this->ComboBoxHeroRankChange(this);
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::BitBtnCopyFromReferenceClick(TObject *Sender)
{
	UpDownNumberOfBattles->Position = m_pReference->Position;
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::ComboBoxHeroRankChange(TObject *Sender)
{
	UpDownDesiredHeroLevel->Position = (ComboBoxHeroRank->ItemIndex + 1) * 10;
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::BitBtnCalcTaskCostClick(TObject *Sender)
{
	LabelTaskCostTotal->Caption = String(UpDownNumberOfBattles->Position * StrToInt(ComboBoxBattleCost->Text));
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::BitBtnCalcNumberOfBattlesClick(TObject *Sender)
{
	//TODO

	LabelNumberOfBattlesTotal->Caption = L"0";
	LabelNBSilverTotal->Caption = L"0";
    LabelNBEnergyTotal->Caption = L"0";
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::BitBtnChangeReferenceClick(TObject *Sender)
{
	m_pReference->Position = StrToInt(LabelNumberOfBattlesTotal->Caption);
}
//---------------------------------------------------------------------------

