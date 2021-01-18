/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "TAMThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TAMThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TAMThread::TAMThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall TAMThread::Execute()
{
  Synchronize(&ShowInfoStatus);

  try
	 {
	   Work();
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "ID: " + IntToStr(Connection->ServerID) + ", " +
								 "Thread: " + IntToStr(int(Connection->ServerThreadID)) + ", " +
								 "TAMThread::Execute(): " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TAMThread::Work()
{
  int ex_res;

  if (!Conn)
  	throw new Exception("Некоректний вказівник TExchangeConnect*");

  passed = Conn->ConnectionConfig->MonitoringInterval;
  interval = Conn->ConnectionConfig->MonitoringInterval;

  while (!Terminated)
	{
	  if (Conn->Working() &&
		  Conn->ConnectionConfig->StartAtTime &&
		  Conn->ConnectionConfig->TimeStart > Time())
		{
		  Conn->ServerStatus = "Очікування...";
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
			  case ES_ERROR_STOP: Terminate(); break;
			  case ES_NO_EXCHANGE: break;
			}

		  if (Conn->ConnectionConfig->RunOnce)
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
  InfoIcon->BalloonFlags = bfInfo;
  InfoIcon->BalloonHint = "[" + IntToStr(Conn->ServerID) + "] " +
						  Conn->ServerCaption + ": " +
						  Conn->ServerStatus;
  InfoIcon->ShowBalloonHint();
}
//---------------------------------------------------------------------------

void __fastcall TAMThread::ShowInfoNewDownload()
{
  InfoIcon->BalloonFlags = bfInfo;
  InfoIcon->BalloonHint = "[" + IntToStr(Conn->ServerID) + "] " +
						  Conn->ServerCaption + ": Завантажені нові файли";
  InfoIcon->ShowBalloonHint();
}
//---------------------------------------------------------------------------

void __fastcall TAMThread::ShowInfoNewUpload()
{
  InfoIcon->BalloonFlags = bfInfo;
  InfoIcon->BalloonHint = "[" + IntToStr(Conn->ServerID) + "] " +
					  	  Conn->ServerCaption + ": Файли вивантажені на сервер";
  InfoIcon->ShowBalloonHint();
}
//---------------------------------------------------------------------------

