object FormPickColor: TFormPickColor
  Left = 0
  Top = 0
  BorderStyle = bsNone
  ClientHeight = 384
  ClientWidth = 384
  Color = clBlack
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWhite
  Font.Height = -19
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnActivate = FormActivate
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 23
  object ImageCapturedFrame: TImage
    Left = 0
    Top = 0
    Width = 384
    Height = 384
    Align = alClient
    OnClick = ImageCapturedFrameClick
    OnMouseMove = ImageCapturedFrameMouseMove
    ExplicitLeft = 288
    ExplicitTop = 96
    ExplicitWidth = 240
    ExplicitHeight = 217
  end
  object LabelColorInfo: TLabel
    Left = 24
    Top = 61
    Width = 65
    Height = 23
    Caption = 'COLOR:'
  end
  object LabelX: TLabel
    Left = 61
    Top = 26
    Width = 63
    Height = 23
    AutoSize = False
    Caption = '%'
  end
  object LabelXInfo: TLabel
    Left = 24
    Top = 24
    Width = 18
    Height = 23
    Caption = 'X:'
  end
  object LabelY: TLabel
    Left = 162
    Top = 24
    Width = 63
    Height = 23
    AutoSize = False
    Caption = '%'
  end
  object LabelYInfo: TLabel
    Left = 130
    Top = 24
    Width = 18
    Height = 23
    Caption = 'Y:'
  end
  object PanelColor: TPanel
    Left = 112
    Top = 55
    Width = 36
    Height = 36
    BevelKind = bkFlat
    BevelOuter = bvNone
    BorderWidth = 1
    Color = clWhite
    ParentBackground = False
    TabOrder = 0
  end
end
