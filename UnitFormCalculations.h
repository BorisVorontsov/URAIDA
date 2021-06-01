//---------------------------------------------------------------------------

#ifndef UnitFormCalculationsH
#define UnitFormCalculationsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
//---------------------------------------------------------------------------
class TFormCalculations : public TForm
{
__published:	// IDE-managed Components
	TLabel *LabelBattleCost;
	TLabel *LabelBCTotalInfo;
	TLabel *LabelBCTotal;
	TEdit *EditBattleCost;
	TUpDown *UpDownBattleCost;
	TLabel *LabelNumberOfBattles;
	TEdit *EditNumberOfBattles;
	TUpDown *UpDownNumberOfBattles;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall UpDownNumberOfBattlesChangingEx(TObject *Sender, bool &AllowChange,
          int NewValue, TUpDownDirection Direction);
	void __fastcall UpDownBattleCostChangingEx(TObject *Sender, bool &AllowChange, int NewValue,
          TUpDownDirection Direction);

private:	// User declarations
	void CalculateBattleCost(unsigned int nNumberOfBattles, unsigned int uBattleCost);

public:		// User declarations
	__fastcall TFormCalculations(TComponent* Owner);

    void Execute(TForm* pParent);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormCalculations *FormCalculations;
//---------------------------------------------------------------------------
#endif
