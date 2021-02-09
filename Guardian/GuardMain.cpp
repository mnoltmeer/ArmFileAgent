/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "..\..\work-functions\ThreadSafeLog.h"
#include "GuardThread.h"
#include "GuardMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TGuardian *Guardian;

TThreadSafeLog *Log;
String DataPath, LogName, LogPath, AgentName;
TDate DateStart;
TGuardThread *Guard;
//---------------------------------------------------------------------------
__fastcall TGuardian::TGuardian(TComponent* Owner)
	: TForm(Owner)
{
  Log = new TThreadSafeLog();

  DataPath = GetEnvironmentVariable("USERPROFILE") + "\\Documents\\AFAgent";
  LogPath = DataPath + "\\Log";
  DateStart = Date().CurrentDate();
  LogName = DateToStr(Date()) + "_guard.log";
  AgentName = GetAgentName();
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::FormCreate(TObject *Sender)
{
  Guard = new TGuardThread(true, AgentName);
  Guard->Resume();
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
	 }
  catch (Exception &e)
	 {
	   Log->Add("Завершення роботи: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::ApplicationEventsMessage(tagMSG &Msg, bool &Handled)
{
  //перехоплення повідомлення від Агента на старт оновлення
  if ((Msg.message == WM_KEYDOWN) && (Msg.wParam == VK_RETURN))
	{
      Log->Add("Старт оновлення Агента");
	  UpdateAgent();
    }
}
//---------------------------------------------------------------------------

String __fastcall TGuardian::GetAgentName()
{
  String result = "";
  TRegistry *reg = new TRegistry(KEY_READ);

  try
	 {
	   reg->RootKey = HKEY_LOCAL_MACHINE;

       if (reg->OpenKey("Software\\ArmFileAgent", false))
		 {
		   if (reg->ValueExists("ModuleName"))
			 result = reg->ReadString("ModuleName");

		   reg->CloseKey();
		 }
	 }
  __finally {delete reg;}

  return result;
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::UpdateAgent()
{
  try
	 {
	   Guard->Suspend();

	   try
		  {
			HWND handle = FindHandleByName(L"Файловий агент АРМ ВЗ");

			if (handle)
			  {
				Log->Add("Спроба завершити роботу Агента");

				PostMessage(handle, WM_QUIT, 0, 0);

				long passed = 0;

				while (FindHandleByName(L"Файловий агент АРМ ВЗ"))
				  {
					if (passed > 50000)
					  {
                        TerminateAgent();
						break;
					  }

					passed += 100;
					Sleep(100);
				  }

				CopyFile(String(DataPath + "\\" + AgentName + "_").c_str(),
						 String(DataPath + "\\" + AgentName).c_str(), 0);

				Log->Add("Агента оновлено");
			  }
		  }
	   __finally {Guard->Resume();}
	 }
  catch (Exception &e)
	 {
	   Log->Add("Оновлення Агента: "  + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TGuardian::TerminateAgent()
{
  int res;
  HANDLE proc;
  DWORD agent_pid;

  try
	 {
	   Log->Add("Знищення процесу Агента");

	   agent_pid = FindPIDByHandle(FindHandleByName(L"Файловий агент АРМ ВЗ"));
	   proc = OpenProcess(PROCESS_TERMINATE, 0, agent_pid);

	   try
		  {
			TerminateProcess(proc, 0);
		  }
	   __finally {if (proc) CloseHandle(proc);}
	 }
  catch (Exception &e)
	 {
	   Log->Add("Знищення процесу Агента: " + e.ToString());
	 }
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

