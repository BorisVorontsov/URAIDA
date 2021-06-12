//---------------------------------------------------------------------------

#ifndef UnitFormCalculationsH
#define UnitFormCalculationsH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <array>
//---------------------------------------------------------------------------

//Ќаграды и стоимость бо€ по картам кампании
typedef struct tagCampaignLevelInfo
{
	unsigned int uXP;
	unsigned int uSilver;
	unsigned int uBattleCost;
} CampaignLevelInfo;

const unsigned int g_uLevelsPerMap = 7;

typedef struct tagMapTable
{
	unsigned int uDifficultyLevel;
	std::array<CampaignLevelInfo, g_uLevelsPerMap> LevelInfo;
} MapTable;

const unsigned int g_uMaxDifficultyLevels = 3;

typedef struct tagCampaignTable
{
	std::array<MapTable, g_uMaxDifficultyLevels> MapTable;
} CampaignTable;

const unsigned int g_uMaxMaps = 12;

//—водна€ таблица с данными (сложность: опыт, серебро, стоимость бо€)
static std::array<CampaignTable, g_uMaxMaps> g_CampaignTable =
{
{
//1
{
	1, {{ {640, 750, 4}, {1600, 800, 4}, {1606, 850, 4}, {2254, 900, 4}, {2260, 950, 4}, {2264, 1000, 4}, {1624, 1200, 5} }},
	2, {{ {4560, 4500, 6}, {4584, 4550, 6}, {4608, 4600, 6}, {4624, 4650, 6}, {4654, 4700, 6}, {4684, 4750, 6}, {4116, 4950, 7} }},
	3, {{ {9600, 5500, 8}, {9600, 5550, 8}, {9600, 5600, 8}, {9600, 5650, 8}, {9600, 5700, 8}, {9600, 5750, 8}, {8400, 5950, 9} }}
},

//2
{
	1, {{ {2288, 1200, 4}, {2296, 1250, 4}, {2304, 1300, 4}, {2320, 1350, 4}, {2336, 1400, 4}, {2352, 1450, 4}, {1684, 1650, 5} }},
	2, {{ {4752, 4950, 6}, {4768, 5000, 6}, {4768, 5050, 6}, {4808, 5100, 6}, {4894, 5150, 6}, {4864, 5200, 6}, {4344, 5400, 7} }},
	3, {{ {9600, 5950, 8}, {9600, 6000, 8}, {9600, 6050, 8}, {9600, 6100, 8}, {9600, 6150, 8}, {9600, 6200, 8}, {8400, 6400, 9} }}
},

//3
{
	1, {{ {3060, 1650, 4}, {3096, 1700, 4}, {3132, 1750, 4}, {3184, 1800, 4}, {3208, 1850, 4}, {3232, 1900, 4}, {2908, 2100, 5} }},
	2, {{ {5564, 5400, 6}, {5670, 5450, 6}, {5670, 5500, 6}, {5778, 5550, 6}, {5778, 5600, 6}, {5814, 5650, 6}, {5232, 5850, 7} }},
	3, {{ {10800, 6400, 8}, {10800, 6450, 8}, {10800, 6500, 8}, {10800, 6550, 8}, {10800, 6600, 8}, {10800, 6650, 8}, {9600, 6850, 9} }}
},

//4
{
	1, {{ {3640, 2100, 4}, {3656, 2150, 4}, {3696, 2200, 4}, {3716, 2250, 4}, {3758, 2300, 4}, {3782, 2350, 4}, {3788, 2550, 5} }},
	2, {{ {6576, 5850, 6}, {6616, 5900, 6}, {6660, 5950, 6}, {6664, 6000, 6}, {6696, 6050, 6}, {6744, 6100, 6}, {6723, 6300, 7} }},
	3, {{ {12000, 6850, 8}, {12000, 6900, 8}, {12000, 6950, 8}, {12000, 7000, 8}, {12000, 7050, 8}, {12000, 7100, 8}, {12000, 7300, 9} }}
},

//5
{
	1, {{ {4180, 2550, 4}, {4222, 2600, 4}, {4246, 2650, 4}, {4288, 2700, 4}, {4312, 2750, 4}, {4354, 2800, 4}, {4366, 3000, 5} }},
	2, {{ {7458, 6300, 6}, {7458, 6350, 6}, {7458, 6400, 6}, {7556, 6450, 6}, {7616, 6500, 6}, {7616, 6550, 6}, {7584, 6750, 7} }},
	3, {{ {13200, 7300, 8}, {13200, 7350, 8}, {13200, 7400, 8}, {13200, 7450, 8}, {13200, 7500, 8}, {13200, 7550, 8}, {13200, 7750, 9} }}
},

//6
{
	1, {{ {4816, 3000, 4}, {4840, 3050, 4}, {4872, 3100, 4}, {4920, 3150, 4}, {4926, 3200, 4}, {4944, 3250, 4}, {4576, 3450, 5} }},
	2, {{ {8448, 6750, 6}, {8448, 6800, 6}, {8448, 6850, 6}, {8576, 6900, 6}, {8592, 6950, 6}, {8640, 7000, 6}, {7888, 7200, 7} }},
	3, {{ {14400, 7750, 8}, {14400, 7800, 8}, {14400, 7850, 8}, {14400, 7900, 8}, {14400, 7950, 8}, {14400, 8000, 8}, {14400, 8200, 9} }}
},

//7
{
	1, {{ {4816, 3450, 4}, {5368, 3500, 4}, {5136, 3550, 4}, {5160, 3600, 4}, {5188, 3650, 4}, {5230, 3700, 4}, {5268, 3900, 5} }},
	2, {{ {8784, 7200, 6}, {8784, 7250, 6}, {8784, 7300, 6}, {8800, 7350, 6}, {8824, 7400, 6}, {8952, 7450, 6}, {8928, 7650, 7} }},
	3, {{ {14000, 8200, 8}, {14000, 8250, 8}, {14000, 8300, 8}, {14000, 8350, 8}, {14000, 8400, 8}, {14000, 8450, 8}, {14000, 8650, 9} }}
},

//8
{
	1, {{ {5328, 3900, 4}, {5368, 3950, 4}, {5390, 4000, 4}, {5424, 4050, 4}, {5472, 4100, 4}, {5512, 4150, 4}, {5528, 4350, 5} }},
	2, {{ {9144, 7650, 6}, {9144, 7700, 6}, {9144, 7750, 6}, {9142, 7800, 6}, {9232, 7850, 6}, {9304, 7900, 6}, {9330, 8100, 7} }},
	3, {{ {14000, 8650, 8}, {14000, 8700, 8}, {14000, 8750, 8}, {14000, 8800, 8}, {14000, 8850, 8}, {14000, 8900, 8}, {14000, 9100, 9} }}
},

//9
{
	1, {{ {6048, 4350, 4}, {6080, 4400, 4}, {6080, 4450, 4}, {6160, 4500, 4}, {6200, 4550, 4}, {6240, 4600, 4}, {4382, 4800, 5} }},
	2, {{ {10208, 8100, 6}, {10264, 8150, 6}, {10264, 8200, 6}, {10320, 8250, 6}, {10320, 8300, 6}, {10320, 8350, 6}, {7288, 8550, 7} }},
	3, {{ {15600, 9100, 8}, {15600, 9150, 8}, {15600, 9200, 8}, {15600, 9250, 8}, {15600, 9300, 8}, {15600, 9350, 8}, {10800, 9550, 9} }}
},

//10
{
	1, {{ {6392, 4800, 4}, {6432, 4850, 4}, {6432, 4900, 4}, {6528, 4950, 4}, {6568, 5000, 4}, {6568, 5050, 4}, {5720, 5250, 5} }},
	2, {{ {10624, 8550, 6}, {10696, 8600, 6}, {10696, 8650, 6}, {10768, 8700, 6}, {10768, 8750, 6}, {10768, 8800, 6}, {9304, 9000, 7} }},
	3, {{ {15704, 9550, 8}, {15784, 9600, 8}, {15784, 9650, 8}, {15864, 9700, 8}, {15864, 9750, 8}, {15864, 9800, 8}, {13664, 10000, 9} }}
},

//11
{
	1, {{ {6808, 5250, 4}, {6848, 5300, 4}, {7092, 5350, 4}, {6944, 5400, 4}, {6984, 5450, 4}, {7024, 5500, 4}, {5520, 5700, 5} }},
	2, {{ {11320, 9000, 6}, {11392, 9050, 6}, {11392, 9100, 6}, {11556, 9150, 6}, {11628, 9200, 6}, {11700, 9250, 6}, {10074, 9450, 7} }},
	3, {{ {17120, 10000, 8}, {17168, 10050, 8}, {17168, 10100, 8}, {17264, 10150, 8}, {17304, 10200, 8}, {17344, 10250, 8}, {14520, 10450, 9} }}
},

//12
{
	1, {{ {7080, 5700, 4}, {7128, 5750, 4}, {7128, 5800, 4}, {7240, 5850, 4}, {7288, 5900, 4}, {7336, 5950, 4}, {5190, 6150, 5} }},
	2, {{ {11800, 9450, 6}, {11800, 9500, 6}, {11800, 9550, 6}, {11800, 9600, 6}, {11800, 9650, 6}, {11800, 9700, 6}, {11800, 9900, 7} }},
	3, {{ {17168, 10450, 8}, {17168, 10500, 8}, {17304, 10550, 8}, {17432, 10600, 8}, {17560, 10650, 8}, {17600, 10700, 8}, {12240, 10900, 9} }}
}
}
};

