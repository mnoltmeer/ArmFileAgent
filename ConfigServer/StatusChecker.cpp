/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
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
	   std::vector<int> id_list;

	   id_list.clear();

	   Collection->SelectRecipients(&id_list);

	   std::unique_ptr<TStringStream> ms(new TStringStream("", TEncoding::UTF8, true));

	   for (int i = 0; i < id_list.size(); i++)
		  {
			RecipientItem *itm = Collection->FindItem(id_list[i]);

			if (itm)
			  {
				ms->Clear();
				ms->WriteString("#status");

				if (!AskFromHost(itm->Host.c_str(), itm->Port.ToInt(), ms.get()))
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
