object SettingsForm: TSettingsForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = #1053#1072#1083#1072#1096#1090#1091#1074#1072#1085#1085#1103' '#1089#1077#1088#1074#1077#1088#1091
  ClientHeight = 180
  ClientWidth = 258
  Color = clBtnFace
  DoubleBuffered = True
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object ServicePort: TLabeledEdit
    Left = 8
    Top = 24
    Width = 121
    Height = 21
    EditLabel.Width = 69
    EditLabel.Height = 13
    EditLabel.Caption = #1056#1086#1073#1086#1095#1080#1081' '#1087#1086#1088#1090
    TabOrder = 0
  end
  object StartMinimised: TCheckBox
    Left = 8
    Top = 99
    Width = 201
    Height = 17
    Caption = #1047#1072#1087#1091#1089#1082#1072#1090#1080' '#1091' '#1079#1075#1086#1088#1085#1091#1090#1086#1084#1091' '#1074#1080#1075#1083#1103#1076#1110
    TabOrder = 1
  end
  object Apply: TBitBtn
    Left = 8
    Top = 141
    Width = 90
    Height = 25
    Caption = #1047#1073#1077#1088#1077#1075#1090#1080
    Kind = bkOK
    NumGlyphs = 2
    TabOrder = 2
    OnClick = ApplyClick
  end
  object Cancel: TBitBtn
    Left = 147
    Top = 141
    Width = 97
    Height = 25
    Caption = #1057#1082#1072#1089#1091#1074#1072#1090#1080
    Kind = bkCancel
    NumGlyphs = 2
    TabOrder = 3
    OnClick = CancelClick
  end
  object EnableAutoStart: TCheckBox
    Left = 8
    Top = 64
    Width = 201
    Height = 17
    Caption = #1040#1074#1090#1086#1079#1072#1087#1091#1089#1082' '#1087#1088#1080' '#1074#1093#1086#1076#1110' '#1082#1086#1088#1080#1089#1090#1091#1074#1072#1095#1072
    TabOrder = 4
  end
end
