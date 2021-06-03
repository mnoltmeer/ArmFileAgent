/*!
Copyright 2019-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "TAMThread.h"
#pragma package(smart_init)

extern String UsedAppLogDir; //вказуємо директорію для логування для функцій з MyFunc.h

__fastcall TAMThread::TAMThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall TAMThread::Execute()
{
  //Synchronize(&ShowInfoStatus);

  try
	 {
	   Work();
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("exceptions.log",
						   UsedAppLogDir,
						   "ID: " + IntToStr(Connection->ID) + ", " +
						   "Thread: " + IntToStr(int(Connection->ThreadID)) + ", " +
						   "TAMThread::Execute: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAMThread::Work()
{
  int ex_res;

  if (!Conn)
  	throw Exception("Некоректний вказівник TExchangeConnect*");

  passed = Conn->Config->MonitoringInterval;
  interval = Conn->Config->MonitoringInterval;

  while (!Terminated)
	{
	  if (Conn->Working() &&
		  (int)Conn->Config->StartAtTime > -1 &&
		  Conn->Config->StartAtTime > Time())
		{
		  Conn->Status = "Очікування...";
		  this->Sleep(300);
		}
	  else if (Conn->Working() && (passed >= interval))
		{
		  passed = 0;
		  ex_res = Conn->Exchange();

		  switch (ex_res)
			{
			  case ES_SUCC_DL: Synchronize(&ShowInfoNewDownload); break;
			  case ES_SUCC_UL: Synchronize(&ShowInfoNewUpload); break;
			  case ES_SUCC_DL_UL: Synchronize(&ShowInfoNewUpload);
								  Synchronize(&ShowInfoNewDownload);
								  break;
			  //case ES_ERROR_STOP: Terminate(); break;
			  case ES_NO_EXCHANGE: break;
			}

		  if (Conn->Config->RunOnce)
			Conn->Stop();
		}
	  else if (Conn->Working() && (passed < interval))
		{
		  this->Sleep(100);
		  passed += 100;
		}
	  else if (!Conn->Working())
        this->Sleep(300);
	}
}
//---------------------------------------------------------------------------

void __fastcall TAMThread::ShowInfoStatus()
{
  try
	 {
       InfoIcon->BalloonFlags = bfInfo;
	   InfoIcon->BalloonHint = "[" + IntToStr(Conn->ID) + "] " +
								Conn->Caption + ": " +
								Conn->Status;
	   InfoIcon->ShowBalloonHint();
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("exceptions.log",
						   UsedAppLogDir,
						   "ID: " + IntToStr(Connection->ID) + ", " +
						   "Thread: " + IntToStr(int(Connection->ThreadID)) + ", " +
						   "TAMThread::ShowInfoStatus: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAMThread::ShowInfoNewDownload()
{
  try
	 {
       InfoIcon->BalloonFlags = bfInfo;
	   InfoIcon->BalloonHint = "[" + IntToStr(Conn->ID) + "] " +
							   Conn->Caption + ": Завантажені нові файли";
	   InfoIcon->ShowBalloonHint();
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("exceptions.log",
						   UsedAppLogDir,
						   "ID: " + IntToStr(Connection->ID) + ", " +
						   "Thread: " + IntToStr(int(Connection->ThreadID)) + ", " +
						   "TAMThread::ShowInfoNewDownload: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAMThread::ShowInfoNewUpload()
{
  try
	 {
       InfoIcon->BalloonFlags = bfInfo;
	   InfoIcon->BalloonHint = "[" + IntToStr(Conn->ID) + "] " +
							   Conn->Caption + ": Файли вивантажені на сервер";
	   InfoIcon->ShowBalloonHint();
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("exceptions.log",
						   UsedAppLogDir,
						   "ID: " + IntToStr(Connection->ID) + ", " +
						   "Thread: " + IntToStr(int(Connection->ThreadID)) + ", " +
						   "TAMThread::ShowInfoNewUpload: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

