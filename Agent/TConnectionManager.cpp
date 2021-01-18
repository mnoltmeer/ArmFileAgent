//---------------------------------------------------------------------------

#pragma hdrstop

#include "TConnectionManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

int TConnectionManager::FDestroyConnections()
{
  try
	 {
	   for (int i = 0; i < Count; i++)
		  delete FConns[i];

	   FConns.clear();
	 }
  catch (Exception &e)
	 {
	   throw new Exception("TConnectionManager::FDestroyConnections: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

TExchangeConnect *TConnectionManager::ReadConns(int ind)
{
  if (ind < 0)
	throw new Exception("TConnectionManager::Links: Out of bounds!");
  else if (ind < FConns.size())
	return FConns[ind];
  else
	{
	  throw new Exception("TConnectionManager::Links: Out of bounds!");
    }
}
//---------------------------------------------------------------------------

void TConnectionManager::WriteConns(int ind, TExchangeConnect *conn)
{
  if (ind < 0)
	throw new Exception("TConnectionManager::Links: Out of bounds!");
  else if (ind < FConns.size())
	FConns[ind] = conn;
  else
	throw new Exception("TConnectionManager::Links: Out of bounds!");
}
//---------------------------------------------------------------------------

TExchangeConnect *TConnectionManager::ReadThreads(int ind)
{
  if (ind < 0)
	throw new Exception("TConnectionManager::Links: Out of bounds!");
  else if (ind < FConns.size())
	return FConns[ind];
  else
	{
	  throw new Exception("TConnectionManager::Links: Out of bounds!");
    }
}
//---------------------------------------------------------------------------

void TConnectionManager::WriteThreads(int ind, TExchangeConnect *conn)
{
  if (ind < 0)
	throw new Exception("TConnectionManager::Links: Out of bounds!");
  else if (ind < FConns.size())
	FConns[ind] = conn;
  else
	throw new Exception("TConnectionManager::Links: Out of bounds!");
}
//---------------------------------------------------------------------------

TExchangeConnect * TConnectionManager::Add(int ID, TTrayIcon *Icon, TThreadSafeLog *Log)
{
  TExchangeConnect *res;

  try
	 {
	   res = new TExchangeConnect(ID, Icon, Log);
	   FConns.push_back(res);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigLinks::Add: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

TExchangeConnect * TConnectionManager::Add(String cfg_file, int ID, TTrayIcon *Icon, TThreadSafeLog *Log)
{
  TExchangeConnect *res;

  try
	 {
	   res = new TExchangeConnect(cfg_file, ID, Icon, Log);
	   FConns.push_back(res);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("TConnectionManager::Add: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void TConnectionManager::Add(TExchangeConnect *conn)
{
  try
	 {
	   FConns.push_back(conn);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("TConnectionManager::Add: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void TConnectionManager::Remove(int ind)
{
  try
	 {
	   delete FCons[i];
	   FConns.erase(FConns.begin() + ind);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("TConnectionManager::Remove: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void TConnectionManager::Remove(const String &caption)
{
  try
	 {
	   int ind = IndexOf(caption);

	   delete FConns[ind];
	   FConns.erase(FConns.begin() + ind);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("TConnectionManager::Remove: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int TConnectionManager::IndexOf(const String &caption)
{
  int res;

  try
	 {
	   for (int i = 0; i < Count; i++)
		  {
			if (FConns[i]->ConnectionConfig->Caption == caption)
			  {
				res = i;
				break;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   throw new Exception("ClientConfigLinks::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int TConnectionManager::IndexOf(const wchar_t *cfg_file)
{
  int res;

  try
	 {
	   for (int i = 0; i < Count; i++)
		  {
			if (FConns[i]->ServerCfgPath == String(cfg_file))
			  {
				res = i;
				break;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   throw new Exception("TConnectionManager::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int TConnectionManager::IndexOf(TExchangeConnect *conn)
{
  int res;

  try
	 {
	   for (int i = 0; i < Count; i++)
		  {
			if (FConns[i] == conn)
			  {
				res = i;
				break;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   throw new Exception("TConnectionManager::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int TConnectionManager::GenConnectionID()
{
  int id = 0;

  for (int i = 0; i < Count; i++)
	 {
	   if (FConns[i]->ServerID > id)
		 id = srv->ServerID;
	 }

  return id + 1;
}
//---------------------------------------------------------------------------

TAMThread* __fastcall TConnectionManager::FindConnectionThread(unsigned int thread_id)
{
  for (int i = 0; i < FThreadList.size(); i++)
	 {
	   if (FThreadList[i]->ThreadID == thread_id)
		 return th;
	 }

  return NULL;
}
//---------------------------------------------------------------------------

void TConnectionManager::Run(TExchangeConnect *conn)
{
  if (conn)
	{
	  if (!conn->Working())
		{
		  if (conn->ServerThreadID > 0)
			Resume(conn);
		  else if (conn->Initialized())
			{
			  TAMThread *conn_thread = new TAMThread(true);

			  conn_thread->InfoIcon = TrayIcon1;
			  conn_thread->Connection = server;
			  conn->ServerThreadID = conn_thread->ThreadID;

			  FThreadList.push_back(conn_thread);

			  conn_thread->Resume();
			  conn->Start();
			}
		  else
			{
			  throw new Exception("З'єднання з ID " + IntToStr(conn->ServerID) + " не ініціалізоване!");
            }
		}
	}
  else
	{
	  throw new Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Resume(TExchangeConnect *conn)
{
  if (conn)
	{
	  if (!conn->Working())
		conn->Start();

	  if (!FindConnectionThread(server->ServerThreadID))
		{
		  throw new Exception("Зі з'єднанням " + IntToStr(conn->ServerID) + ":" +
							  conn->ServerCaption +
							  " не пов'язано жодного потоку!");
		}
	}
  else
	{
	  throw new Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Stop(TExchangeConnect *conn)
{
  if (conn)
	{
	  if (conn->Working())
		conn->Stop();

	  TAMThread *th = FindConnectionThread(conn->ServerThreadID);

	  if (th)
		{
		  th->Terminate();

          while (!th->Finished)
			Sleep(100);

		  th->Connection = NULL;
		  DeleteServerThread(th->ThreadID);
          server->ServerThreadID = 0;
		}
      else
		{
		  throw new Exception("Помилковий ID потоку " + conn->ServerCaption +
							  ", поток: " + IntToStr((int)conn->ServerThreadID));
		}
	}
  else
	{
	  throw new Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

TExchangeConnect* __fastcall TConnectionManager::Find(int id)
{
  for (int i = 0; i < Count; i++)
	 {
	   if (FConn[i]->ServerID == id)
		 return srv;
	 }

  return NULL;
}
//---------------------------------------------------------------------------

void __fastcall TConnectionManager::DeleteThread(unsigned int id)
{
  for (int i = 0; i < FThreadList.size(); i++)
	 {
	   if (FThreadList[ind]->ThreadID == id)
		 {
		   delete th;
		   FThreadList.erase(FThreadList.begin() + ind);

		   return;
		 }
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DeleteThreads()
{
  for (int i = 0; i < FThreadList.size(); i++)
	 delete FThreadList[i];

  FThreadList.clear();
}
//---------------------------------------------------------------------------