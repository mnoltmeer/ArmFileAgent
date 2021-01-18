/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "..\..\work-functions\ThreadSafeLog.h"
#include "TExchangeConnect.h"
#include "TConnectionManager.h"
#include "FirstStart.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

String StationID, IndexVZ, MailFrom, MailTo, ConfigServerHost,
	   MailSubjectErr, MailSubjectOK, MailCodePage,
	   SmtpHost, LogName, DataPath, ConnPath, LogPath,
	   ModulesPath, AppVersion, ControlScript;

int SmtpPort, RemAdmPort, ConfigServerPort, ScriptInterval;
bool SendReportToMail, ScriptLog;

bool Initialised; //флаг активності інстансу, стає істиною після вдалого виконання InitInstance()

TThreadSafeLog *Log;

TConnectionManager *ConnManager;

HINSTANCE dllhandle;

GETELIINTERFACE GetELI;
FREEELIINTERFACE FreeELI;

ELI_INTERFACE *eIface;

TAMEliThread *EliThread;
//---------------------------------------------------------------------------

__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
  AppPath = Application->ExeName;
  int pos = AppPath.LastDelimiter("\\");
  AppPath.Delete(pos, AppPath.Length() - (pos - 1));

  DataPath = GetEnvironmentVariable("USERPROFILE") + "\\Documents\\AFAgent";
  ModulesPath = DataPath + "\\Modules";
  ConnPath = DataPath + "\\Connections";
  LogPath = DataPath + "\\Log";

  if (!DirectoryExists(DataPath))
	CreateDir(DataPath);

  if (!DirectoryExists(ModulesPath))
	CreateDir(ModulesPath);

  if (!DirectoryExists(ConnPath))
	CreateDir(ConnPath);

  if (!DirectoryExists(LogPath))
	CreateDir(LogPath);

  ThreadList = new TList();
  MenuItemList = new TList();
  Log = new TThreadSafeLog();
  ConnManager = new TConnectionManager;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FirstStartInitialisation(int type)
{
  try
	 {
	   if (type == INIT_DIALOG)
		 FirstStartForm->Show();
	   else if (type == INIT_MANUAL)
		 {
		   int pos;
		   StationID = GetPCName();

		   IndexVZ = ParamStr(2);
		   pos = IndexVZ.LastDelimiter("=");
		   IndexVZ.Delete(1, pos);

		   ConfigServerHost = ParamStr(3);
		   pos = ConfigServerHost.LastDelimiter("=");
		   ConfigServerHost.Delete(1, pos);

		   String port = ParamStr(4);
		   pos = port.LastDelimiter("=");
		   port.Delete(1, pos);
		   ConfigServerPort = port.ToInt();

		   port = ParamStr(5);
		   pos = port.LastDelimiter("=");
		   port.Delete(1, pos);
		   RemAdmPort = port.ToInt();

		   if (ParamStr(6) == "-auto")
			 {
			   if (ParamStr(7) == "-all")
				 AddAppAutoStart("ArmFileAgent", Application->ExeName, FOR_ALL_USERS);
			   else
				 AddAppAutoStart("ArmFileAgent", Application->ExeName, FOR_CURRENT_USER);
			 }
		   else if (ParamStr(6) == "-noauto")
			 {
			   if (ParamStr(7) == "-all")
				 RemoveAppAutoStart("ArmFileAgent", FOR_ALL_USERS);
			   else
				 RemoveAppAutoStart("ArmFileAgent", FOR_CURRENT_USER);
			 }
         }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка первинного запуску: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Migration()
{
  try
	 {
	   int pos;
	   StationID = GetPCName();
	   ConfigServerHost = ParamStr(4);
	   pos = ConfigServerHost.LastDelimiter("=");
	   ConfigServerHost.Delete(1, pos);

	   String port = ParamStr(5);
	   pos = port.LastDelimiter("=");
	   port.Delete(1, pos);
	   ConfigServerPort = port.ToInt();

	   String cfg = ParamStr(3);

	   Log->Add("Розпочато процес міграції з файлу " + cfg);

       int rp;

	   rp = ReadParameter(cfg, "IndexVZ", &IndexVZ, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка читання параметру IndexVZ: " + String(GetLastReadParamResult()));
		   return;
		 }

	   rp = ReadParameter(cfg, "RemAdmPort", &RemAdmPort, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка читання параметру RemAdmPort: " + String(GetLastReadParamResult()));
		   return;
		 }

	   bool autorun, autorun_all;

	   rp = ReadParameter(cfg, "EnableAutoStart", &autorun, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка читання параметру EnableAutoStart: " + String(GetLastReadParamResult()));
		   return;
		 }

	   rp = ReadParameter(cfg, "AutoStartForAllUsers", &autorun_all, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка читання параметру AutoStartForAllUsers: " + String(GetLastReadParamResult()));
		   return;
		 }

	   if (autorun)
		 AddAppAutoStart("ArmFileAgent", Application->ExeName, autorun_all);
	   else if (ParamStr(7) == "-noauto")
		 {
		   if (autorun_all)
			 RemoveAppAutoStart("ArmFileAgent", FOR_ALL_USERS);
		   else
		     RemoveAppAutoStart("ArmFileAgent", FOR_CURRENT_USER);
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка міграції: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckConfig(String cfg_file)
{
  try
	 {
	   for (int i = 0; i < MainPrmCnt; i++)
		  {
			if (GetConfigLine(cfg_file, MainParams[i]) == "^no_line")
			  {
				if (wcscmpi(MainParams[i], L"ScriptInterval") == 0)
				  AddConfigLine(cfg_file, MainParams[i], "10");
				else
				  AddConfigLine(cfg_file, MainParams[i], "0");
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   throw new Exception("Помилка перевірки конфігу з " + cfg_file +
			   			   " помилка: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ReadConfig()
{
  Log->Add("Завантаження конфігу з " + DataPath + "\\main.cfg");

  int result = 0;

//Основний конфіг
  try
	 {
	   CheckConfig(DataPath + "\\main.cfg");

	   int rp;

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailFrom", &MailFrom, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailFrom: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailTo", &MailTo, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailTo: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailCodePage", &MailCodePage, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailCodePage: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "SmtpHost", &SmtpHost, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SmtpHost: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "SmtpPort", &SmtpPort, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SmtpPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "RemAdmPort", &RemAdmPort, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру RemAdmPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "SendReportToMail", &SendReportToMail, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SendReportToMail: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   String str;

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailSubjectErr", &str, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailSubjectErr: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 MailSubjectErr = IndexVZ + " " + str;

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailSubjectOK", &str, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailSubjectOK: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 MailSubjectErr = IndexVZ + " " + str;

	   rp = ReadParameter(DataPath + "\\main.cfg", "ControlScriptName", &ControlScript, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ControlScriptName: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "ScriptLog", &ScriptLog, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ScriptLog: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "ScriptInterval", &ScriptInterval, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ScriptInterval: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

            srv->ReInitialize();
		  }
	 }
  catch(Exception &e)
	 {
	   Log->Add("Помилка читання конфігу: " + e.ToString());

	   result = -1;
	 }

  return result;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ReadSettings()
{
  int res = 1;

  try
	 {
       TRegistry *reg = new TRegistry();

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (reg->OpenKey("Software\\ArmFileAgent", false))
			  {
				if (reg->ValueExists("ConfigServerHost"))
				  ConfigServerHost = reg->ReadString("ConfigServerHost");
				else
				  res = 0;

				if (reg->ValueExists("ConfigServerPort"))
				  ConfigServerPort = reg->ReadInteger("ConfigServerPort");

				if (reg->ValueExists("StationID"))
				  StationID = reg->ReadString("ConfigServerHost");
                else
				  res = 0;

				if (reg->ValueExists("IndexVZ"))
				  IndexVZ = reg->ReadString("IndexVZ");
                else
				  res = 0;

				if (reg->ValueExists("RemAdmPort"))
				  RemAdmPort = reg->ReadInteger("RemAdmPort");
                else
				  res = 0;

				reg->CloseKey();
			  }
			else
			  res = -1;
		  }
	   __finally {delete reg;}
	 }
  catch (Exception &e)
	 {
	   res = 0;
	   Log->Add("Читання налаштувань з реєстру: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CreateServers()
{
  TStringList *servers = new TStringList();
  String str;

  try
	 {
	   GetFileList(servers, ConnPath, "*.cfg", false, true);

	   for (int i = 0; i < servers->Count; i++)
		  CreateConnection(servers->Strings[i]);
	 }
  __finally {delete servers;}
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::ConnectToSMTP()
{
  MailSender->Username = "noname@ukrposhta.com";
  MailSender->Password = "noname";
  MailSender->Host = SmtpHost;
  MailSender->Port = SmtpPort;

  try
	{
	  MailSender->Connect();
	}
  catch (Exception &e)
	{
	  Log->Add("SMTP помилка: " + e.ToString());

	  return false;
	}

  return MailSender->Connected();
}
//-------------------------------------------------------------------------

void __fastcall TMainForm::SendMsg(String mail_addr, String subject, String from, String log)
{
  if (MailSender->Connected())
	{
	  TIdMessage* msg = new TIdMessage(MainForm);

	  msg->CharSet = MailCodePage;
	  msg->Body->Text = log;
	  msg->From->Text = from;
	  msg->Recipients->EMailAddresses = mail_addr;
	  msg->Subject = subject;
	  msg->Priority = TIdMessagePriority(mpHighest);

	  MailSender->Send(msg);
	  MailSender->Disconnect();

	  delete msg;
	}
}
//-------------------------------------------------------------------------

void __fastcall TMainForm::ShowInfoMsg(String text)
{
  TrayIcon1->BalloonFlags = bfInfo;
  TrayIcon1->BalloonHint = text;
  TrayIcon1->ShowBalloonHint();
}
//-------------------------------------------------------------------------

int __fastcall TMainForm::SendLog(String mail_addr, String subject, String from, String log)
{
  int try_cnt = 0;

  while (!ConnectToSMTP())
	{
	  try_cnt++;

	  if (try_cnt >= 10)
		{
		  return 0;
		}

	  Sleep(30000);
	}

  SendMsg(mail_addr, subject, from, log);

  return 1;
}
//-------------------------------------------------------------------------

void __fastcall TMainForm::AURAServerExecute(TIdContext *AContext)
{
  String msg, cfg;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  TStringList *list = new TStringList();

  AContext->Connection->IOHandler->ReadStream(ms);

  try
	 {
	   try
		  {
			ms->Position = 0;
  			msg = ms->ReadString(ms->Size);

			StrToList(list, msg, "%");

			if (list->Strings[0] == "#send")
			  {
				if (list->Strings[1] == "status")
				  ASendStatus(AContext);
				else if (list->Strings[1] == "cfg")
				  ASendConfig(list, AContext);
				else if (list->Strings[1] == "log")
				  ASendLog(AContext);
				else if (list->Strings[1] == "srvlist")
				  ASendConnList(AContext);
				else if (list->Strings[1] == "thlist")
				  ASendThreadList(AContext);
				else if (list->Strings[1] == "file")
				  ASendFile(list, AContext);
				else if (list->Strings[1] == "version")
				  ASendVersion(AContext);
			  }
            else if (list->Strings[0] == "#get")
			  {
				TStringList *ls = new TStringList();

				try
				   {
					 int ind = list->Strings[2].ToInt();

					 StrToList(ls, list->Strings[3], "#");

					 if (ind == 0)
					   {
						 ls->SaveToFile(DataPath + "\\main.cfg", TEncoding::UTF8);
						 ReadConfig();
					   }
					 else
					   {
						 TExchangeConnect *srv = FindServer(ind);

						 if (srv)
						   {
							 ls->SaveToFile(srv->ServerCfgPath, TEncoding::UTF8);
							 srv->ReInitialize();
						   }
						 else
						   throw Exception("невідомий ID з'єднання: " + list->Strings[2]);
					   }

					 TrayIcon1->BalloonFlags = bfInfo;
					 TrayIcon1->BalloonHint = "Дані з конфігу оновлені";
					 TrayIcon1->ShowBalloonHint();
				   }
				__finally {delete ls;}
			  }
			else if (list->Strings[0] == "#get_new")
			  {
				//#get_new%<filename>%<cfg_data>
				TStringList *ls = new TStringList();

                try
				   {
					 StrToList(ls, list->Strings[2], "#");

					 ls->SaveToFile(ConnPath + "\\" + list->Strings[1], TEncoding::UTF8);

					 TrayIcon1->BalloonFlags = bfInfo;
					 TrayIcon1->BalloonHint = "Отримано новий конфіг";
					 TrayIcon1->ShowBalloonHint();
                     ReadConfig();
				   }
				__finally {delete ls;}
			  }
			else if (list->Strings[0] == "#delcfg_file")
			  {
				//#delcfg_file%<filename>
				Log->Add("FARA: отримано команду видалення конфігу, ім'я файлу: " + list->Strings[1]);
				DeleteFile(ConnPath + "\\" + list->Strings[1] + ".cfg");
			  }
			else if (list->Strings[0] == "#delcfg_name")
			  {
				//#delcfg_name%<caption>
				Log->Add("FARA: отримано команду видалення конфігу, ім'я з'єднання: " + list->Strings[1]);

				int ind = GetConnectionID(list->Strings[1]);

				TExchangeConnect *srv = FindServer(ind);

				if (srv)
				  {
					DeleteFile(srv->ServerCfgPath);
					DestroyConnection(srv->ServerID);
				  }
				else
				  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
			  }
			else if (list->Strings[0] == "#delcfg_id")
			  {
				//#delcfg_id%<id>
				Log->Add("FARA: отримано команду видалення конфігу, ID з'єднання: " + list->Strings[1]);

				int ind = list->Strings[1].ToInt();

				TExchangeConnect *srv = FindServer(ind);

				if (srv)
				  {
					DeleteFile(srv->ServerCfgPath);
                    DestroyConnection(srv->ServerID);
				  }
				else
				  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
              }
			else if (list->Strings[0] == "#run")
			  {
				int ind = list->Strings[1].ToInt();

				if (ind == 0)
				  StartWork();
				else
				  {
					TExchangeConnect *srv = FindServer(ind);

					if (srv)
					  {
						TAMThread *th = FindServerThread(srv->ServerThreadID);

						if (th)
						  th->PassedTime = srv->ConnectionConfig->MonitoringInterval;

						if (!srv->Working())
						  RunWork(srv);
					  }
					else
					  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
			else if (list->Strings[0] == "#stop")
			  {
				int ind = list->Strings[1].ToInt();

				if (ind == 0)
				  StopWork();
				else
				  {
					TExchangeConnect *srv = FindServer(ind);

					if (srv)
					  EndWork(srv);
					else
					  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
			else if (list->Strings[0] == "#restart")
			  {
				int ind = list->Strings[1].ToInt();

				if (ind == 0)
				  {
					StopWork();
					Sleep(1000);
					StartWork();
				  }
				else
				  {
					TExchangeConnect *srv = FindServer(ind);

					if (srv)
					  {
						EndWork(srv);
						Sleep(1000);
                        RunWork(srv);
					  }
					else
					  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
			else if (list->Strings[0] == "#shutdown")
			  {
				Log->Add("FARA: отримано команду shutdown");
				PostMessage(Application->Handle, WM_QUIT, 0, 0);
			  }
			else if (list->Strings[0] == "#restart_guard")
			  {
				Log->Add("FARA: отримано команду перезапуску Guardian");
				PostMessage(FindHandleByName(L"Guardian Менеджера обміну файлами АРМ ВЗ"), WM_QUIT, 0, 0);
				Sleep(3000);
				RunGuardian();
			  }
			else if (list->Strings[0] == "#reload_cfg")
			  {
				Log->Add("FARA: отримано команду перечитування конфігів");
				ReadConfig();
			  }
			else if (list->Strings[0] == "#exec_script")
			  {
				Log->Add("FARA: надійшов керуючий скрипт");

				try
				   {
					 eIface->RunScript(list->Strings[1].c_str(),
									   L"",
									   ScriptLog);

                     TStringList *lst = new TStringList();

					 try
						{
						  StrToList(lst, eIface->ShowInfoMessages(), "\r\n");

						  for (int i = 0; i < lst->Count; i++)
							Log->Add(lst->Strings[i]);
						}
					 __finally {delete lst;}
				   }
				catch (Exception &e)
				   {
					 Log->Add("ELI: помилка виконання скрипту " + e.ToString());
				   }
			  }
			else
			  throw Exception("Невідомі дані: " + list->Strings[0]);
		  }
	   catch (Exception &e)
		  {
			Log->Add("FARA: " + e.ToString());
		  }
	 }
  __finally {delete list; delete ms;}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopWork()
{
  try
	 {
	   SwitchOff->Visible = false;
	   SwitchOn->Visible = true;
	   LbStatus->Caption = "Зупинено";
	   LbStatus->Font->Color = clRed;
	   LbStatus->Tag = 0;

	   TExchangeConnect *srv;

	   for (int i = 0; i < SrvList->Count; i++)
		  EndWork(reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]));
	 }
  catch (Exception &e)
	 {
	   Log->Add("Зупинка роботи: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StartWork()
{
  try
	 {
	   SwitchOn->Visible = false;
	   SwitchOff->Visible = true;
	   LbStatus->Caption = "Робота";
	   LbStatus->Font->Color = clLime;
	   LbStatus->Tag = 1;

	   TExchangeConnect *srv;

	   for (int i = 0; i < SrvList->Count; i++)
		  RunWork(reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]));
	 }
  catch (Exception &e)
	 {
	   Log->Add("Початок роботи: " + e.ToString());

	   throw new Exception("Помилка запуску!");
	 }
}
//---------------------------------------------------------------------------

TExchangeConnect* __fastcall TMainForm::FindServer(int id)
{
  for (int i = 0; i < SrvList->Count; i++)
	 {
	   TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

	   if (srv->ServerID == id)
		 return srv;
	 }

  return NULL;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DeleteServerThread(unsigned int id)
{
  int ind = 0;

  while (ind < ThreadList->Count)
	 {
	   TAMThread *th = (TAMThread*)ThreadList->Items[ind];

	   if (th->ThreadID == id)
		 {
		   ThreadList->Delete(ind);
		   delete th;

		   return;
		 }
	   else
         ind++;
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DeleteServerThreads()
{
  int ind = 0;

  while (ind < ThreadList->Count)
	 {
	   TAMThread *th = (TAMThread*)ThreadList->Items[ind];

	   ThreadList->Delete(ind);
       delete th;
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RunWork(TExchangeConnect *server)
{
  if (server)
	{
	  if (!server->Working())
		{
		  if (server->ServerThreadID > 0)
			ResumeWork(server);
		  else if (server->Initialized())
			{
			  TAMThread *serv_thread = new TAMThread(true);
			  serv_thread->InfoIcon = TrayIcon1;
			  serv_thread->Connection = server;
			  server->ServerThreadID = serv_thread->ThreadID;
			  ThreadList->Add(serv_thread);
			  serv_thread->Resume();
			  server->Start();
			}
		  else
			{
			  Log->Add("З'єднання з ID " + IntToStr(server->ServerID) + " не ініціалізоване!");
			  Log->SaveToFile(LogPath + "\\" + LogName);
            }
		}
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ResumeWork(TExchangeConnect *server)
{
  if (server)
	{
	  if (!server->Working())
		server->Start();

	  if (!FindServerThread(server->ServerThreadID))
		{
		  Log->Add("Зі з'єднанням " + IntToStr(server->ServerID) + ":" +
				   server->ServerCaption +
				   " не пов'язано жодного потоку!");
		}
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::EndWork(TExchangeConnect *server)
{
  if (server)
	{
	  if (server->Working())
		server->Stop();

	  TAMThread *th = FindServerThread(server->ServerThreadID);

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
		  Log->Add("Помилковий ID потоку " + server->ServerCaption +
				   ", поток: " + IntToStr((int)server->ServerThreadID));
		}
	}
  else
	{
	  Log->Add("Помилковий вказівник TExchangeConnect*");
	}
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::GuardianRunning()
{
  DWORD MngrPID = GetProcessByExeName(L"AFAGuard.exe");

  if (MngrPID == 0)
	return false;
  else
   return true;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::RunGuardian()
{
  if (FileExists(DataPath + "\\ArmMngrGuard.exe"))
	{
	  Log->Add("Запуск Guardian");

	  ShellExecute(NULL,
				   L"open",
				   String(AppPath + "\\AFAGuard.exe").c_str(),
				   L"",
				   NULL,
				   SW_SHOWNORMAL);

	  if (GuardianRunning())
		return 0;
	  else
		return -1;
	}
  else
	{
	  Log->Add("Не вдалося запустити Guardian: відсутній файл");

      return -1;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP1Click(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwitchOnClick(TObject *Sender)
{
  StartWork();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SwitchOffClick(TObject *Sender)
{
  StopWork();
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SaveLogTimerTimer(TObject *Sender)
{
  TStringStream *ss = new TStringStream("", TEncoding::UTF8, true);

  if (!FileExists(LogPath + "\\" + LogName))
	SaveToFile(LogPath + "\\" + LogName, "");

  TFileStream *fs = new TFileStream(LogPath + "\\" + LogName, fmOpenReadWrite);

  try
	 {
	   Log->SaveToStream(ss);

	   if (ss->Size > fs->Size)
		 {
		   fs->Position = fs->Size;
		   ss->Position = fs->Size;
		   fs->Write(ss->Bytes, ss->Position, ss->Size - ss->Position);
		 }
	 }
  __finally {delete ss; delete fs;}

  if (Date().CurrentDate() > DateStart)
	{
	  Log->Clear();
	  LogName = DateToStr(Date()) + ".log";
	  DateStart = Date().CurrentDate();
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
  WindowState = wsMinimized;
  AppVersion = GetVersionInString(Application->ExeName.c_str());
  ModuleVersion->Caption = AppVersion;
  TrayIcon1->Hint = Caption;

  if (!DirectoryExists(LogPath))
	CreateDir(LogPath);

  if (!DirectoryExists(ConnPath))
	CreateDir(ConnPath);

  DateStart = Date().CurrentDate();
  LogName = DateToStr(Date()) + ".log";

  if (FileExists(LogPath + "\\" + LogName))
	Log->LoadFromFile(LogPath + "\\" + LogName);

  if (ParamStr(1) == "-init")
	{
	  if (ParamStr(2) == "-dialog")
		FirstStartInitialisation(INIT_DIALOG);
	  else if (ParamStr(2) == "-migrate")
		Migration();
	  else
		FirstStartInitialisation(INIT_MANUAL);
	}

  int settings = ReadSettings();

  if (settings < 0)
	FirstStartInitialisation(INIT_DIALOG);
  else if (settings == 0)
	{
	  Log->Add("Не всі параметри зчитано з реєстру. Проведіть процедуру первинної ініціалізації");
      Application->Terminate();
	}

  AURAServer->DefaultPort = RemAdmPort;
  AURAServer->Active = true;

  if (ReadConfig() == 0)
	{
	  SaveLogTimer->Enabled = true;
//підписуємось на отримання повідомлень WM_WTSSESSION_CHANGE
//щоб зупиняти Менеджер, коли користувач виходить з обліковки
	  if (!WTSRegisterSessionNotification(this->Handle, NOTIFY_FOR_THIS_SESSION))
		throw new Exception("WTSRegisterSessionNotification() fail: " + IntToStr((int)GetLastError()));

	  if (!GuardianRunning())
		{
		  if (RunGuardian() < 0)
			Log->Add("Помилка під час запуску Guardian");
		}

	  try
		 {
		   InitInstance();
		 }
	  catch (Exception &e)
		 {
		   Log->Add("Ініціалізація: " + e.ToString());
		 }
	}
  else
	{
	  Log->SaveToFile(LogPath + "\\" + LogName);
	  LbStatus->Caption = "Помилка!";
	  SwitchOn->Enabled = false;
	  SwitchOff->Enabled = false;

	  if (SendReportToMail)
		SendLog(MailTo, MailSubjectErr, MailFrom, Log->GetText());
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::InitInstance()
{
  try
	 {
	   if (ConnectELI() == 0)
		 {
		   ExecuteScript("admin.es");

		   EliThread = new TAMEliThread(true);

		   EliThread->ScriptPath = DataPath + "\\" + ControlScript;
		   EliThread->Logging = ScriptLog;
		   EliThread->SetRunInterval(ScriptInterval);
		   EliThread->ELIInterface = eIface;
		   EliThread->Resume();
		   		   
		   Log->Add("ELI: очікування на командний скрипт");
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ініціалізація та запуск: " + e.ToString());
	 }

  try
	 {
	   CreateServers();

	   Log->Add("Початок роботи");
	   Log->Add("Версія модулю: " + AppVersion);

	   StartWork();

	   TrayIcon1->BalloonFlags = bfInfo;
	   TrayIcon1->BalloonHint = "Файловий агент АРМ ВЗ запущено";
	   TrayIcon1->ShowBalloonHint();

	   Initialised = true;
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ініціалізація: " + e.ToString());
	   Initialised = false;

	   throw new Exception("Помилка ініціалізації!");
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopInstance()
{
  try
	 {
	   if (EliThread)
		 {
		   EliThread->Terminate();

		   while (!EliThread->Finished)
			 Sleep(100);

		   delete EliThread;
		 }

	   ReleaseELI();

	   if (LbStatus->Tag == 1)
		 StopWork();

	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

			if (srv->Working())
			  EndWork(srv);

			delete srv;
		  }

       delete ConnManager;
	   delete ThreadList;

	   for (int i = 0; i < MenuItemList->Count; i++)
		  {
			TMenuItem *m = reinterpret_cast<TMenuItem*>(MenuItemList->Items[i]);
			delete m;
		  }

	   delete MenuItemList;

	   TrayIcon1->Visible = false;
	   Log->Add("Кінець роботи");

	   if (SendReportToMail)
		 SendLog(MailTo, MailSubjectOK, MailFrom, Log->GetText());
	 }
  catch (Exception &e)
	 {
	   Log->Add("Завершення роботи: " + e.ToString());

	   throw new Exception("Помилка завершення роботи!");
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPPConnClick(TObject *Sender)
{
  TMenuItem *menu = reinterpret_cast<TMenuItem*>(Sender);
  TExchangeConnect *srv;
  TAMThread *th;

  try
	 {
	   srv = FindServer(menu->Hint.ToInt());

	   if (srv)
		 {
		   th = FindServerThread(srv->ServerThreadID);

		   if (th)
			 th->PassedTime = srv->ConnectionConfig->MonitoringInterval;

           if (!srv->Working())
		   	 RunWork(srv);
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ручний запуск обміну: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
  try
	 {
	   StopInstance();
	 }
  catch (Exception &e)
	 {
	   Log->Add(e.ToString());
	 }

//відписуємось від розсилки повідомлень WM_WTSSESSION_CHANGE
  WTSUnRegisterSessionNotification(this->Handle);

  AURAServer->Active = false;
  SaveLogTimer->Enabled = false;

  Log->SaveToFile(LogPath + "\\" + LogName);

  delete Log;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendStatus(TIdContext *AContext)
{
  Log->Add("FARA: запит статусу");

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);
			msg += IntToStr(srv->ServerID) + ": " + srv->ServerCaption + "=" + srv->ServerStatus + "#";
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendConfig(TStringList *list, TIdContext *AContext)
{
  Log->Add("FARA: запит конфігурації");

  int ind, res = -1;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  TStringList *ls = new TStringList();

  try
	 {
       String msg = "";

	   try
		  {
			ind = list->Strings[2].ToInt();

            if (ind == 0)
			  ls->LoadFromFile(DataPath + "\\main.cfg");
			else
			  {
				TExchangeConnect *srv = FindServer(ind);

				if (srv)
				  ls->LoadFromFile(srv->ServerCfgPath);
				else
				  throw Exception("Невідомий ID з'єднання: " + list->Strings[2]);
			  }

            for (int i = 0; i < ls->Count; i++)
			   {
				 msg += ls->Strings[i] + "#";
			   }

			if (msg == "")
			  msg = "error ";

			msg.Delete(msg.Length(), 1);
			ms->Clear();
			ms->WriteString(msg);
			ms->Position = 0;
            res = AAnswerToClient(AContext, ms);
		  }
	   catch (Exception &e)
		  {
			Log->Add("FARA: " + e.ToString());
		  }
     }
  __finally {delete ls; delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendLog(TIdContext *AContext)
{
  Log->Add("FARA: запит логу");

  String msg = "";
  int res = -1;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

  try
	 {
	   for (int i = 0; i < Log->Count; i++)
		  {
			msg += Log->Get(i) + "&";
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendConnList(TIdContext *AContext)
{
  Log->Add("FARA: запит списку з'єднань");

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   for (int i = 0; i < SrvList->Count; i++)
		  {
			TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);
			msg += IntToStr(srv->ServerID) + ": " + srv->ServerCaption + "#";
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendThreadList(TIdContext *AContext)
{
  Log->Add("FARA: запит списку потоків");

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   for (int i = 0; i < ThreadList->Count; i++)
		  {
			TAMThread *th = (TAMThread*)ThreadList->Items[i];
			msg += "Thread: " + IntToStr((int)th->ThreadID) + "=";

			if (th->Connection)
			  {
				msg += "Connection: id=" + IntToStr((int)th->Connection->ServerID) + ", " +
					   th->Connection->ServerCaption + ", ";

				if (th->Connection->Working())
				  msg += "Runnig#";
				else
				  msg += "Stoped#";
			  }
			else
			  {
				msg += "No connection#";
			  }
		  }

	   msg.Delete(msg.Length(), 1);

	   ms->Clear();
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendFile(TStringList *list, TIdContext *AContext)
{
  Log->Add("FARA: запит файлу " + list->Strings[2]);

  String msg = "";
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   if (FileExists(list->Strings[2]))
		 ms->LoadFromFile(list->Strings[2]);

	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ASendVersion(TIdContext *AContext)
{
  Log->Add("FARA: запит версії");

  String msg = GetVersionInString(Application->ExeName.c_str());
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
	   ms->WriteString(msg);
	   ms->Position = 0;
	   res = AAnswerToClient(AContext, ms);
	 }
  __finally {delete ms;}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::AAnswerToClient(TIdContext *AContext, TStringStream *ms)
{
  try
	 {
	   AContext->Connection->IOHandler->Write(ms, ms->Size, true);
	 }
  catch (Exception &e)
	 {
	   Log->Add("FARA: " + e.ToString());

	   return -1;
	 }

  return 0;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ConnectELI()
{
  int res;
  dllhandle = LoadLibrary(String(DataPath + "\\ELI.dll").c_str());

  if (!dllhandle)
	{
	  Log->Add("Помилка ініціалізації інтерфейсу ELI_INTERFACE");
	  res = -1;
	}
  else
	{
	  GetELI = (GETELIINTERFACE) GetProcAddress(dllhandle, "GetELIInterface");
	  FreeELI = (FREEELIINTERFACE) GetProcAddress(dllhandle, "FreeELIInterface");

	  if (!GetELI)
		{
		  Log->Add("Помилка ініціалізації GetELI");
		  res = -2;
		}
	  else if (!FreeELI)
		{
		  Log->Add("Помилка ініціалізації FreeELI");
		  res = -3;
		}
	  else if (!GetELI(&eIface))
		{
		  Log->Add("Помилка ініціалізації інтерфейсу ELI_INTERFACE");
		  res = -4;
		}
	  else
		{
		  Log->Add("Інтерфейс ELI_INTERFACE ініціалізований");
		  LoadFunctionsToELI();
		  res = 0;
		}
	}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ReleaseELI()
{
  int res;

  if (eIface)
	{
      if (!FreeELI(&eIface))
		{
		  Log->Add("Помилка звільнення інтерфейсу ELI_INTERFACE");
		  res = -1;
		}
	  else
		{
		  Log->Add("Інтерфейс ELI_INTERFACE вивільнений");
		  res = 0;
		}
	}

  FreeLibrary(dllhandle);

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ExecuteScript(String ctrl_script_name)
{
  if (FileExists(DataPath + "\\" + ctrl_script_name))
	{
	  try
		 {
		   Log->Add("ELI: запуск скрипту " + DataPath + "\\" + ctrl_script_name);

		   eIface->RunScriptFromFile(String(DataPath + "\\" + ctrl_script_name).c_str(),
									 L"",
									 ScriptLog);

		   DeleteFile(DataPath + "\\" + ctrl_script_name);

		   TStringList *lst = new TStringList();

		   try
			  {
				StrToList(lst, eIface->ShowInfoMessages(), "\r\n");

				for (int i = 0; i < lst->Count; i++)
				   Log->Add(lst->Strings[i]);
			  }
		   __finally {delete lst;}
		 }
	  catch (Exception &e)
		 {
		   Log->Add("ELI: помилка виконання скрипту " + e.ToString());
		 }
	}
  else
	{
	  Log->Add("ELI: відсутній файл керуючого скрипта " + DataPath + "\\" + ctrl_script_name);
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::LoadFunctionsToELI()
{
  eIface->AddFunction(L"_ConnCount", L"", &eConnectionsCount);
  eIface->AddFunction(L"_CreateConn", L"sym pFile,num pAddMenu", &eCreateConnection);
  eIface->AddFunction(L"_DestroyConn", L"num pID", &eDestroyConnection);
  eIface->AddFunction(L"_RemoveConn", L"num pID", &eRemoveConnection);
  eIface->AddFunction(L"_StartConn", L"num pID", &eStartConnection);
  eIface->AddFunction(L"_StopConn", L"num pID", &eStopConnection);
  eIface->AddFunction(L"_ConnID", L"sym pCap", &eConnectionID);
  eIface->AddFunction(L"_ConnIDInd", L"num pIndex", &eConnectionIDInd);
  eIface->AddFunction(L"_ConnStatus", L"num pID", &eConnectionStatus);
  eIface->AddFunction(L"_ConnCfgPath", L"num pID", &eConnectionCfgPath);
  eIface->AddFunction(L"_ReloadCfg", L"", &eReloadConfig);
  eIface->AddFunction(L"_ReadCfg", L"sym pFile,sym pPrm", &eReadFromCfg);
  eIface->AddFunction(L"_RemCfg", L"sym pFile,sym pPrm", &eRemoveFromCfg);
  eIface->AddFunction(L"_WriteCfg", L"sym pFile,sym pPrm,sym pVal", &eWriteToCfg);
  eIface->AddFunction(L"_WriteConnCfg", L"num pID,sym pPrm,sym pVal", &eWriteToCfgByID);
  eIface->AddFunction(L"_WriteToLog", L"sym pMsg", &eWriteMsgToLog);
  eIface->AddFunction(L"_GetAppPath", L"", &eGetAppPath);
  eIface->AddFunction(L"_GetDataPath", L"", &eGetDataPath);
  eIface->AddFunction(L"_ShutdownMngr", L"", &eShutdownManager);
  eIface->AddFunction(L"_ShutdownGuard", L"", &eShutdownGuardian);
  eIface->AddFunction(L"_StartGuard", L"", &eStartGuardian);
  eIface->AddFunction(L"_RestartGuard", L"", &eRestartGuardian);
}
//---------------------------------------------------------------------------

TExchangeConnect* __fastcall TMainForm::CreateConnection(String file)
{
  TExchangeConnect *res;

  try
	 {
	   res = ConnManager->Add(file, GenConnectionID(), TrayIcon1, Log);

	   TMenuItem *srv_menu = new TMenuItem(PopupMenu1);

	   srv_menu->Caption = "Запустити " + res->ConnectionConfig->Caption;
	   srv_menu->Hint = IntToStr(res->ServerID);
	   IconPP5->Add(srv_menu);
	   IconPP5->SubMenuImages = ImageList1;
	   srv_menu->ImageIndex = 4;
	   srv_menu->OnClick = IconPPConnClick;

	   MenuItemList->Add(srv_menu);
	   SrvList->Add(res);

	   RunWork(res);
	 }
  catch (Exception &e)
	 {
	   if (res) delete res;

	   res = NULL;
	   Log->Add("CreateConnection: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::CreateConnection(String file, bool create_menu)
{
  Log->Add("ELI: Створення з'єднання");

//використаний шлях типу ".\file" - використовується поточний каталог
  if (file[1] == '.')
	{
	  file.Delete(1, 1);
	  file = DataPath + file;
	}

  if (!FileExists(file))
    return -1;

  TExchangeConnect *srv = new TExchangeConnect(file, GenConnectionID(), TrayIcon1, Log);

  if (create_menu)
	{
      TMenuItem *srv_menu = new TMenuItem(PopupMenu1);

	  srv_menu->Caption = "Запустити " + srv->ConnectionConfig->Caption;
	  srv_menu->Hint = IntToStr(srv->ServerID);
	  IconPP5->Add(srv_menu);
	  IconPP5->SubMenuImages = ImageList1;
	  srv_menu->ImageIndex = 4;
	  srv_menu->OnClick = IconPPConnClick;

	  MenuItemList->Add(srv_menu);
    }

  SrvList->Add(srv);

  return srv->ServerID;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DestroyConnection(int id)
{
  Log->Add("ELI: Знищення з'єднання");

  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  EndWork(srv);
      SrvList->Delete(SrvList->IndexOf(srv));

	  delete srv;
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RemoveConnection(int id)
{
  Log->Add("ELI: Знищення з'єднання та видалення конфігу");

  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  EndWork(srv);
	  SrvList->Delete(SrvList->IndexOf(srv));
	  DeleteFile(srv->ServerCfgPath);

	  delete srv;
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StartConnection(int id)
{
  Log->Add("ELI: Запуск з'єднання");
  RunWork(FindServer(id));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopConnection(int id)
{
  Log->Add("ELI: Зупинка з'єднання");
  EndWork(FindServer(id));
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::GetConnectionID(String caption)
{
  int res = 0;

  for (int i = 0; i < SrvList->Count; i++)
	 {
	   TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[i]);

	   if (srv->ConnectionConfig->Caption == caption)
		 {
		   res = srv->ServerID;
           break;
		 }
	 }

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::GetConnectionID(int index)
{
  int res = 0;

  if ((index >= 0) && (index < SrvList->Count))
	{
	  TExchangeConnect *srv = reinterpret_cast<TExchangeConnect*>(SrvList->Items[index]);
	  res = srv->ServerID;
	}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ConnectionStatus(int id)
{
  int res;
  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  if (srv->Initialized() && srv->Working())
		res = 1;
	  else if (srv->Initialized() && !srv->Working())
		res = 0;
	  else if (!srv->Initialized())
        res = -1;
	}

  return res;
}
//---------------------------------------------------------------------------

String __fastcall TMainForm::ConnectionCfgPath(int id)
{
  String res = "";
  TExchangeConnect *srv = FindServer(id);

  if (srv)
	res = srv->ServerCfgPath;

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ReloadConfig()
{
  Log->Add("ELI: Команда оновлення конфігів");

  TrayIcon1->BalloonFlags = bfInfo;
  TrayIcon1->BalloonHint = "Дані з конфігу оновлені";
  TrayIcon1->ShowBalloonHint();
  ReadConfig();
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::RemoveFromCfg(String file, String param)
{
  Log->Add("ELI: Читання параметру " + param + " з файлу " + file);

//використаний шлях типу ".\file" - використовується поточний каталог
  if (file[1] == '.')
	{
	  file.Delete(1, 1);
	  file = DataPath + file;
	}

  if (!FileExists(file))
	return false;
  else
	return RemConfigLine(file, param);
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::WriteToCfg(String file, String param, String val)
{
  Log->Add("ELI: Запис параметру " + param + " у файл " + file);

  bool res;

//використаний шлях типу ".\file" - використовується поточний каталог
  if (file[1] == '.')
	{
	  file.Delete(1, 1);
	  file = DataPath + file;
	}

  if (!FileExists(file))
	res = false;
  else
	{
      if (GetConfigLineInd(file, param) > -1)
		res = SetConfigLine(file, param, val);
	  else
		res = AddConfigLine(file, param, val);
    }

  return res;
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::WriteToCfg(int id, String param, String val)
{
  Log->Add("ELI: Запис параметру " + param + " у конфіг підключення з ID = " + id);

  bool res;

  TExchangeConnect *srv = FindServer(id);

  if (srv)
	{
	  if (GetConfigLineInd(srv->ServerCfgPath, param) > -1)
		res = SetConfigLine(srv->ServerCfgPath, param, val);
	  else
		res = AddConfigLine(srv->ServerCfgPath, param, val);
	}

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteToMngrLog(String msg)
{
  Log->Add("ELI: " + msg);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShutdownManager()
{
  Log->Add("ELI: Отримано команду shutdown");
  PostMessage(Application->Handle, WM_QUIT, 0, 0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShutdownGuardian()
{
  Log->Add("ELI: Отримано команду зупинки Guardian");
  PostMessage(FindHandleByName(L"Guardian Файлового агенту АРМ ВЗ"), WM_QUIT, 0, 0);
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::StartGuardian()
{
  Log->Add("ELI: Отримано команду запуску Guardian");

  return RunGuardian();
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::RestartGuardian()
{
  Log->Add("ELI: Отримано команду перезапуску Guardian");

  PostMessage(FindHandleByName(L"Guardian Файлового агенту АРМ ВЗ"), WM_QUIT, 0, 0);

  Sleep(1000);

  return RunGuardian();
}
//---------------------------------------------------------------------------

void __stdcall eConnectionsCount(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[10];
  swprintf(res, L"%d", MainForm->Connections->Count);

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eCreateConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  bool menu = (bool)ep->GetParamToInt(L"pAddMenu");
  wchar_t res[3];

  try
	 {
	   int id = MainForm->CreateConnection(ep->GetParamToStr(L"pFile"), menu);

	   wcscpy(res, IntToStr(id).c_str());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eCreateConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eDestroyConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->DestroyConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eDestroyConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eRemoveConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->RemoveConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eRemoveConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eStartConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->StartConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eStartConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eStopConnection(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   MainForm->StopConnection(ep->GetParamToInt(L"pID"));
       wcscpy(res, L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eStopConnection(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionID(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToStr(L"pCap")));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionID(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionIDInd(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToInt(L"pIndex")));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionIDInd(): " + e.ToString());
	   wcscpy(res, L"0");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionStatus(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToInt(L"pID")));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionStatus(): " + e.ToString());
	   wcscpy(res, L"-2");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eConnectionCfgPath(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  String res;

  try
	 {
	   res = MainForm->ConnectionCfgPath(ep->GetParamToInt(L"pID"));
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eConnectionCfgPath(): " + e.ToString());
	   res = "";
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
}
//---------------------------------------------------------------------------

void __stdcall eReloadConfig(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  MainForm->ReloadConfig();

  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
}
//---------------------------------------------------------------------------

void __stdcall eReadFromCfg(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  String res;

  if (ReadParameter(ep->GetParamToStr(L"pFile"),
					ep->GetParamToStr(L"pPrm"),
					&res,
					TT_TO_STR) == RP_OK)
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
	}
  else
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
}
//---------------------------------------------------------------------------

void __stdcall eRemoveFromCfg(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  if (MainForm->RemoveFromCfg(ep->GetParamToStr(L"pFile"),
							  ep->GetParamToStr(L"pPrm")))
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	}
  else
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
}
//---------------------------------------------------------------------------

void __stdcall eWriteToCfg(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  if (!MainForm->WriteToCfg(ep->GetParamToStr(L"pFile"),
							ep->GetParamToStr(L"pPrm"),
							ep->GetParamToStr(L"pVal")))
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
  else
	ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
}
//---------------------------------------------------------------------------

void __stdcall eWriteToCfgByID(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  if (!MainForm->WriteToCfg(ep->GetParamToInt(L"pID"),
							ep->GetParamToStr(L"pPrm"),
							ep->GetParamToStr(L"pVal")))
	{
	  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	}
  else
	ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
}
//---------------------------------------------------------------------------

void __stdcall eWriteMsgToLog(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  try
	 {
	   MainForm->WriteToMngrLog(ep->GetParamToStr(L"pMsg"));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	 }
  catch (...)
	 {
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eGetAppPath(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  ep->SetFunctionResult(ep->GetCurrentFuncName(), AppPath.c_str());
}
//---------------------------------------------------------------------------

void __stdcall eGetDataPath(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  ep->SetFunctionResult(ep->GetCurrentFuncName(), DataPath.c_str());
}
//---------------------------------------------------------------------------

void __stdcall eShutdownManager(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  MainForm->ShutdownManager();

  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
}
//---------------------------------------------------------------------------

void __stdcall eShutdownGuardian(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;

  MainForm->ShutdownGuardian();

  ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
}
//---------------------------------------------------------------------------

void __stdcall eStartGuardian(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->StartGuardian());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eStartGuardian(): " + e.ToString());
	   wcscpy(res, L"-2");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __stdcall eRestartGuardian(void *p)
{
  ELI_INTERFACE *ep = (ELI_INTERFACE*)p;
  wchar_t res[3];

  try
	 {
	   swprintf(res, L"%d", MainForm->RestartGuardian());
	 }
  catch (Exception &e)
	 {
	   SaveLog("exceptions.log", "eRestartGuardian(): " + e.ToString());
	   wcscpy(res, L"-2");
	 }

  ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WndProc(Messages::TMessage& Msg)
{
  if (Msg.Msg == WM_WTSSESSION_CHANGE)
	{
	  if (Msg.WParam == WTS_CONSOLE_DISCONNECT )
		{
		  Log->Add("WTS_CONSOLE_DISCONNECT");

		  try
			 {
			   ShutdownGuardian();
			   StopWork();
			   AURAServer->Active = false;
			 }
		  catch (Exception &e)
			 {
			   Log->Add("WTS_CONSOLE_CONNECT: " + e.ToString());
			 }
		}
	  else if (Msg.WParam == WTS_CONSOLE_CONNECT)
		{
		  Log->Add("WTS_CONSOLE_CONNECT");

		  try
			 {
               StartGuardian();
			   StartWork();
			   AURAServer->Active = true;
			 }
		  catch (Exception &e)
			 {
			   Log->Add("WTS_CONSOLE_CONNECT: " + e.ToString());
			 }
		}
	}

  TForm::WndProc(Msg);
}
//---------------------------------------------------------------------------


