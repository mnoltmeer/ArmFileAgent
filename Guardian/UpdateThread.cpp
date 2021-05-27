/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "..\..\work-functions\ThreadSafeLog.h"
#include "GuardThread.h"
#include "UpdateThread.h"

#pragma package(smart_init)

extern TThreadSafeLog *Log;
extern TGuardThread *Guard;
//---------------------------------------------------------------------------

__fastcall TUpdateThread::TUpdateThread(const String &agent_path, const String &upd_path)
										: TThread(false)
{
  FAgentPath = agent_path;
  FUpdatePath = upd_path;
  FreeOnTerminate = true;
}
//---------------------------------------------------------------------------

bool __fastcall TUpdateThread::NeedToUpdateAgent()
{
  bool res;

  try
	{
	  Log->Add("Звірка версій поточного модуля та файлу оновлення");

	  TDateTime agent_date;
	  long agent_size;
	  int current_version[4] = {0,0,0,0}, candidate_version[4] = {0,0,0,0};

	  try {GetAppVersion(FUpdatePath.c_str(), candidate_version);}
	  catch (Exception &e){throw new Exception("Не вдалося отримати версію файлу оновлення");}

	  try {GetAppVersion(FAgentPath.c_str(), current_version);}
	  catch (Exception &e){throw new Exception("Не вдалося отримати версію файлу Агента");}

	  try {agent_date = GetFileDateTime(FAgentPath);}
	  catch (Exception &e){throw new Exception("Не вдалося отримати дату змін файлу Агента");}

	  try {agent_size = GetFileSize(FAgentPath);}
	  catch (Exception &e){throw new Exception("Не вдалося отримати розмір файлу Агента");}

	  if (CompareVersions(current_version, candidate_version) == 2)
		{
		  Log->Add("Файл оновлення має новішу версію");
		  res = true;
		}
	  else if (agent_date < GetFileDateTime(FUpdatePath))
		{
		  Log->Add("Файл оновлення має новішу дату");
		  res = true;
		}
	  else if (agent_size < GetFileSize(FUpdatePath))
		{
		  Log->Add("Файл оновлення має більший розмір");
		  res = true;
		}
	  else
		{
		  Log->Add("Поточна версія Агента є актуальною");
		  res = false;
		}
	}
  catch (Exception &e)
	{
	  res = false;
	  Log->Add("Помилка звірки версій: " + e.ToString());
	}

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TUpdateThread::UpdateAgent()
{
  try
	 {
	   HWND handle = FindHandleByName(L"Файловий агент АРМ ВЗ");

	   if (handle)
		 {
		   Log->Add("Спроба завершити роботу Агента");

		   PostMessage(handle, WM_QUIT, 0, 0);

		   if (!WaitForAppClose(L"Файловий агент АРМ ВЗ", 10000))
			 TerminateAgent();

		   String agent_name = GetFileNameFromFilePath(FAgentPath);

		   if (CopyFile(FUpdatePath.c_str(), FAgentPath.c_str(), 0))
		     Log->Add("Агента оновлено");
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Оновлення Агента: "  + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TUpdateThread::TerminateAgent()
{
  int res;
  HANDLE proc;
  DWORD agent_pid;

  try
	 {
	   Log->Add("Знищення процесу Агента");

	   agent_pid = FindPIDByHandle(FindHandleByName(L"Файловий агент АРМ ВЗ"));
	   proc = OpenProcess(PROCESS_TERMINATE, 0, agent_pid);

       if (proc == INVALID_HANDLE_VALUE)
		 throw new Exception("Не вдалось отримати доступ до процесу Агента");

	   try
		  {
			TerminateProcess(proc, 0);
		  }
	   __finally {CloseHandle(proc);}
	 }
  catch (Exception &e)
	 {
	   Log->Add("Знищення процесу Агента: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TUpdateThread::TryToUpdateAgent()
{
  try
	 {
	   if (NeedToUpdateAgent())
		 {
		   Log->Add("Старт оновлення Агента");

		   UpdateAgent();
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Оновлення Агента: "  + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TUpdateThread::Execute()
{
  try
	 {
	   try
		  {
			Guard->Suspend();

			Sleep(300);

			TryToUpdateAgent();
          }
	   __finally {Guard->Resume();}
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка потоку TUpdateThread, ID =" +
				IntToStr((int)this->ThreadID) + ": "  + e.ToString());
	 }
}
//---------------------------------------------------------------------------
