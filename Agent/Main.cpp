/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "..\..\work-functions\ThreadSafeLog.h"

#include "TConfigLoaderThread.h"
#include "TConnectionManager.h"
#include "FirstStart.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;

extern String UsedAppLogDir; //вказуємо директорію для логування для функцій з MyFunc.h

String StationID, IndexVZ, MailFrom, MailTo, ConfigServerHost,
	   MailSubjectErr, MailSubjectOK, MailCodePage,
	   SmtpHost, LogName, DataPath, LogPath, AppVersion, AppPath, AppName;

int SmtpPort, RemAdmPort, ConfigServerPort;
bool SendReportToMail, ScriptLog;

TThreadSafeLog *Log;

TConnectionManager *ConnManager;

TConfigLoaderThread *ConfigLoader;

HINSTANCE dllhandle;

GETELIINTERFACE GetELI;
FREEELIINTERFACE FreeELI;

ELI_INTERFACE *eIface;

TDate DateStart;

const int MainPrmCnt = 9;

const wchar_t *MainParams[MainPrmCnt] = {L"MailFrom",
										 L"MailTo",
										 L"MailCodePage",
										 L"SmtpHost",
										 L"SmtpPort",
										 L"SendReportToMail",
										 L"MailSubjectErr",
										 L"MailSubjectOK",
										 L"ScriptLog"};
//---------------------------------------------------------------------------

