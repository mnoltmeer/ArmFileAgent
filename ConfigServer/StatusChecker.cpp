/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "Main.h"
#include "StatusChecker.h"
#pragma package(smart_init)

extern TServerForm *ServerForm;
//---------------------------------------------------------------------------

__fastcall TStatusCheckThread::TStatusCheckThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
  FInterval = 60000;
}
//---------------------------------------------------------------------------

void __fastcall TStatusCheckThread::Execute()
{
  std::unique_ptr<TTCPRequester> tmp(new TTCPRequester());
  FSender = std::move(tmp);

  int passed = CheckInterval;

  if (FSender)
	{
	  while (!Terminated)
		{
		  if (passed >= CheckInterval)
			{
			  Check();
			  passed = 0;
			}
		  else
			{
			  this->Sleep(100);
		  	  passed += 100;
			}
		}
	}
  else
	{
	  ServerForm->WriteLog("StatusCheckThread: Sender not created. End thread");
	}
}
//---------------------------------------------------------------------------

void __fastcall TStatusCheckThread::Check()
{
  try
	 {
	   std::unique_ptr<TStringStream> ms(new TStringStream("", TEncoding::UTF8, true));
	   std::vector<int> id_list;

	   id_list.clear();

	   Collection->SelectRecipients(&id_list);

	   for (int i = 0; i < id_list.size(); i++)
		  {
			RecipientItem *itm = Collection->FindItem(id_list[i]);

			if (itm)
			  {
				ms->Clear();
				ms->WriteString("#status");

				try
				   {
					 FSender->Host = itm->Host;
					 FSender->Port = itm->Port.ToInt();

					 if (!FSender->Connect())
					   itm->Node->StateIndex = 4;
					 else if (!FSender->AskData(ms.get()))
					   itm->Node->StateIndex = 4;
					 else if (ms->ReadString(ms->Size) == "#ok")
					   itm->Node->StateIndex = 3;
				   }
				__finally{FSender->Disconnect();}
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   ServerForm->WriteLog("StatusCheckThread::Check: " + e.ToString());
	 }
  catch (std::exception &e)
	 {
	   ServerForm->WriteLog("StatusCheckThread::Check: " + String(e.what()));
	 }
  catch (...)
	 {
	   ServerForm->WriteLog("StatusCheckThread::Check: Unhandled exception");
	 }
}
//---------------------------------------------------------------------------