//“ребовани€ опыта на каждый уровень геро€ (по рангам)
static std::array<unsigned int, 9> g_Rank1HeroXP =
	{897, 1118, 1393, 1736, 2163, 2695, 3359, 4185, 5215};

static std::array<unsigned int, 19> g_Rank2HeroXP[] =
	{1110, 1258, 1426, 1616, 1831, 2075, 2351, 2664, 3019, 3421, 3876, 4392, 4977, 5640, 6390, 7241, 8205, 9298, 10536};

static std::array<unsigned int, 29> g_Rank3HeroXP[] =
	{1279, 1411, 1556, 1717, 1893, 2088, 2303, 2541, 2802, 3091, 3409, 3760, 4147, 4574, 5045, 5565, 6138, 6769, 7466,
	8235, 9083, 10018, 11050, 12188, 13442, 14827, 16353, 18037, 19894};

static std::array<unsigned int, 39> g_Rank4HeroXP[] =
	{1460, 1591, 1734, 1890, 2060, 2245, 2447, 2666, 2906, 3167, 3451, 3761, 4099, 4467, 4868, 5305, 5782, 6301, 6867,
	7483, 8155, 8888, 9686, 10556, 11504, 12537, 13663, 14890, 16227, 17684, 19272, 21003, 22889, 24945, 27185, 29627,
	32287, 35187, 38347};

