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
	this->PopupParent = nullptr; //Костыль для обхода проблемы заблокированного модального окна с формой OnTop
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
	ComboBoxBattleCost->ItemIndex = 7;

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
	unsigned int uMaxHeroLevel = (ComboBoxHeroRank->ItemIndex + 1) * 10;
	UpDownActualHeroLevel->Max = uMaxHeroLevel - 1;
	UpDownActualHeroLevel->Position = std::min(UpDownActualHeroLevel->Position, static_cast<int>(uMaxHeroLevel - 1));
	UpDownDesiredHeroLevel->Max = uMaxHeroLevel;
	UpDownDesiredHeroLevel->Position = uMaxHeroLevel;
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::BitBtnCalcTaskCostClick(TObject *Sender)
{
	LabelTaskCostTotal->Caption = String(UpDownNumberOfBattles->Position * StrToInt(ComboBoxBattleCost->Text));
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::BitBtnCalcNumberOfBattlesClick(TObject *Sender)
{
	if (UpDownActualHeroLevel->Position >= UpDownDesiredHeroLevel->Position)
	{
		MessageBox(this->Handle, L"Целевой уровень героя(ев) не может быть равен или ниже действительного",
			L"Ошибка", MB_ICONEXCLAMATION);
		return;
	}

	unsigned int uRequiredXP = 0;
	unsigned int nBattleCount = 0;
	unsigned int uTotalLevelXP = 0, uTotalLevelSilver = 0, uTotalLevelEnergy = 0;

	//Сначала вычисляем, сколько опыта необходимо герою для достижения нужного уровня
	HeroXPTable XPTable = g_HeroXPTable[ComboBoxHeroRank->ItemIndex];
	for (size_t i = (UpDownActualHeroLevel->Position - 1); i < (UpDownDesiredHeroLevel->Position - 1); i++)
	{
		uRequiredXP += XPTable.XPArray[i];
	}

	//Затем вычисляем, сколько итераций уровня даст необходимую сумму опыта, учитывая количество героев и бустер опыта
	CampaignLevelInfo LevelInfo;
	LevelInfo = g_CampaignTable[ComboBoxChapter->ItemIndex].MapTable[ComboBoxDifficulty->ItemIndex].LevelInfo[ComboBoxChapterLevel->ItemIndex];
	while (true)
	{
		if (uTotalLevelXP >= uRequiredXP)
			break;

		float fXPPerHero = ceil(static_cast<float>(LevelInfo.uXP) / static_cast<float>(ComboBoxHeroesCount->ItemIndex + 1));
		uTotalLevelXP += fXPPerHero * ((CheckBoxXPBooster->Checked)?2:1);
		uTotalLevelSilver += LevelInfo.uSilver;
		uTotalLevelEnergy += LevelInfo.uBattleCost;

		nBattleCount++;
	}

	LabelNumberOfBattlesTotal->Caption = String(nBattleCount);
	LabelNBSilverTotal->Caption = Format(L"%.0n", ARRAYOFCONST((static_cast<float>(uTotalLevelSilver))));
	LabelNBEnergyTotal->Caption = String(uTotalLevelEnergy);
}
//---------------------------------------------------------------------------

void __fastcall TFormCalculations::BitBtnChangeReferenceClick(TObject *Sender)
{
	m_pReference->Position = StrToInt(LabelNumberOfBattlesTotal->Caption);
}
//---------------------------------------------------------------------------

