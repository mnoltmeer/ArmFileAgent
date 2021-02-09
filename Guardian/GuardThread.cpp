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

__fastcall TGuardThread::TGuardThread(bool CreateSuspended, const String agent_name)
	: TThread(CreateSuspended)
{
  FAgentName = agent_name;
}
//---------------------------------------------------------------------------

void __fastcall TGuardThread::CheckAgent()
{
  try
	 {

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