__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner)
{
  UsedAppLogDir = "AFAgent\\Log";

  AppPath = GetDirPathFromFilePath(Application->ExeName);
  AppName = GetFileNameFromFilePath(Application->ExeName);

  DataPath = GetEnvironmentVariable("USERPROFILE") + "\\Documents\\AFAgent";
  LogPath = DataPath + "\\Log";
  DataPath += "\\Data";

  if (!DirectoryExists(DataPath))
	ForceDirectories(DataPath);

  if (!DirectoryExists(LogPath))
	CreateDir(LogPath);

  AppVersion = GetVersionInString(Application->ExeName.c_str());
  ModuleVersion->Caption = AppVersion;
  TrayIcon->Hint = Caption;

  DateStart = Date().CurrentDate();
  LogName = DateToStr(Date()) + ".log";

  MenuItemList = new TList();
  Log = new TThreadSafeLog();
  ConnManager = new TConnectionManager();
  ConfigLoader = new TConfigLoaderThread(true);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormCreate(TObject *Sender)
{
  try
	 {
       WindowState = wsMinimized;
	   StartApplication();

//підписуємось на отримання повідомлень WM_WTSSESSION_CHANGE
//щоб зупиняти Менеджер, коли користувач виходить з обліковки
	   if (!WTSRegisterSessionNotification(this->Handle, NOTIFY_FOR_THIS_SESSION))
		 throw new Exception("WTSRegisterSessionNotification() fail: " + IntToStr((int)GetLastError()));
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка під час запуску");
       Log->Add("Ініціалізація: " + e.ToString());
	   Log->SaveToFile(LogPath + "\\" + LogName);

	   if (SendReportToMail)
	     MainForm->SendLog(MailTo, MailSubjectErr, MailFrom, Log->GetText());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FormDestroy(TObject *Sender)
{
  try
	 {
       try
		  {
			StopApplication();

            if (ConfigLoader && ConfigLoader->Started)
			  {
				ConfigLoader->Terminate();

				while (!ConfigLoader->Finished)
				  Sleep(100);

				delete ConfigLoader;
			  }

			ReleaseELI();

			delete ConnManager;

			for (int i = 0; i < MenuItemList->Count; i++)
			   {
				 TMenuItem *m = reinterpret_cast<TMenuItem*>(MenuItemList->Items[i]);
				 delete m;
			   }

	   		delete MenuItemList;
		  }
	   catch (Exception &e)
		  {
	   		Log->Add(e.ToString());
		  }
	 }
  __finally
	 {
//відписуємось від розсилки повідомлень WM_WTSSESSION_CHANGE
	   WTSUnRegisterSessionNotification(this->Handle);

	   if (SendReportToMail)
		 SendLog(MailTo, MailSubjectOK, MailFrom, Log->GetText());

	   Log->SaveToFile(LogPath + "\\" + LogName);

	   delete Log;
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::FirstStartInitialisation(int type)
{
  try
	 {
	   if (type == INIT_DIALOG)
		 {
		   Log->Add("Первинна ініціалізація: діалогове вікно");
		   Application->CreateForm(__classid(TFirstStartForm), &FirstStartForm);
		 }
	   else if (type == INIT_CMDLINE)
		 {
		   Log->Add("Первинна ініціалізація: командний рядок");

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

		   WriteSettings();

		   if (ParamStr(6) == "-firewall-add")
			 AddFirewallRule();
		   else if (ParamStr(6) == "-firewall-rem")
		 	 RemoveFirewallRule();
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
       String cfg = ParamStr(3) + "\\main.cfg";
	   Log->Add("Розпочато процес міграції з файлу " + cfg);

	   int pos;
	   StationID = GetPCName();
	   ConfigServerHost = ParamStr(4);
	   pos = ConfigServerHost.LastDelimiter("=");
	   ConfigServerHost.Delete(1, pos);

	   String port = ParamStr(5);
	   pos = port.LastDelimiter("=");
	   port.Delete(1, pos);
	   ConfigServerPort = port.ToInt();

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

	   WriteSettings();

	   if (ParamStr(6) == "-firewall-add")
		 AddFirewallRule();
	   else if (ParamStr(6) == "-firewall-rem")
		 RemoveFirewallRule();

	   Log->Add("Реєстраційні дані Агента додані у систему");

       if (CheckAppAutoStart("ArmFileManager", FOR_ALL_USERS))
		 RemoveAppAutoStart("ArmFileManager", FOR_ALL_USERS);
	   else if (CheckAppAutoStart("ArmFileManager", FOR_CURRENT_USER))
		 RemoveAppAutoStart("ArmFileManager", FOR_CURRENT_USER);

	   Log->Add("Реєстраційні дані Менеджеру файлів АРМ ВЗ видалені із системи");

       HWND handle = FindHandleByName(L"Менеджер обміну файлами АРМ ВЗ");

	   if (handle)
		 {
		   Log->Add("Спроба завершити роботу Guardian Менеджера");

		   PostMessage(handle, WM_QUIT, 0, 0);

		   if (!WaitForAppClose(L"Guardian Менеджера обміну файлами АРМ ВЗ", 5000))
			 ShutdownProcessByExeName("ArmMngrGuard.exe");
		 }

       handle = FindHandleByName(L"Менеджер обміну файлами АРМ ВЗ");

	   if (handle)
		 {
		   Log->Add("Спроба завершити роботу Менеджера");

		   PostMessage(handle, WM_QUIT, 0, 0);

		   if (!WaitForAppClose(L"Менеджер обміну файлами АРМ ВЗ", 5000))
			 ShutdownProcessByExeName("ArmMngr.exe");
		 }

       Sleep(2000);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка міграції: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::Unregistration()
{
  try
	 {
	   Caption = "ArmAgent -unreg";

	   TRegistry *reg = new TRegistry(KEY_WRITE);

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (reg->KeyExists("Software\\ArmFileAgent"))
			  reg->DeleteKey("Software\\ArmFileAgent");

            reg->RootKey = HKEY_LOCAL_MACHINE;

			if (reg->KeyExists("Software\\ArmFileAgent"))
			  reg->DeleteKey("Software\\ArmFileAgent");
		  }
	   __finally {delete reg;}

	   if (CheckAppAutoStart("ArmFileAgent", FOR_ALL_USERS))
		 RemoveAppAutoStart("ArmFileAgent", FOR_ALL_USERS);
	   else if (CheckAppAutoStart("ArmFileAgent", FOR_CURRENT_USER))
		 RemoveAppAutoStart("ArmFileAgent", FOR_CURRENT_USER);

	   Log->Add("Реєстраційні дані Агента видалені із системи");

	   ShutdownGuardian();

	   HWND hAgent;

	   while (hAgent = FindHandleByName(L"Файловий агент АРМ ВЗ"))
		 {
		   PostMessage(hAgent, WM_QUIT, 0, 0);
		   Sleep(100);
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка відміни реєстрації Агента у системі: " + e.ToString());
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

  int result = 1;

//Основний конфіг
  try
	 {
	   CheckConfig(DataPath + "\\main.cfg");

	   int rp;

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailFrom", &MailFrom, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailFrom: " + String(GetLastReadParamResult()));
		   result = 0;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailTo", &MailTo, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailTo: " + String(GetLastReadParamResult()));
		   result = 0;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailCodePage", &MailCodePage, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailCodePage: " + String(GetLastReadParamResult()));
		   result = 0;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "SmtpHost", &SmtpHost, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SmtpHost: " + String(GetLastReadParamResult()));
		   result = 0;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "SmtpPort", &SmtpPort, TT_TO_INT);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SmtpPort: " + String(GetLastReadParamResult()));
		   result = 0;
		 }

	   rp = ReadParameter(DataPath + "\\main.cfg", "SendReportToMail", &SendReportToMail, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру SendReportToMail: " + String(GetLastReadParamResult()));
		   result = 0;
		 }

	   String str;

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailSubjectErr", &str, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailSubjectErr: " + String(GetLastReadParamResult()));
		   result = 0;
		 }
	   else
		 MailSubjectErr = IndexVZ + " " + str;

	   rp = ReadParameter(DataPath + "\\main.cfg", "MailSubjectOK", &str, TT_TO_STR);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру MailSubjectOK: " + String(GetLastReadParamResult()));
		   result = 0;
		 }
	   else
		 MailSubjectErr = IndexVZ + " " + str;

	   rp = ReadParameter(DataPath + "\\main.cfg", "ScriptLog", &ScriptLog, TT_TO_BOOL);

	   if (rp != RP_OK)
		 {
		   Log->Add("Помилка створення параметру ScriptLog: " + String(GetLastReadParamResult()));
		   result = 0;
		 }

	   for (int i = 0; i < ConnManager->ConnectionCount; i++)
		  ConnManager->Connections[i]->ReInitialize();
	 }
  catch(Exception &e)
	 {
	   Log->Add("Помилка читання конфігу: " + e.ToString());

	   result = 0;
	 }

  return result;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::AddFirewallRule()
{
  try
	 {
	   Log->Add("Створення правил для файрволу");

	   AnsiString cmd;

	   if (system("netsh advfirewall firewall show rule name=\"ArmAgent\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"ArmAgent\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FARA\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FACS\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=in action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=in action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=in action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Створення правила для файрволу: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RemoveFirewallRule()
{
  try
	 {
	   Log->Add("Вилучення правил для файрволу");

	   system("netsh advfirewall firewall delete rule name=\"ArmAgent\"");
	   system("netsh advfirewall firewall delete rule name=\"FARA\"");
	   system("netsh advfirewall firewall delete rule name=\"FACS\"");
	 }
  catch (Exception &e)
	 {
	   Log->Add("Вилучення правила для файрволу: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteSettings()
{
  try
	 {
	   TRegistry *reg = new TRegistry(KEY_WRITE);

	   try
		  {
			reg->RootKey = HKEY_LOCAL_MACHINE;

			if (!reg->KeyExists("Software\\ArmFileAgent"))
			  reg->CreateKey("Software\\ArmFileAgent");

			if (reg->OpenKey("Software\\ArmFileAgent", false))
			  {
				if (ConfigServerPort)
				  reg->WriteInteger("ConfigServerPort", ConfigServerPort);

				if (RemAdmPort)
				  reg->WriteInteger("RemAdmPort", RemAdmPort);

				if (ConfigServerHost != "")
				  reg->WriteString("ConfigServerHost", ConfigServerHost);

				if (StationID != "")
				  reg->WriteString("StationID", StationID);

				if (IndexVZ != "")
				  reg->WriteString("IndexVZ", IndexVZ);

				reg->CloseKey();
			  }
		  }
	   __finally {delete reg;}
	 }
  catch (Exception &e)
	 {
	   Log->Add("Запис налаштувань до реєстру: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteModulePath()
{
  try
	 {
	   TRegistry *reg = new TRegistry(KEY_WRITE);

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (!reg->KeyExists("Software\\ArmFileAgent"))
			  reg->CreateKey("Software\\ArmFileAgent");

			if (reg->OpenKey("Software\\ArmFileAgent", false))
			  {
				reg->WriteString("ModulePath", Application->ExeName);

				reg->CloseKey();
			  }
		  }
	   __finally {delete reg;}
	 }
  catch (Exception &e)
	 {
	   Log->Add("Запис налаштувань до реєстру: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ReadSettings()
{
  int res = 1;

  try
	 {
	   TRegistry *reg = new TRegistry(KEY_READ);

	   try
		  {
			reg->RootKey = HKEY_LOCAL_MACHINE;

			if (reg->OpenKey("Software\\ArmFileAgent", false))
			  {
				if (reg->ValueExists("ConfigServerHost"))
				  ConfigServerHost = reg->ReadString("ConfigServerHost");
				else
				  res = 0;

				if (reg->ValueExists("ConfigServerPort"))
				  ConfigServerPort = reg->ReadInteger("ConfigServerPort");

				if (reg->ValueExists("StationID"))
				  StationID = reg->ReadString("StationID");
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
  TrayIcon->BalloonFlags = bfInfo;
  TrayIcon->BalloonHint = text;
  TrayIcon->ShowBalloonHint();
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
						 TExchangeConnect *srv = ConnManager->Find(ind);

						 if (srv)
						   {
							 ls->SaveToFile(srv->ConfigPath, TEncoding::UTF8);
							 srv->ReInitialize();
						   }
						 else
						   throw Exception("невідомий ID з'єднання: " + list->Strings[2]);
					   }

					 TrayIcon->BalloonFlags = bfInfo;
					 TrayIcon->BalloonHint = "Дані з конфігу оновлені";
					 TrayIcon->ShowBalloonHint();
				   }
				__finally {delete ls;}
			  }
			else if (list->Strings[0] == "#run")
			  {
				int ind = list->Strings[1].ToInt();

				if (ind == 0)
				  {
                    for (int i = 0; i < ConnManager->ConnectionCount; i++)
					   ConnManager->Run(ConnManager->Connections[i]);
                  }
				else
				  {
					TExchangeConnect *srv = ConnManager->Find(ind);

					if (srv)
					  {
						TAMThread *th = ConnManager->FindThread(srv->ThreadID);

						if (th)
						  th->PassedTime = srv->Config->MonitoringInterval;

						if (!srv->Working())
					  	  ConnManager->Run(srv);
					  }
					else
					  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
			else if (list->Strings[0] == "#stop")
			  {
				int ind = list->Strings[1].ToInt();

                if (ind == 0)
				  {
                    for (int i = 0; i < ConnManager->ConnectionCount; i++)
					   ConnManager->Stop(ConnManager->Connections[i]);
                  }
				else
				  {
					TExchangeConnect *srv = ConnManager->Find(ind);

					if (srv)
					  ConnManager->Stop(srv);
					else
				  	  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
				  }
			  }
			else if (list->Strings[0] == "#restart")
			  {
				int ind = list->Strings[1].ToInt();

				TExchangeConnect *srv = ConnManager->Find(ind);

				if (srv)
				  {
					ConnManager->Stop(srv);
					Sleep(1000);
					ConnManager->Run(srv);
				  }
				else
				  throw Exception("невідомий ID з'єднання: " + list->Strings[1]);
			  }
			else if (list->Strings[0] == "#shutdown")
			  {
				Log->Add("FARA: отримано команду shutdown");
				PostMessage(Application->Handle, WM_QUIT, 0, 0);
			  }
			else if (list->Strings[0] == "#restart_guard")
			  {
				Log->Add("FARA: отримано команду перезапуску Guardian");
				RestartGuardian();
			  }
			else if (msg.SubString(1, 6) == "#begin") //з цього слова починається будь-який скрипт
			  {
				Log->Add("FARA: надійшов керуючий скрипт");

				ExecuteScript(msg.c_str());
			  }
			else if (list->Strings[0] == "#status") //запит статусу від сервера конфігів
			  {
				ASendStatusAnswer(AContext);
			  }
			else if (list->Strings[0] == "#checkupdate") //команда примусового з'єднання з сервером конфігів
			  {
				Log->Add("FARA: надійшла команда перевірки оновлень");

				if (ConfigLoader)
				  {
					ConfigLoader->Terminate();
					HANDLE hLoader = reinterpret_cast<HANDLE>(ConfigLoader->Handle);

					DWORD wait = WaitForSingleObject(hLoader, 300);

					if (wait == WAIT_TIMEOUT)
					  TerminateThread(hLoader, 0);

					delete ConfigLoader;
				  }

				 ConfigLoader = new TConfigLoaderThread(true);

				if (!WaitForLoadFromServer(2000))
				  Log->Add("Немає відповіді від серверу конфігурацій");
				else
				  Log->Add("Встановлено зв'язок із сервером конфігурацій");
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
  if (FileExists(DataPath + "\\AFAGuard.exe"))
	{
	  Log->Add("Запуск Guardian");

	  StartProcessByExeName(DataPath + "\\AFAGuard.exe");

	  if (GuardianRunning())
		return 1;
	  else
		return 0;
	}
  else
	{
	  Log->Add("Не вдалося запустити Guardian: відсутній файл");

	  return 0;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::UpdateRequest()
{
  bool guard_running = GuardianRunning();

  if (guard_running)
	{
	  Log->Add("Запит на оновлення");
	  SendStartUpdateMessage();
	}
  else
	Log->Add("Запит на оновлення не відправлено: Guardian не запущено");
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::SendStartUpdateMessage()
{
  try
	{
	  HWND guard_handle = FindHandleByName(L"ArmAgent Guardian");

	  if (!guard_handle)
		throw new Exception("Не вдалося отримати хендл вікна Guardian");
	  else
		SendMessage(guard_handle, WM_KEYDOWN, VK_RETURN, NULL);
	}
  catch (Exception &e)
	{
	  Log->Add("Відправка Guardian повідомлення про початок оновлення: " + e.ToString());
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::IconPP1Click(TObject *Sender)
{
  Close();
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

void __fastcall TMainForm::AURAStartTimerTimer(TObject *Sender)
{
  try
	{
	  AURAServer->Active = true;

	  if (AURAServer->Active)
		{
		  Log->Add("Сервер службових команд запущено");
		  AURAStartTimer->Enabled = false;
		}
	}
  catch (Exception &e)
	{
	  Log->Add("Старт серверу службових команд: " + e.ToString());
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StartApplication()
{
  ConnManager->InfoIcon = TrayIcon;

  if (FileExists(LogPath + "\\" + LogName))
	Log->LoadFromFile(LogPath + "\\" + LogName);

  Log->Add("Агента запущено");

  if (ParamStr(1) == "-init")
	{
	  if (ParamStr(2) == "-dialog")
		FirstStartInitialisation(INIT_DIALOG);
	  else if (ParamStr(2) == "-migrate")
		Migration();
	  else
		FirstStartInitialisation(INIT_CMDLINE);
	}
  else if (ParamStr(1) == "-unreg")
	{
	  Unregistration();
	  Application->Terminate();

	  return;
	}
  else if (ParamStr(1) == "-firewall-add")
	{
	  AddFirewallRule();
	}
  else if (ParamStr(1) == "-firewall-rem")
	{
	  RemoveFirewallRule();
	}

  int settings = ReadSettings();

  if (settings < 0)
	FirstStartInitialisation(INIT_DIALOG);
  else if (settings == 0)
	Log->Add("Не всі параметри зчитано з реєстру. Проведіть процедуру первинної ініціалізації");
  else
	{
	  WriteModulePath();

	  AURAServer->DefaultPort = RemAdmPort;
	  SaveLogTimer->Enabled = true;
      AURAStartTimer->Enabled = true;

	  if (!WaitForLoadFromServer(2000))
		{
		  Log->Add("Немає відповіді від серверу конфігурацій");

		  Log->Add("Іініціалізація з'єднаннь з наявних файлів");
		  CreateExistConnections();
		}
	  else
		Log->Add("Встановлено зв'язок із сервером конфігурацій");

	   LbStatus->Caption = "Робота";
	   LbStatus->Font->Color = clLime;
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopApplication()
{
  try
	 {
	   LbStatus->Caption = "Зупинено";
	   LbStatus->Font->Color = clRed;

	   AURAServer->Active = false;

	   Log->Add("Кінець роботи");

	   AURAStartTimer->Enabled = false;
	   SaveLogTimer->Enabled = false;

       Log->SaveToFile(LogPath + "\\" + LogName);
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
	   srv = ConnManager->Find(menu->Hint.ToInt());

	   if (srv)
		 {
		   th = ConnManager->FindThread(srv->ThreadID);

		   if (th)
			 th->PassedTime = srv->Config->MonitoringInterval;

           if (!srv->Working())
			 ConnManager->Run(srv);
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Ручний запуск обміну: " + e.ToString());
	 }
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
	   for (int i = 0; i < ConnManager->ConnectionCount; i++)
		  {
			TExchangeConnect *srv = ConnManager->Connections[i];
			msg += IntToStr(srv->ID) + ": " + srv->Caption + "=" + srv->Status + "#";
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
				TExchangeConnect *srv = ConnManager->Find(ind);

				if (srv)
				  ls->LoadFromFile(srv->ConfigPath);
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
	   for (int i = 0; i < ConnManager->ConnectionCount; i++)
		  {
			TExchangeConnect *srv = ConnManager->Connections[i];
			msg += IntToStr(srv->ID) + ": " + srv->Caption + "#";
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
	   for (int i = 0; i < ConnManager->ThreadCount; i++)
		  {
			TAMThread *th = ConnManager->Threads[i];
			msg += "Thread: " + IntToStr((int)th->ThreadID) + "=";

			if (th->Connection)
			  {
				msg += "Connection: id=" + IntToStr((int)th->Connection->ID) + ", " +
					   th->Connection->Caption + ", ";

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

int __fastcall TMainForm::ASendStatusAnswer(TIdContext *AContext)
{
  TStringStream *ms = new TStringStream("#ok", TEncoding::UTF8, true);
  int res = -1;

  try
	 {
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
  if (ConnectELI() == 0)
	{
      if (FileExists(DataPath + "\\" + ctrl_script_name))
		{
		  try
			 {
			   Log->Add("ELI: запуск скрипту " + DataPath + "\\" + ctrl_script_name);

			   eIface->RunScriptFromFile(String(DataPath + "\\" + ctrl_script_name).c_str(),
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
		{
		  Log->Add("ELI: відсутній файл керуючого скрипта " + DataPath + "\\" + ctrl_script_name);
		}

	  ReleaseELI();
	}
  else
	{
	  Log->Add("ELI: помилка підключення бібліотеки");
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ExecuteScript(const wchar_t *ctrl_script_text)
{
  if (ConnectELI() == 0)
	{
	  try
		 {
		   Log->Add("ELI: запуск скрипту");

		   eIface->RunScript(ctrl_script_text, L"", ScriptLog);

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

	  ReleaseELI();
	}
  else
	{
	  Log->Add("ELI: помилка підключення бібліотеки");
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
  eIface->AddFunction(L"_ShutdownAgent", L"", &eShutdownAgent);
  eIface->AddFunction(L"_ShutdownGuard", L"", &eShutdownGuardian);
  eIface->AddFunction(L"_StartGuard", L"", &eStartGuardian);
  eIface->AddFunction(L"_RestartGuard", L"", &eRestartGuardian);
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::WaitForLoadFromServer(long millisec)
{
  bool res;
  long passed = 0;

  try
	 {
	   Log->Add("Очікування зв'язку з сервером конфігурацій");

	   if (ConfigLoader->Suspended)
		 ConfigLoader->Resume();
	   else if (ConfigLoader->Finished)
		 ConfigLoader->Start();

	   while (!ConfigLoader->Connected)
		 {
		   if (passed > millisec)
			 break;
		   else
			 {
			   Sleep(100);
			   passed += 100;
             }
		 }

       res = ConfigLoader->Connected;
	 }
  catch (Exception &e)
	 {
	   res = false;

	   Log->Add("Очікування зв'язку з сервером конфігурацій: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

TExchangeConnect* __fastcall TMainForm::CreateConnection(String file)
{
  TExchangeConnect *res;

  try
	 {
	   res = ConnManager->Add(file, Log);

	   if (!res)
		 throw new Exception("З'єднання з конфігу " + file + " вже існує");

	   if (!res->Initialized())
		 {
		   ConnManager->Remove(res);

		   return NULL;
         }

	   TMenuItem *srv_menu = new TMenuItem(PopupMenu);

	   srv_menu->Caption = "Запустити " + res->Config->Caption;
	   srv_menu->Hint = IntToStr(res->ID);
	   IconPP5->Add(srv_menu);
	   IconPP5->SubMenuImages = ImageList;
	   srv_menu->ImageIndex = 4;
	   srv_menu->OnClick = IconPPConnClick;

	   MenuItemList->Add(srv_menu);

	   ConnManager->Run(res);

	   Log->Add("Створене з'єднання \"" + res->Config->Caption + "\" з конфігу " + file);
	 }
  catch (Exception &e)
	 {
	   if (res) ConnManager->Remove(res);

	   res = NULL;
	   Log->Add("Створення з'єднання: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RebuildConnection(String file)
{
  try
	 {
	   Log->Add("Перестворення з'єднання");

	   TExchangeConnect *conn = ConnManager->Find(DataPath + "\\" + file);

	   if (conn)
		 DestroyConnection(conn->ID);

	   CreateConnection(DataPath + "\\" + file);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Перестворення з'єднання: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CheckAndStartConnection(String file)
{
  try
	 {
	   Log->Add("Перевірка та запуск з'єднання");

	   TExchangeConnect *conn = ConnManager->Find(DataPath + "\\" + file);

	   if (conn && (conn->Working()))
		 {
		   Log->Add("З'єднання " + conn->Caption + " вже запущене");
		   return;
		 }
	   else if (!conn)
		 CreateConnection(DataPath + "\\" + file);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Перевірка та запуск з'єднання :" + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::CreateExistConnections()
{
  TStringList *conns = new TStringList();
  String str;

  try
	 {
	   GetFileList(conns, DataPath, "*.cfg", WITHOUT_DIRS, WITHOUT_FULL_PATH);

	   for (int i = 0; i < conns->Count; i++)
		  {
			if (UpperCase(conns->Strings[i]) != "MAIN.CFG")
		      CreateConnection(DataPath + "\\" + conns->Strings[i]);
		  }
	 }
  __finally {delete conns;}
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::CreateConnection(String file, bool create_menu)
{
//використаний шлях типу ".\file" - використовується поточний каталог
  if (file[1] == '.')
	{
	  file.Delete(1, 1);
	  file = DataPath + file;
	}

  if (!FileExists(file))
    return -1;

  TExchangeConnect *srv = ConnManager->Add(file, Log);

  if (create_menu)
	{
	  TMenuItem *srv_menu = new TMenuItem(PopupMenu);

	  srv_menu->Caption = "Запустити " + srv->Config->Caption;
	  srv_menu->Hint = IntToStr(srv->ID);
	  IconPP5->Add(srv_menu);
	  IconPP5->SubMenuImages = ImageList;
	  srv_menu->ImageIndex = 4;
	  srv_menu->OnClick = IconPPConnClick;

	  MenuItemList->Add(srv_menu);
    }

  return srv->ID;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::DestroyConnection(int id)
{
  TExchangeConnect *srv = ConnManager->Find(id);

  if (srv)
	{
	  ConnManager->Stop(srv);

	  for (int i = 0; i < MenuItemList->Count; i++)
		  {
			TMenuItem *m = reinterpret_cast<TMenuItem*>(MenuItemList->Items[i]);

			if (m->Hint.ToInt() == srv->ID)
			  {
				delete m;
                MenuItemList->Delete(i);
                break;
			  }
		  }

      ConnManager->Remove(srv);
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::RemoveConnection(int id)
{
  TExchangeConnect *srv = ConnManager->Find(id);

  if (srv)
	{
	  DeleteFile(srv->ConfigPath);
      DestroyConnection(id);
    }
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StartConnection(int id)
{
  ConnManager->Run(ConnManager->Find(id));
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopConnection(int id)
{
  ConnManager->Stop(ConnManager->Find(id));
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::GetConnectionID(String caption)
{
  int res = 0;

  for (int i = 0; i < ConnManager->ConnectionCount; i++)
	 {
	   TExchangeConnect *srv = ConnManager->Connections[i];

	   if (srv->Config->Caption == caption)
		 {
		   res = srv->ID;
           break;
		 }
	 }

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::GetConnectionID(int index)
{
  int res = 0;

  if ((index >= 0) && (index < ConnManager->ConnectionCount))
	{
	  TExchangeConnect *srv = ConnManager->Connections[index];
	  res = srv->ID;
	}

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::ConnectionStatus(int id)
{
  int res;
  TExchangeConnect *srv = ConnManager->Find(id);

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
  TExchangeConnect *srv = ConnManager->Find(id);

  if (srv)
	res = srv->ConfigPath;

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ReloadConfig()
{
  //тут буде примусове з'єднання з сервером конфігів та зчитування отриманих конфігів
}
//---------------------------------------------------------------------------

bool __fastcall TMainForm::RemoveFromCfg(String file, String param)
{
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
  bool res;

  TExchangeConnect *srv = ConnManager->Find(id);

  if (srv)
	{
	  if (GetConfigLineInd(srv->ConfigPath, param) > -1)
		res = SetConfigLine(srv->ConfigPath, param, val);
	  else
		res = AddConfigLine(srv->ConfigPath, param, val);
	}

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::WriteToMngrLog(String msg)
{
  Log->Add("ELI: " + msg);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShutdownAgent()
{
  PostMessage(Application->Handle, WM_QUIT, 0, 0);
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::ShutdownGuardian()
{
  PostMessage(FindHandleByName(L"ArmAgent Guardian"), WM_CLOSE, 0, 0);
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::StartGuardian()
{
  return RunGuardian();
}
//---------------------------------------------------------------------------

int __fastcall TMainForm::RestartGuardian()
{
  PostMessage(FindHandleByName(L"ArmAgent Guardian"), WM_CLOSE, 0, 0);

  Sleep(1000);

  return RunGuardian();
}
//---------------------------------------------------------------------------

void __stdcall eConnectionsCount(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionsCount(): " + e.ToString());

	   return;
	 }

  try
	 {
       wchar_t res[10];
	   swprintf(res, L"%d", ConnManager->ConnectionCount);

	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionsCount(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"-1");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eCreateConnection(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eCreateConnection(): " + e.ToString());

	   return;
	 }

  try
	 {
       wchar_t res[3];
	   bool menu = (bool)ep->GetParamToInt(L"pAddMenu");
	   String file = ep->GetParamToStr(L"pFile");

	   Log->Add("ELI: Створення з'єднання з конфігу " + file);

	   int id = MainForm->CreateConnection(file, menu);

	   wcscpy(res, IntToStr(id).c_str());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eCreateConnection(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eDestroyConnection(void *p)
{
  Log->Add("ELI: Знищення з'єднання");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eDestroyConnection(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   MainForm->DestroyConnection(ep->GetParamToInt(L"pID"));
	   wcscpy(res, L"1");
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eDestroyConnection(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eRemoveConnection(void *p)
{
  Log->Add("ELI: Знищення з'єднання та видалення конфігу");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eRemoveConnection(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   MainForm->RemoveConnection(ep->GetParamToInt(L"pID"));
	   wcscpy(res, L"1");
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eRemoveConnection(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eStartConnection(void *p)
{
  Log->Add("ELI: Запуск з'єднання");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eStartConnection(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   MainForm->StartConnection(ep->GetParamToInt(L"pID"));
	   wcscpy(res, L"1");
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eStartConnection(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eStopConnection(void *p)
{
  Log->Add("ELI: Зупинка з'єднання");

 ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eStopConnection(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   MainForm->StopConnection(ep->GetParamToInt(L"pID"));
	   wcscpy(res, L"1");
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eStopConnection(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eConnectionID(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionID(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToStr(L"pCap")));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionID(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eConnectionIDInd(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionIDInd(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToInt(L"pIndex")));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionIDInd(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eConnectionStatus(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionStatus(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   swprintf(res, L"%d", MainForm->GetConnectionID(ep->GetParamToInt(L"pID")));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionStatus(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"-2");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eConnectionCfgPath(void *p)
{
 ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionCfgPath(): " + e.ToString());

	   return;
	 }

  try
	 {
	   String res = MainForm->ConnectionCfgPath(ep->GetParamToInt(L"pID"));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eConnectionCfgPath(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eReloadConfig(void *p)
{
  Log->Add("ELI: Команда оновлення конфігів");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eReloadConfig(): " + e.ToString());

	   return;
	 }

  try
	 {
	   MainForm->ReloadConfig();
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eReloadConfig(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eReadFromCfg(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eReadFromCfg(): " + e.ToString());

	   return;
	 }

  try
	 {
	   String res, param = ep->GetParamToStr(L"pPrm"), file = ep->GetParamToStr(L"pFile");

	   Log->Add("ELI: Читання параметру " + param + " з файлу " + file);

	   if (ReadParameter(file, param, &res, TT_TO_STR) == RP_OK)
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), res.c_str());
	   else
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eReadFromCfg(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eRemoveFromCfg(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eRemoveFromCfg(): " + e.ToString());

	   return;
	 }

  try
	 {
	   String res, param = ep->GetParamToStr(L"pPrm"), file = ep->GetParamToStr(L"pFile");

	   Log->Add("ELI: Видалення параметру " + param + " з файлу " + file);

	   if (MainForm->RemoveFromCfg(file, param))
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	   else
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eRemoveFromCfg(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eWriteToCfg(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eWriteToCfg(): " + e.ToString());

	   return;
	 }

  try
	 {
	   String res, param = ep->GetParamToStr(L"pPrm"), file = ep->GetParamToStr(L"pFile");

	   Log->Add("ELI: Запис параметру " + param + " у файл " + file);

	   if (MainForm->WriteToCfg(file, param, ep->GetParamToStr(L"pVal")))
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	   else
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eWriteToCfg(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eWriteToCfgByID(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eWriteToCfgByID(): " + e.ToString());

	   return;
	 }

  try
	 {
	   String param = ep->GetParamToStr(L"pPrm"), id = ep->GetParamToStr(L"pID");

	   Log->Add("ELI: Запис параметру " + param + " у конфіг підключення з ID = " + id);

	   if (MainForm->WriteToCfg(id.ToInt(), param, ep->GetParamToStr(L"pVal")))
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	   else
		 ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eWriteToCfgByID(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eWriteMsgToLog(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eWriteMsgToLog(): " + e.ToString());

	   return;
	 }

  try
	 {
	   MainForm->WriteToMngrLog(ep->GetParamToStr(L"pMsg"));
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eWriteMsgToLog(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eGetAppPath(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eGetAppPath(): " + e.ToString());

	   return;
	 }

  try
	 {
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), AppPath.c_str());
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eGetAppPath(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eGetDataPath(void *p)
{
  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eGetDataPath(): " + e.ToString());

	   return;
	 }

  try
	 {
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), DataPath.c_str());
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eGetDataPath(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eShutdownAgent(void *p)
{
  Log->Add("ELI: Отримано команду shutdown");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eShutdownAgent(): " + e.ToString());

	   return;
	 }

  try
	 {
	   MainForm->ShutdownAgent();
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eShutdownAgent(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eShutdownGuardian(void *p)
{
  Log->Add("ELI: Отримано команду зупинки Guardian");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eShutdownGuardian(): " + e.ToString());

	   return;
	 }

  try
	 {
	   MainForm->ShutdownGuardian();
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"1");
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eShutdownGuardian(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"0");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eStartGuardian(void *p)
{
  Log->Add("ELI: Отримано команду запуску Guardian");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eStartGuardian(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   swprintf(res, L"%d", MainForm->StartGuardian());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eStartGuardian(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"-2");
	 }
}
//---------------------------------------------------------------------------

void __stdcall eRestartGuardian(void *p)
{
  Log->Add("ELI: Отримано команду перезапуску Guardian");

  ELI_INTERFACE *ep;

  try
	 {
	   ep = static_cast<ELI_INTERFACE*>(p);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eRestartGuardian(): " + e.ToString());

	   return;
	 }

  try
	 {
	   wchar_t res[3];

	   swprintf(res, L"%d", MainForm->RestartGuardian());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), res);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("eli.log", UsedAppLogDir, "eRestartGuardian(): " + e.ToString());
	   ep->SetFunctionResult(ep->GetCurrentFuncName(), L"-2");
	 }
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
			   StopApplication();
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
			   StartApplication();
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


