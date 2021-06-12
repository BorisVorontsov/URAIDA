object FormCalculations: TFormCalculations
  Left = 0
  Top = 0
  BorderStyle = bsNone
  Caption = #1042#1099#1095#1080#1089#1083#1077#1085#1080#1103
  ClientHeight = 639
  ClientWidth = 461
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -16
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnCreate = FormCreate
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 19
  object PanelCalculations: TPanel
    Left = 0
    Top = 0
    Width = 461
    Height = 639
    Align = alClient
    BevelKind = bkFlat
    BevelOuter = bvNone
    TabOrder = 0
    ExplicitLeft = 208
    ExplicitTop = 192
    ExplicitWidth = 185
    ExplicitHeight = 41
    object Bevel1: TBevel
      Left = 8
      Top = 162
      Width = 441
      Height = 16
      Shape = bsBottomLine
    end
    object GroupBoxEnergy: TGroupBox
      Left = 8
      Top = 16
      Width = 441
      Height = 147
      Caption = #1042#1099#1095#1080#1089#1083#1077#1085#1080#1077' '#1101#1085#1077#1088#1075#1080#1080
      TabOrder = 0
      object LabelNumberOfBattles: TLabel
        Left = 11
        Top = 27
        Width = 137
        Height = 19
        AutoSize = False
        Caption = #1050#1086#1083#1080#1095#1077#1089#1090#1074#1086' '#1073#1086#1105#1074':'
      end
      object LabelBattleCost: TLabel
        Left = 11
        Top = 69
        Width = 126
        Height = 19
        AutoSize = False
        Caption = #1057#1090#1086#1080#1084#1086#1089#1090#1100' '#1073#1086#1103':'
      end
      object LabelTaskCostTotalInfo: TLabel
        Left = 266
        Top = 107
        Width = 57
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = #1048#1090#1086#1075#1086':'
      end
      object LabelTaskCostTotal: TLabel
        Left = 329
        Top = 107
        Width = 99
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = '%'
      end
      object EditNumberOfBattles: TEdit
        Left = 374
        Top = 24
        Width = 38
        Height = 27
        Alignment = taCenter
        ReadOnly = True
        TabOrder = 0
        Text = '1'
      end
      object UpDownNumberOfBattles: TUpDown
        Left = 412
        Top = 24
        Width = 16
        Height = 27
        Associate = EditNumberOfBattles
        Min = 1
        Max = 999
        Position = 1
        TabOrder = 1
      end
      object BitBtnCopyFromReference: TBitBtn
        Left = 327
        Top = 17
        Width = 41
        Height = 41
        Caption = #1050
        TabOrder = 2
        OnClick = BitBtnCopyFromReferenceClick
      end
      object ComboBoxBattleCost: TComboBox
        Left = 374
        Top = 66
        Width = 54
        Height = 27
        Style = csDropDownList
        TabOrder = 3
      end
      object BitBtnCalcTaskCost: TBitBtn
        Left = 219
        Top = 97
        Width = 41
        Height = 41
        Caption = '='
        TabOrder = 4
        OnClick = BitBtnCalcTaskCostClick
      end
    end
    object GroupBoxNumberOfBattles: TGroupBox
      Left = 8
      Top = 194
      Width = 441
      Height = 421
      Caption = #1042#1099#1095#1080#1089#1083#1077#1085#1080#1077' '#1073#1086#1105#1074
      Padding.Right = 8
      TabOrder = 1
      object LabelNumberOfBattlesTotalInfo: TLabel
        Left = 266
        Top = 323
        Width = 57
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = #1048#1090#1086#1075#1086':'
      end
      object LabelNumberOfBattlesTotal: TLabel
        Left = 329
        Top = 323
        Width = 99
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = '%'
      end
      object LabelNBSilverTotalInfo: TLabel
        Left = 120
        Top = 368
        Width = 203
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = #1041#1091#1076#1077#1090' '#1087#1086#1083#1091#1095#1077#1085#1086' '#1089#1077#1088#1077#1073#1088#1072':'
      end
      object LabelNBSilverTotal: TLabel
        Left = 329
        Top = 368
        Width = 99
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = '%'
      end
      object LabelNBEnergyTotalInfo: TLabel
        Left = 120
        Top = 393
        Width = 203
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = #1041#1091#1076#1077#1090' '#1087#1086#1090#1088#1072#1095#1077#1085#1086' '#1101#1085#1077#1088#1075#1080#1080':'
      end
      object LabelNBEnergyTotal: TLabel
        Left = 329
        Top = 393
        Width = 99
        Height = 19
        Alignment = taRightJustify
        AutoSize = False
        Caption = '%'
      end
      object GroupBoxMissionSetup: TGroupBox
        Left = 8
        Top = 24
        Width = 425
        Height = 169
        Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099' '#1084#1080#1089#1089#1080#1080' '#1082#1072#1084#1087#1072#1085#1080#1080
        TabOrder = 0
        object LabelDifficulty: TLabel
          Left = 8
          Top = 30
          Width = 160
          Height = 19
          AutoSize = False
          Caption = #1059#1088#1086#1074#1077#1085#1100' '#1089#1083#1086#1078#1085#1086#1089#1090#1080':'
        end
        object LabelChapter: TLabel
          Left = 8
          Top = 60
          Width = 54
          Height = 19
          AutoSize = False
          Caption = #1043#1083#1072#1074#1072':'
        end
        object LabelChapterLevel: TLabel
          Left = 7
          Top = 93
          Width = 73
          Height = 19
          AutoSize = False
          Caption = #1059#1088#1086#1074#1077#1085#1100':'
        end
        object Label4: TLabel
          Left = 7
          Top = 126
          Width = 161
          Height = 19
          AutoSize = False
          Caption = #1050#1086#1083#1080#1095#1077#1089#1090#1074#1086' '#1075#1077#1088#1086#1077#1074':'
        end
        object ComboBoxDifficulty: TComboBox
          Left = 259
          Top = 22
          Width = 156
          Height = 27
          Style = csDropDownList
          ItemIndex = 2
          TabOrder = 0
          Text = #1053#1077#1074#1086#1079#1084#1086#1078#1085#1099#1081
          Items.Strings = (
            #1054#1073#1099#1095#1085#1099#1081
            #1058#1088#1091#1076#1085#1099#1081
            #1053#1077#1074#1086#1079#1084#1086#1078#1085#1099#1081)
        end
        object ComboBoxChapter: TComboBox
          Left = 361
          Top = 57
          Width = 54
          Height = 27
          Style = csDropDownList
          ItemIndex = 11
          TabOrder = 1
          Text = '12'
          Items.Strings = (
            '1'
            '2'
            '3'
            '4'
            '5'
            '6'
            '7'
            '8'
            '9'
            '10'
            '11'
            '12')
        end
        object ComboBoxChapterLevel: TComboBox
          Left = 361
          Top = 90
          Width = 54
          Height = 27
          Style = csDropDownList
          ItemIndex = 2
          TabOrder = 2
          Text = '3'
          Items.Strings = (
            '1'
            '2'
            '3'
            '4'
            '5'
            '6'
            '7')
        end
        object ComboBoxHeroesCount: TComboBox
          Left = 361
          Top = 123
          Width = 54
          Height = 27
          Style = csDropDownList
          ItemIndex = 3
          TabOrder = 3
          Text = '4'
          Items.Strings = (
            '1'
            '2'
            '3'
            '4')
        end
      end
      object BitBtnChangeReference: TBitBtn
        Left = 220
        Top = 313
        Width = 41
        Height = 41
        Caption = #1042
        TabOrder = 1
        OnClick = BitBtnChangeReferenceClick
      end
      object CheckBoxXPBooster: TCheckBox
        Left = 11
        Top = 325
        Width = 126
        Height = 17
        Caption = #1041#1091#1089#1090#1077#1088' '#1086#1087#1099#1090#1072
        TabOrder = 2
      end
      object GroupBoxHeroesSetup: TGroupBox
        Left = 8
        Top = 196
        Width = 425
        Height = 106
        Caption = #1055#1072#1088#1072#1084#1077#1090#1088#1099' '#1075#1077#1088#1086#1077#1074
        TabOrder = 3
        object LabelHeroLevel: TLabel
          Left = 8
          Top = 35
          Width = 147
          Height = 19
          AutoSize = False
          Caption = #1059#1088#1086#1074#1077#1085#1100' '#1075#1077#1088#1086#1103'('#1077#1074'):'
        end
        object LabelDesiredHeroLevel: TLabel
          Left = 318
          Top = 27
          Width = 36
          Height = 19
          AutoSize = False
          Caption = #1076#1086':'
        end
        object LabelActualHeroLevel: TLabel
          Left = 224
          Top = 27
          Width = 36
          Height = 19
          AutoSize = False
          Caption = #1086#1090':'
        end
        object LabelHeroRank: TLabel
          Left = 8
          Top = 65
          Width = 46
          Height = 19
          AutoSize = False
          Caption = #1056#1072#1085#1075':'
        end
        object EditActualHeroLevel: TEdit
          Left = 255
          Top = 24
          Width = 38
          Height = 27
          Alignment = taCenter
          ReadOnly = True
          TabOrder = 0
          Text = '1'
        end
        object UpDownActualHeroLevel: TUpDown
          Left = 293
          Top = 24
          Width = 16
          Height = 27
          Associate = EditActualHeroLevel
          Min = 1
          Max = 59
          Position = 1
          TabOrder = 1
        end
        object EditDesiredHeroLevel: TEdit
          Left = 350
          Top = 24
          Width = 38
          Height = 27
          Alignment = taCenter
          ReadOnly = True
          TabOrder = 2
          Text = '2'
        end
        object UpDownDesiredHeroLevel: TUpDown
          Left = 388
          Top = 24
          Width = 16
          Height = 27
          Associate = EditDesiredHeroLevel
          Min = 2
          Max = 60
          Position = 2
          TabOrder = 3
        end
        object ComboBoxHeroRank: TComboBox
          Left = 306
          Top = 57
          Width = 98
          Height = 27
          Style = csDropDownList
          ItemIndex = 2
          TabOrder = 4
          Text = #9733#9733#9733
          OnChange = ComboBoxHeroRankChange
          Items.Strings = (
            #9733
            #9733#9733
            #9733#9733#9733
            #9733#9733#9733#9733
            #9733#9733#9733#9733#9733
            #9733#9733#9733#9733#9733#9733)
        end
      end
      object BitBtnCalcNumberOfBattles: TBitBtn
        Left = 175
        Top = 313
        Width = 41
        Height = 41
        Caption = '='
        TabOrder = 4
        OnClick = BitBtnCalcNumberOfBattlesClick
      end
    end
  end
end
