object FormCalculations: TFormCalculations
  Left = 0
  Top = 0
  BorderStyle = bsDialog
  Caption = #1042#1099#1095#1080#1089#1083#1077#1085#1080#1103
  ClientHeight = 42
  ClientWidth = 553
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -16
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 19
  object LabelBattleCost: TLabel
    Left = 211
    Top = 11
    Width = 115
    Height = 19
    AutoSize = False
    Caption = #1057#1090#1086#1080#1084#1086#1089#1090#1100' '#1073#1086#1103':'
  end
  object LabelBCTotalInfo: TLabel
    Left = 411
    Top = 11
    Width = 57
    Height = 19
    AutoSize = False
    Caption = #1048#1090#1086#1075#1086':'
  end
  object LabelBCTotal: TLabel
    Left = 467
    Top = 11
    Width = 78
    Height = 19
    AutoSize = False
    Caption = '%'
  end
  object LabelNumberOfBattles: TLabel
    Left = 8
    Top = 11
    Width = 137
    Height = 19
    AutoSize = False
    Caption = #1050#1086#1083#1080#1095#1077#1089#1090#1074#1086' '#1073#1086#1105#1074':'
  end
  object EditBattleCost: TEdit
    Left = 332
    Top = 8
    Width = 38
    Height = 27
    Alignment = taCenter
    ReadOnly = True
    TabOrder = 2
    Text = '1'
  end
  object UpDownBattleCost: TUpDown
    Left = 370
    Top = 8
    Width = 16
    Height = 27
    Associate = EditBattleCost
    Min = 1
    Position = 1
    TabOrder = 3
    OnChangingEx = UpDownBattleCostChangingEx
  end
  object EditNumberOfBattles: TEdit
    Left = 146
    Top = 8
    Width = 38
    Height = 27
    Alignment = taCenter
    ReadOnly = True
    TabOrder = 0
    Text = '1'
  end
  object UpDownNumberOfBattles: TUpDown
    Left = 184
    Top = 8
    Width = 16
    Height = 27
    Associate = EditNumberOfBattles
    Min = 1
    Max = 999
    Position = 1
    TabOrder = 1
    OnChangingEx = UpDownNumberOfBattlesChangingEx
  end
end