static std::array<unsigned int, 49> g_Rank5HeroXP[] =
	{1625, 1760, 1907, 2066, 2238, 2424, 2626, 2845, 3082, 3338, 3616, 3918, 4244, 4597, 4980, 5395, 5844, 6331, 6858,
	7430, 8048, 8719, 9445, 10231, 11084, 12007, 13007, 14090, 15264, 16535, 17912, 19404, 21020, 22770, 24667, 26721,
	28947, 31358, 33970, 36799, 39864, 43184, 46780, 50677, 54897, 59470, 64423, 69788, 75601};

static std::array<unsigned int, 59> g_Rank6HeroXP[] =
	{1813, 1956, 2110, 2277, 2457, 2651, 2860, 3086, 3329, 3592, 3876, 4182, 4512, 4869, 5253, 5668, 6115, 6598, 7119,
	7681, 8288, 8942, 9648, 10410, 11232, 12119, 13076, 14109, 15223, 16425, 17722, 19121, 20631, 22260, 24018, 25914,
	27961, 30168, 32551, 35121, 37894, 40886, 44115, 47598, 51357, 55412, 59787, 64508, 69602, 75098, 81028, 87426,
	94329, 101778, 109815, 118486, 127842, 137937, 148828};

class TFormCalculations : public TForm
{
__published:	// IDE-managed Components
	TPanel *PanelCalculations;
	TBevel *Bevel1;
	TGroupBox *GroupBoxEnergy;
	TLabel *LabelNumberOfBattles;
	TEdit *EditNumberOfBattles;
	TUpDown *UpDownNumberOfBattles;
	TBitBtn *BitBtnCopyFromReference;
	TLabel *LabelBattleCost;
	TComboBox *ComboBoxBattleCost;
	TLabel *LabelTaskCostTotalInfo;
	TLabel *LabelTaskCostTotal;
	TGroupBox *GroupBoxNumberOfBattles;
	TGroupBox *GroupBoxMissionSetup;
	TLabel *LabelDifficulty;
	TComboBox *ComboBoxDifficulty;
	TLabel *LabelChapter;
	TComboBox *ComboBoxChapter;
	TLabel *LabelChapterLevel;
	TComboBox *ComboBoxChapterLevel;
	TLabel *Label4;
	TComboBox *ComboBoxHeroesCount;
	TBitBtn *BitBtnChangeReference;
	TLabel *LabelNumberOfBattlesTotalInfo;
	TLabel *LabelNumberOfBattlesTotal;
	TCheckBox *CheckBoxXPBooster;
	TGroupBox *GroupBoxHeroesSetup;
	TLabel *LabelHeroLevel;
	TEdit *EditActualHeroLevel;
	TUpDown *UpDownActualHeroLevel;
	TLabel *LabelDesiredHeroLevel;
	TEdit *EditDesiredHeroLevel;
	TUpDown *UpDownDesiredHeroLevel;
	TLabel *LabelActualHeroLevel;
	TLabel *LabelHeroRank;
	TComboBox *ComboBoxHeroRank;
	TBitBtn *BitBtnCalcNumberOfBattles;
	TBitBtn *BitBtnCalcTaskCost;
	TLabel *LabelNBSilverTotalInfo;
	TLabel *LabelNBSilverTotal;
	TLabel *LabelNBEnergyTotalInfo;
	TLabel *LabelNBEnergyTotal;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall BitBtnCopyFromReferenceClick(TObject *Sender);
	void __fastcall ComboBoxHeroRankChange(TObject *Sender);
	void __fastcall BitBtnCalcTaskCostClick(TObject *Sender);
	void __fastcall BitBtnCalcNumberOfBattlesClick(TObject *Sender);
	void __fastcall BitBtnChangeReferenceClick(TObject *Sender);


private:	// User declarations
	TUpDown* m_pReference;

public:		// User declarations
	__fastcall TFormCalculations(TComponent* Owner);

	void Execute(TForm* pParent, TUpDown* pReference = nullptr);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormCalculations *FormCalculations;
//---------------------------------------------------------------------------
#endif
