object Guardian: TGuardian
  Left = 0
  Top = 0
  Caption = 'ArmAgent Guardian'
  ClientHeight = 120
  ClientWidth = 261
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object ApplicationEvents: TApplicationEvents
    OnMessage = ApplicationEventsMessage
    Left = 32
    Top = 8
  end
  object SaveLogTimer: TTimer
    Enabled = False
    Interval = 120000
    OnTimer = SaveLogTimerTimer
    Left = 112
    Top = 8
  end
end
