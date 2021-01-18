object AddGroupForm: TAddGroupForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = #1057#1090#1074#1086#1088#1080#1090#1080'/'#1088#1077#1076#1072#1075#1091#1074#1072#1090#1080' '#1075#1088#1091#1087#1091
  ClientHeight = 58
  ClientWidth = 298
  Color = clBtnFace
  DoubleBuffered = True
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesigned
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Name: TLabeledEdit
    Left = 8
    Top = 24
    Width = 177
    Height = 21
    EditLabel.Width = 59
    EditLabel.Height = 13
    EditLabel.Caption = #1042#1082#1072#1078#1110#1090#1100' '#1110#1084#39#1103
    TabOrder = 0
  end
  object Apply: TBitBtn
    Left = 200
    Top = 22
    Width = 90
    Height = 25
    Caption = #1047#1073#1077#1088#1077#1075#1090#1080
    Kind = bkOK
    NumGlyphs = 2
    TabOrder = 1
    OnClick = ApplyClick
  end
end
