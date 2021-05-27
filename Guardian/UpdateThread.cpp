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
	  Log->Add("����� ����� ��������� ������ �� ����� ���������");

	  TDateTime agent_date;
	  long agent_size;
	  int current_version[4] = {0,0,0,0}, candidate_version[4] = {0,0,0,0};

	  try {GetAppVersion(FUpdatePath.c_str(), candidate_version);}
	  catch (Exception &e){throw new Exception("�� ������� �������� ����� ����� ���������");}

	  try {GetAppVersion(FAgentPath.c_str(), current_version);}
	  catch (Exception &e){throw new Exception("�� ������� �������� ����� ����� ������");}

	  try {agent_date = GetFileDateTime(FAgentPath);}
	  catch (Exception &e){throw new Exception("�� ������� �������� ���� ��� ����� ������");}

	  try {agent_size = GetFileSize(FAgentPath);}
	  catch (Exception &e){throw new Exception("�� ������� �������� ����� ����� ������");}

	  if (CompareVersions(current_version, candidate_version) == 2)
		{
		  Log->Add("���� ��������� �� ����� �����");
		  res = true;
		}
	  else if (agent_date < GetFileDateTime(FUpdatePath))
		{
		  Log->Add("���� ��������� �� ����� ����");
		  res = true;
		}
	  else if (agent_size < GetFileSize(FUpdatePath))
		{
		  Log->Add("���� ��������� �� ������ �����");
		  res = true;
		}
	  else
		{
		  Log->Add("������� ����� ������ � ����������");
		  res = false;
		}
	}
  catch (Exception &e)
	{
	  res = false;
	  Log->Add("������� ����� �����: " + e.ToString());
	}

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TUpdateThread::UpdateAgent()
{
  try
	 {
	   HWND handle = FindHandleByName(L"�������� ����� ��� ��");

	   if (handle)
		 {
		   Log->Add("������ ��������� ������ ������");

		   PostMessage(handle, WM_QUIT, 0, 0);

		   if (!WaitForAppClose(L"�������� ����� ��� ��", 10000))
			 TerminateAgent();

		   String agent_name = GetFileNameFromFilePath(FAgentPath);

		   if (CopyFile(FUpdatePath.c_str(), FAgentPath.c_str(), 0))
		     Log->Add("������ ��������");
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("��������� ������: "  + e.ToString());
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
	   Log->Add("�������� ������� ������");

	   agent_pid = FindPIDByHandle(FindHandleByName(L"�������� ����� ��� ��"));
	   proc = OpenProcess(PROCESS_TERMINATE, 0, agent_pid);

       if (proc == INVALID_HANDLE_VALUE)
		 throw new Exception("�� ������� �������� ������ �� ������� ������");

	   try
		  {
			TerminateProcess(proc, 0);
		  }
	   __finally {CloseHandle(proc);}
	 }
  catch (Exception &e)
	 {
	   Log->Add("�������� ������� ������: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TUpdateThread::TryToUpdateAgent()
{
  try
	 {
	   if (NeedToUpdateAgent())
		 {
		   Log->Add("����� ��������� ������");

		   UpdateAgent();
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("��������� ������: "  + e.ToString());
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
	   Log->Add("������� ������ TUpdateThread, ID =" +
				IntToStr((int)this->ThreadID) + ": "  + e.ToString());
	 }
}
//---------------------------------------------------------------------------
