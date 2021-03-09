/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "..\..\work-functions\ThreadSafeLog.h"
#include "GuardThread.h"
#include "UpdateThread.h"
#include "GuardMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TGuardian *Guardian;

TThreadSafeLog *Log;
String DataPath, LogName, LogPath, AgentPath;
TDate DateStart;
TGuardThread *Guard;
TUpdateThread *Updater;

extern String UsedAppLogDir; //������� ��������� ��� ��������� ��� ������� � MyFunc.h

//---------------------------------------------------------------------------
__fastcall TGuardian::TGuardian(TComponent* Owner)
	: TForm(Owner)
{
  UsedAppLogDir = "AFAgent\\Log";

  Log = new TThreadSafeLog();

  DataPath = GetEnvironmentVariable("USERPROFILE") + "\\Documents\\AFAgent";
  LogPath = DataPath + "\\Log";
  DataPath += "\\Data";

  DateStart = Date().CurrentDate();
  LogName = DateToStr(Date()) + "_guard.log";
  AgentPath = GetAgentPath();
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::FormCreate(TObject *Sender)
{
  WindowState = wsMinimized;

  if (FileExists(LogPath + "\\" + LogName))
	Log->LoadFromFile(LogPath + "\\" + LogName);

  Log->Add("������� ������");

  Guard = new TGuardThread(true, AgentPath);
  Guard->Resume();

  Updater = new TUpdateThread(true, AgentPath, DataPath + "\\" + GetFileNameFromFilePath(AgentPath));

  SaveLogTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::FormClose(TObject *Sender, TCloseAction &Action)
{
  try
	 {
	   if (Guard && Guard->Started)
		 {
		   Guard->Terminate();

		   while (!Guard->Finished)
			 Sleep(100);

		   delete Guard;
		 }

	   if (Updater && Updater->Started)
		 {
		   Updater->Terminate();

		   while (!Updater->Finished)
			 Sleep(100);

		   delete Updater;
		 }

       Log->Add("ʳ���� ������");
	 }
  catch (Exception &e)
	 {
	   Log->Add("���������� ������: " + e.ToString());
	 }

  Log->SaveToFile(LogPath + "\\" + LogName);

  delete Log;
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::ApplicationEventsMessage(tagMSG &Msg, bool &Handled)
{
  //������������ ����������� �� ������ �� ����� ���������
  if ((Msg.message == WM_KEYDOWN) && (Msg.wParam == VK_RETURN))
	Updater->Resume();
}
//---------------------------------------------------------------------------

String __fastcall TGuardian::GetAgentPath()
{
  String result = "";
  TRegistry *reg = new TRegistry(KEY_READ);

  try
	 {
	   reg->RootKey = HKEY_CURRENT_USER;

       if (reg->OpenKey("Software\\ArmFileAgent", false))
		 {
		   if (reg->ValueExists("ModulePath"))
			 result = reg->ReadString("ModulePath");

		   reg->CloseKey();
		 }
	 }
  __finally {delete reg;}

  return result;
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::SaveLogTimerTimer(TObject *Sender)
{
  TStringStream *ss = new TStringStream("", TEncoding::UTF8, true);

  if (!FileExists(LogPath + "\\" + LogName))
	SaveToFile(LogPath + "\\" + LogName, "");

  TFileStream *fs = new TFileStream(LogPath + "\\" + LogName, fmOpenReadWrite);

  try
	 {
	   Log->SaveToStream(ss);

	   if (ss->Size > fs->Size)
		 {
		   fs->Position = fs->Size;
		   ss->Position = fs->Size;
		   fs->Write(ss->Bytes, ss->Position, ss->Size - ss->Position);
		 }
	 }
  __finally {delete ss; delete fs;}

  if (Date().CurrentDate() > DateStart)
	{
	  Log->Clear();
	  LogName = DateToStr(Date()) + "_guard.log";
	  DateStart = Date().CurrentDate();
	}
}
//---------------------------------------------------------------------------

