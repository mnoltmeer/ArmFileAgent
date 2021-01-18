object FirstStartForm: TFirstStartForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = #1055#1077#1088#1096#1080#1081' '#1079#1072#1087#1091#1089#1082' '#1060#1072#1081#1083#1086#1074#1086#1075#1086' '#1072#1075#1077#1085#1090#1091
  ClientHeight = 608
  ClientWidth = 411
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object LbStationID: TLabel
    Left = 80
    Top = 8
    Width = 65
    Height = 16
    Caption = 'LbStationID'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object Label1: TLabel
    Left = 9
    Top = 8
    Width = 60
    Height = 16
    Caption = 'ID '#1089#1090#1072#1085#1094#1110#1111':'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Tahoma'
    Font.Style = []
    ParentFont = False
  end
  object MigrationInitPanel: TPanel
    Left = 8
    Top = 215
    Width = 393
    Height = 146
    TabOrder = 2
    object Label2: TLabel
      Left = 152
      Top = 55
      Width = 65
      Height = 16
      Caption = 'LbStationID'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label3: TLabel
      Left = 12
      Top = 55
      Width = 68
      Height = 16
      Caption = #1030#1085#1076#1077#1082#1089' '#1042#1055#1047':'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label4: TLabel
      Left = 152
      Top = 77
      Width = 65
      Height = 16
      Caption = 'LbStationID'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label5: TLabel
      Left = 12
      Top = 77
      Width = 132
      Height = 16
      Caption = #1055#1086#1088#1090' '#1072#1076#1084#1110#1085#1110#1089#1090#1088#1091#1074#1072#1085#1085#1103':'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label6: TLabel
      Left = 152
      Top = 99
      Width = 65
      Height = 16
      Caption = 'LbStationID'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label7: TLabel
      Left = 12
      Top = 99
      Width = 65
      Height = 16
      Caption = #1040#1074#1090#1086#1089#1090#1072#1088#1090':'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label8: TLabel
      Left = 12
      Top = 121
      Width = 134
      Height = 16
      Caption = #1044#1083#1103' '#1074#1089#1110#1093' '#1082#1086#1088#1080#1089#1090#1091#1074#1072#1095#1110#1074':'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object Label9: TLabel
      Left = 152
      Top = 121
      Width = 65
      Height = 16
      Caption = 'LbStationID'
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Tahoma'
      Font.Style = []
      ParentFont = False
    end
    object ManagerCfgPath: TLabeledEdit
      Left = 8
      Top = 28
      Width = 338
      Height = 21
      EditLabel.Width = 88
      EditLabel.Height = 13
      EditLabel.Caption = #1064#1083#1103#1093' '#1076#1086' main.cfg'
      TabOrder = 0
    end
    object OpenManagerCfg: TButton
      Left = 352
      Top = 26
      Width = 25
      Height = 25
      Caption = '...'
      TabOrder = 1
    end
  end
  object InitKind: TRadioGroup
    Left = 10
    Top = 80
    Width = 393
    Height = 105
    Caption = #1054#1073#1077#1088#1110#1090#1100' '#1090#1080#1087' '#1110#1085#1110#1094#1110#1072#1083#1110#1079#1072#1094#1110#1111
    ItemIndex = 0
    Items.Strings = (
      #1056#1091#1095#1085#1072' ('#1074#1074#1077#1089#1090#1080' '#1087#1072#1088#1072#1084#1077#1090#1088#1080')'
      #1052#1110#1075#1088#1072#1094#1110#1103' ('#1074#1080#1082#1086#1088#1080#1089#1090#1072#1090#1080' '#1082#1086#1085#1092#1110#1075' '#1040#1057' "'#1052#1077#1085#1077#1076#1078#1077#1088' '#1092#1072#1081#1083#1110#1074' '#1040#1056#1052' '#1042#1047'")')
    TabOrder = 0
    OnClick = InitKindClick
  end
  object ManualInitPanel: TPanel
    Left = 8
    Top = 377
    Width = 393
    Height = 146
    TabOrder = 1
    object Index: TLabeledEdit
      Left = 8
      Top = 31
      Width = 81
      Height = 21
      EditLabel.Width = 56
      EditLabel.Height = 13
      EditLabel.Caption = #1030#1085#1076#1077#1082#1089' '#1042#1055#1047
      TabOrder = 0
    end
    object RemoteAdminPort: TLabeledEdit
      Left = 8
      Top = 76
      Width = 81
      Height = 21
      EditLabel.Width = 204
      EditLabel.Height = 13
      EditLabel.Caption = #1055#1086#1088#1090' '#1076#1083#1103' '#1074#1110#1076#1076#1072#1083#1077#1085#1085#1086#1075#1086' '#1072#1076#1084#1110#1085#1110#1089#1090#1088#1091#1074#1072#1085#1085#1103
      TabOrder = 1
    end
    object EnableAutoStart: TCheckBox
      Left = 8
      Top = 111
      Width = 193
      Height = 17
      Caption = #1040#1074#1090#1086#1084#1072#1090#1080#1095#1085#1086' '#1079#1072#1087#1091#1089#1082#1072#1090#1080' '#1040#1075#1077#1085#1090#1072
      TabOrder = 2
      OnClick = EnableAutoStartClick
    end
    object AllUsersAutoStart: TCheckBox
      Left = 240
      Top = 111
      Width = 137
      Height = 17
      Caption = #1044#1083#1103' '#1074#1089#1110#1093' '#1082#1086#1088#1080#1089#1090#1091#1074#1072#1095#1110#1074
      TabOrder = 3
    end
  end
  object Cancel: TBitBtn
    Left = 306
    Top = 561
    Width = 97
    Height = 25
    Caption = #1057#1082#1072#1089#1091#1074#1072#1090#1080
    Kind = bkCancel
    NumGlyphs = 2
    TabOrder = 3
    OnClick = CancelClick
  end
  object Apply: TBitBtn
    Left = 8
    Top = 561
    Width = 90
    Height = 25
    Caption = #1047#1073#1077#1088#1077#1075#1090#1080
    Kind = bkOK
    NumGlyphs = 2
    TabOrder = 4
    OnClick = ApplyClick
  end
  object CfgServerHost: TLabeledEdit
    Left = 8
    Top = 53
    Width = 308
    Height = 21
    EditLabel.Width = 149
    EditLabel.Height = 13
    EditLabel.Caption = #1040#1076#1088#1077#1089#1072' '#1089#1077#1088#1074#1077#1088#1091' '#1082#1086#1085#1092#1110#1075#1091#1088#1072#1094#1110#1081
    TabOrder = 5
  end
  object CfgServerPort: TLabeledEdit
    Left = 322
    Top = 53
    Width = 81
    Height = 21
    EditLabel.Width = 25
    EditLabel.Height = 13
    EditLabel.Caption = #1055#1086#1088#1090
    TabOrder = 6
  end
  object OpenCfgDialog: TOpenDialog
    Filter = #1082#1086#1085#1092#1110#1075#1091#1088#1072#1094#1110#1081#1085#1110' '#1092#1072#1081#1083#1080' '#1052#1077#1085#1077#1076#1078#1077#1088#1091'|main.cfg'
    Left = 239
  end
end
