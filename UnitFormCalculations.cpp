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
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TFormCalculations::FormShow(TObject *Sender)
{
	this->Left = (Screen->Width - this->ClientWidth) / 2;
	this->Top = (Screen->Height - this->ClientHeight) / 2;

	this->CalculateBattleCost(UpDownNumberOfBattles->Position, UpDownBattleCost->Position);
}
//---------------------------------------------------------------------------
void TFormCalculations::CalculateBattleCost(unsigned int nNumberOfBattles, unsigned int uBattleCost)
{
	LabelBCTotal->Caption = String(nNumberOfBattles * uBattleCost);
}
//---------------------------------------------------------------------------
void __fastcall TFormCalculations::UpDownNumberOfBattlesChangingEx(TObject *Sender,
		  bool &AllowChange, int NewValue, TUpDownDirection Direction)
{
	this->CalculateBattleCost(NewValue, UpDownBattleCost->Position);
}
//---------------------------------------------------------------------------
void __fastcall TFormCalculations::UpDownBattleCostChangingEx(TObject *Sender, bool &AllowChange,
          int NewValue, TUpDownDirection Direction)
{
	this->CalculateBattleCost(UpDownNumberOfBattles->Position, NewValue);
}
//---------------------------------------------------------------------------
void TFormCalculations::Execute(TForm* pParent)
{
	this->PopupParent = nullptr; // остыль дл€ обхода проблемы заблокированного модального окна с формой OnTop
	this->PopupParent = pParent;
	this->ShowModal();
}
//---------------------------------------------------------------------------

