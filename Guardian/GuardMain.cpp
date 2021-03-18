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

extern String UsedAppLogDir; //вказуємо директорію для логування для функцій з MyFunc.h

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

  Log->Add("Початок роботи");

  Guard = new TGuardThread(true, AgentPath);
  Guard->Resume();

  SaveLogTimer->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::FormClose(TObject *Sender, TCloseAction &Action)
{
  try
	 {
	   if (Guard)
		 {
		   Guard->Terminate();
		   HANDLE hGuard = reinterpret_cast<HANDLE>(Guard->Handle);

		   DWORD wait = WaitForSingleObject(hGuard, 300);

		   if (wait == WAIT_TIMEOUT)
			 TerminateThread(hGuard, 0);

		   delete Guard;
		 }

       Log->Add("Кінець роботи");
	 }
  catch (Exception &e)
	 {
	   Log->Add("Завершення роботи: " + e.ToString());
	 }

  Log->SaveToFile(LogPath + "\\" + LogName);

  delete Log;
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

void __fastcall TGuardian::WndProc(Messages::TMessage& Msg)
{
  if ((Msg.Msg == WM_KEYDOWN) && (Msg.WParam == VK_RETURN))
	{
      Log->Add("Надійшов запит на оновлення");

      try
		 {
           Updater = new TUpdateThread(AgentPath, DataPath + "\\" + GetFileNameFromFilePath(AgentPath));

		   Updater->Resume();
		 }
	  catch (Exception &e)
		 {
		   Log->Add("Ініціалізація процесу оновлення: " + e.ToString());
		 }
	}

  TForm::WndProc(Msg);
}
//---------------------------------------------------------------------------

