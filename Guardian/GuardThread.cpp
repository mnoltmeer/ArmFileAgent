/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "..\..\work-functions\ThreadSafeLog.h"
#include "GuardThread.h"

#pragma package(smart_init)

extern TThreadSafeLog *Log;
//---------------------------------------------------------------------------

__fastcall TGuardThread::TGuardThread(bool CreateSuspended, const String agent_path)
	: TThread(CreateSuspended)
{
  FAgentPath = agent_path;
  FAgentName = GetFileNameFromFilePath(FAgentPath);
}
//---------------------------------------------------------------------------

void __fastcall TGuardThread::CheckAgent()
{
  try
	 {
	   DWORD agent_pid = GetProcessByExeName(FAgentName.c_str());

	   if (!agent_pid)
		 {
		   Log->Add("Запуск Агента");
		   StartProcessByExeName(FAgentPath);
         }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Контроль роботи Агента: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TGuardThread::Execute()
{
  FInterval = 5000;
  FPassed = FInterval;

  try
	 {
       while (!Terminated)
		 {
		   if (FPassed >= FInterval)
			 {
			   CheckAgent();
			   FPassed = 0;
			 }
		   else
			 {
			   this->Sleep(100);
			   FPassed += 100;
			 }
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка потоку TGuardThread, ID =" +
				IntToStr((int)this->ThreadID) + ": " + e.ToString());
	 }
}
//---------------------------------------------------------------------------
