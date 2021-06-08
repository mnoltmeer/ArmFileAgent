/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <memory>
#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\TCPRequester.h"
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
  int passed = CheckInterval;

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
//---------------------------------------------------------------------------

void __fastcall TStatusCheckThread::Check()
{
  try
	 {
	   auto ms = std::make_unique<TStringStream>("", TEncoding::UTF8, true);
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

				auto sender = std::make_unique<TTCPRequester>(itm->Host, itm->Port.ToInt());

				if (!sender->AskData(ms.get()))
				  itm->Node->StateIndex = 4;
				else if (ms->ReadString(ms->Size) == "#ok")
				  itm->Node->StateIndex = 3;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   ServerForm->WriteLog("StatusCheckThread::Check: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------
