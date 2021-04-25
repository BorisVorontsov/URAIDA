object FormPickPoint: TFormPickPoint
  Left = 0
  Top = 0
  BorderIcons = []
  BorderStyle = bsNone
  ClientHeight = 285
  ClientWidth = 262
  Color = clBtnFace
  Constraints.MinHeight = 256
  Constraints.MinWidth = 256
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWhite
  Font.Height = -16
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 19
  object PanelCapturedFrame: TPanel
    Left = 0
    Top = 0
    Width = 262
    Height = 285
    Align = alClient
    BevelOuter = bvNone
    Color = clGreen
    DoubleBuffered = True
    ParentBackground = False
    ParentDoubleBuffered = False
    TabOrder = 0
    object ImageCapturedFrame: TImage
      Left = 0
      Top = 0
      Width = 262
      Height = 285
      Align = alClient
      OnClick = ImageCapturedFrameClick
      OnMouseMove = ImageCapturedFrameMouseMove
      ExplicitLeft = -6
      ExplicitTop = -29
      ExplicitWidth = 384
      ExplicitHeight = 384
    end
    object LabelColorInfo: TLabel
      Left = 24
      Top = 74
      Width = 41
      Height = 19
      Caption = #1062#1074#1077#1090':'
    end
    object LabelX: TLabel
      Left = 85
      Top = 24
      Width = 63
      Height = 23
      AutoSize = False
      Caption = '%'
    end
    object LabelXInfo: TLabel
      Left = 24
      Top = 24
      Width = 15
      Height = 19
      Caption = 'X:'
    end
    object LabelY: TLabel
      Left = 85
      Top = 49
      Width = 63
      Height = 23
      AutoSize = False
      Caption = '%'
    end
    object LabelYInfo: TLabel
      Left = 23
      Top = 49
      Width = 16
      Height = 19
      Caption = 'Y:'
    end
    object LabelColorInRGB: TLabel
      Left = 114
      Top = 76
      Width = 177
      Height = 19
      AutoSize = False
      Caption = 'R: % G: % B: %'
      Transparent = True
    end
    object PanelColor: TPanel
      Left = 85
      Top = 76
      Width = 20
      Height = 20
      BevelKind = bkFlat
      BevelOuter = bvNone
      BorderWidth = 1
      Color = clWhite
      ParentBackground = False
      TabOrder = 0
      StyleElements = []
    end
  end
  object PanelMenu: TPanel
    Left = 0
    Top = 0
    Width = 262
    Height = 285
    Align = alClient
    BevelKind = bkFlat
    BevelOuter = bvNone
    TabOrder = 1
    object LabelSource: TLabel
      Left = 36
      Top = 71
      Width = 150
      Height = 19
      Caption = #1042#1099#1073#1077#1088#1080#1090#1077' '#1080#1089#1090#1086#1095#1085#1080#1082':'
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object ButtonSourceFromFile: TButton
      Left = 36
      Top = 143
      Width = 186
      Height = 41
      Caption = #1047#1072#1075#1088#1091#1079#1082#1072' '#1080#1079' '#1092#1072#1081#1083#1072'...'
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
      OnClick = ButtonSourceFromFileClick
    end
    object ButtonSourceFromGame: TButton
      Left = 36
      Top = 96
      Width = 186
      Height = 41
      Caption = #1047#1072#1093#1074#1072#1090' '#1080#1079' '#1080#1075#1088#1099
      Default = True
      Font.Charset = RUSSIAN_CHARSET
      Font.Color = clWindowText
      Font.Height = -16
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
      OnClick = ButtonSourceFromGameClick
    end
  end
  object FileOpenDialog1: TFileOpenDialog
    FavoriteLinks = <>
    FileTypes = <
      item
        DisplayName = 'JPEG files'
        FileMask = '*.jpg;*.jpeg'
      end
      item
        DisplayName = 'PNG files'
        FileMask = '*.png'
      end
      item
        DisplayName = 'Windows Bitmaps'
        FileMask = '*.bmp'
      end>
    Options = [fdoStrictFileTypes, fdoFileMustExist, fdoForcePreviewPaneOn]
    Title = #1042#1099#1073#1077#1088#1080#1090#1077' '#1080#1089#1090#1086#1095#1085#1080#1082' '#1076#1083#1103' '#1082#1086#1085#1090#1088#1086#1083#1100#1085#1086#1081' '#1090#1086#1095#1082#1080
    Left = 16
    Top = 184
  end
end
