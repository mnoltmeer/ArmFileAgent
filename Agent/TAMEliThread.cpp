/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "TAMEliThread.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(&UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall TAMEliThread::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall TAMEliThread::TAMEliThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------
void __fastcall TAMEliThread::Execute()
{
  passed = interval;

  while (!Terminated)
	{
	  if (passed >= interval)
		{
		  passed = 0;

		  if (FileExists(ScriptPath))
			{
			  iface->RunScriptFromFile(ScriptPath.c_str(), L"", logging);
              DeleteFile(ScriptPath);
			}
		}
	  else
		{
		  this->Sleep(100);
		  passed += 100;
		}
	}
}
//---------------------------------------------------------------------------


