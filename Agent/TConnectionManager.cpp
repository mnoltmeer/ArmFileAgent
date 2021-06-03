//---------------------------------------------------------------------------

#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "TConnectionManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

void TConnectionManager::FDestroyConnections()
{
  try
	 {
	   for (int i = 0; i < ConnectionCount; i++)
		  {
			Stop(FConns[i]);
			delete FConns[i];
		  }

	   FConns.clear();
	 }
  catch (Exception &e)
	 {
	   throw Exception("TConnectionManager::FDestroyConnections: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

TExchangeConnect *TConnectionManager::ReadConns(int ind)
{
  if (ind < 0)
	throw Exception("TConnectionManager::Connections: Out of bounds!");
  else if (ind < FConns.size())
	return FConns[ind];
  else
	{
	  throw Exception("TConnectionManager::Connections: Out of bounds!");
    }
}
//---------------------------------------------------------------------------

void TConnectionManager::WriteConns(int ind, TExchangeConnect *conn)
{
  if (ind < 0)
	throw Exception("TConnectionManager::Connections: Out of bounds!");
  else if (ind < FConns.size())
	FConns[ind] = conn;
  else
	throw Exception("TConnectionManager::Connections: Out of bounds!");
}
//---------------------------------------------------------------------------

TAMThread *TConnectionManager::ReadThreads(int ind)
{
  if (ind < 0)
	throw Exception("TConnectionManager::Threads: Out of bounds!");
  else if (ind < FThreadList.size())
	return FThreadList[ind];
  else
	{
	  throw Exception("TConnectionManager::Threads: Out of bounds!");
    }
}
//---------------------------------------------------------------------------

void TConnectionManager::WriteThreads(int ind, TAMThread *th)
{
  if (ind < 0)
	throw Exception("TConnectionManager::Threads: Out of bounds!");
  else if (ind < FThreadList.size())
	FThreadList[ind] = th;
  else
	throw Exception("TConnectionManager::Threads: Out of bounds!");
}
//---------------------------------------------------------------------------

TExchangeConnect * TConnectionManager::Add(TThreadSafeLog *Log)
{
  TExchangeConnect *res;

  try
	 {
	   res = new TExchangeConnect(GenConnectionID(), Log);
	   FConns.push_back(res);
	 }
  catch (Exception &e)
	 {
	   throw Exception("TConnectionManager::Add: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

TExchangeConnect * TConnectionManager::Add(String cfg_file, TThreadSafeLog *Log)
{
  TExchangeConnect *res;

  try
	 {
	   if (!Find(cfg_file))
		 {
		   res = new TExchangeConnect(cfg_file, GenConnectionID(), Log);
		   FConns.push_back(res);
		 }
	 }
  catch (Exception &e)
	 {
	   res = NULL;
	   throw Exception("TConnectionManager::Add: " + e.ToString());
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
	   throw Exception("TConnectionManager::Add: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void TConnectionManager::Remove(int ind)
{
  try
	 {
	   delete FConns[ind];
	   FConns.erase(FConns.begin() + ind);
	 }
  catch (Exception &e)
	 {
	   throw Exception("TConnectionManager::Remove: " + e.ToString());
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
	   throw Exception("TConnectionManager::Remove: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void TConnectionManager::Remove(TExchangeConnect *conn)
{
  try
	 {
	   int ind = IndexOf(conn);

	   delete FConns[ind];
	   FConns.erase(FConns.begin() + ind);
	 }
  catch (Exception &e)
	 {
	   throw Exception("TConnectionManager::Remove: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int TConnectionManager::IndexOf(const String &caption)
{
  int res = -1;

  try
	 {
	   for (int i = 0; i < ConnectionCount; i++)
		  {
			if (FConns[i]->Config->Caption == caption)
			  {
				res = i;
				break;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   throw Exception("TConnectionManager::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int TConnectionManager::IndexOf(const wchar_t *cfg_file)
{
  int res = -1;

  try
	 {
	   for (int i = 0; i < ConnectionCount; i++)
		  {
			if (FConns[i]->ConfigPath == String(cfg_file))
			  {
				res = i;
				break;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   throw Exception("TConnectionManager::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int TConnectionManager::IndexOf(TExchangeConnect *conn)
{
  int res;

  try
	 {
	   for (int i = 0; i < ConnectionCount; i++)
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
	   throw Exception("TConnectionManager::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int TConnectionManager::GenConnectionID()
{
  int id = 0;

  for (int i = 0; i < ConnectionCount; i++)
	 {
	   if (FConns[i]->ID > id)
		 id = FConns[i]->ID;
	 }

  return id + 1;
}
//---------------------------------------------------------------------------

TAMThread* TConnectionManager::FindThread(unsigned int thread_id)
{
  for (int i = 0; i < FThreadList.size(); i++)
	 {
	   if (FThreadList[i]->ThreadID == thread_id)
		 return FThreadList[i];
	 }

  return NULL;
}
//---------------------------------------------------------------------------

TAMThread* TConnectionManager::AddThread(TExchangeConnect *conn)
{
  TAMThread *res;

  try
	 {
	   res = new TAMThread(true);
	   res->Connection = conn;

	   FThreadList.push_back(res);
	 }
  catch (Exception &e)
	 {
	   res = NULL;

	   throw Exception("TConnectionManager::AddThread: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void TConnectionManager::DeleteThread(unsigned int id)
{
  for (int i = 0; i < FThreadList.size(); i++)
	 {
	   if (FThreadList[i]->ThreadID == id)
		 {
		   delete FThreadList[i];
		   FThreadList.erase(FThreadList.begin() + i);

		   return;
		 }
	 }
}
//---------------------------------------------------------------------------

void TConnectionManager::DeleteThreads()
{
  for (int i = 0; i < FThreadList.size(); i++)
	 delete FThreadList[i];

  FThreadList.clear();
}
//---------------------------------------------------------------------------

void TConnectionManager::Run(TExchangeConnect *conn)
{
  if (conn)
	{
	  if (!conn->Working())
		{
		  if (conn->ThreadID > 0)
			Resume(conn);
		  else if (conn->Initialized())
			{
			  TAMThread *conn_thread = AddThread(conn);

			  conn_thread->InfoIcon = InfoIcon;
			  conn_thread->Connection = conn;
			  conn->ThreadID = conn_thread->ThreadID;
			  conn_thread->Resume();
			  conn->Start();
			}
		  else
			{
			  throw Exception("З'єднання з ID " + IntToStr(conn->ID) + " не ініціалізоване!");
            }
		}
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Resume(TExchangeConnect *conn)
{
  if (conn)
	{
	  if (!conn->Working())
		conn->Start();

	  if (!FindThread(conn->ThreadID))
		{
		  throw Exception("Зі з'єднанням " + IntToStr(conn->ID) + ":" +
							  conn->Caption +
							  " не пов'язано жодного потоку!");
		}
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Stop(TExchangeConnect *conn)
{
  if (conn)
	{
	  if (conn->Working())
		conn->Stop();

	  TAMThread *th = FindThread(conn->ThreadID);

	  if (th)
		{
		  th->Terminate();

          while (!th->Finished)
			Sleep(100);

		  th->Connection = NULL;
		  DeleteThread(th->ThreadID);
		  conn->ThreadID = 0;
		}
      else
		{
		  throw Exception("Помилковий ID потоку " + conn->Caption +
							  ", поток: " + IntToStr((int)conn->ThreadID));
		}
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Run(int id)
{
  TExchangeConnect *conn = Find(id);

  if (conn)
	{
	  Run(conn);
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Resume(int id)
{
  TExchangeConnect *conn = Find(id);

  if (conn)
	{
	  Resume(conn);
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Stop(int id)
{
  TExchangeConnect *conn = Find(id);

  if (conn)
	{
	  Stop(conn);
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Run(const String &caption)
{
  TExchangeConnect *conn = Find(caption);

  if (conn)
	{
	  Run(conn);
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Resume(const String &caption)
{
  TExchangeConnect *conn = Find(caption);

  if (conn)
	{
	  Resume(conn);
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void TConnectionManager::Stop(const String &caption)
{
  TExchangeConnect *conn = Find(caption);

  if (conn)
	{
	  Stop(conn);
	}
  else
	{
	  throw Exception("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

TExchangeConnect *TConnectionManager::Find(int id)
{
  for (int i = 0; i < ConnectionCount; i++)
	 {
	   if (FConns[i]->ID == id)
		 return FConns[i];
	 }

  return NULL;
}
//---------------------------------------------------------------------------

TExchangeConnect *TConnectionManager::Find(const String &cfg_file)
{
  for (int i = 0; i < ConnectionCount; i++)
	 {
	   if (FConns[i]->ConfigPath == cfg_file)
		 return FConns[i];
	 }

  return NULL;
}
//---------------------------------------------------------------------------

