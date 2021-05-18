/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------
#include <System.hpp>
#pragma hdrstop

#include "RecpThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

__fastcall TRecpientCollectionThread::TRecpientCollectionThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
  FChanged = false;
  FInterval = 10000;
}
//---------------------------------------------------------------------------
void __fastcall TRecpientCollectionThread::Execute()
{
  int passed = CheckInterval;

  while (!Terminated)
	{
	  if ((passed >= CheckInterval) && (CollectionChanged))
		{
		  Collection->Save();
		  CollectionChanged = false;
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
