/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "RecpOrganizer.h"
#include "RecpThread.h"
#include "ClientConfigLinks.h"
#include "AddRecord.h"
#include "AddGroup.h"
#include "Settings.h"
#include "Main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TServerForm *ServerForm;

extern String UsedAppLogDir; //вказуємо директорію для логування для функцій з MyFunc.h

TRecpientItemCollection *AddrBook;
TRecpientCollectionThread *AddrBookChecker;
ClientConfigManager *ConfigManager;
String LogFile, LogDir, DataDir;
int ListenPort, HideWnd, FullScreen, ActivityCheckInterval;
TDate DateStart;

extern TAddRecordForm *AddRecordForm;
extern TAddGroupForm *AddGroupForm;
//---------------------------------------------------------------------------

__fastcall TServerForm::TServerForm(TComponent* Owner)
	: TForm(Owner)
{
  try
	 {
	   UsedAppLogDir = "AFAConfigServer\\Log";
	   DataDir = GetEnvironmentVariable("USERPROFILE") + "\\Documents\\AFAConfigServer";
	   LogDir = DataDir + "\\Log";

	   if (!DirectoryExists(LogDir))
		 CreateDir(LogDir);

	   DateStart = Date().CurrentDate();

	   LogFile = DateToStr(DateStart) + ".log";

	   ReadSettings();
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::WriteLog(String record)
{
  SaveLog(LogDir + "\\" + LogFile, record);
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::FormCreate(TObject *Sender)
{
  if (FullScreen && !HideWnd)
	WindowState = wsMaximized;
  else if (HideWnd)
	WindowState = wsMinimized;

  SwServerOff->Show();
  LbVersion->Caption = "Версія: " + GetVersionInString(Application->ExeName.c_str());
  WriteLog("Початок роботи");

  try
	 {
	   if (!FileExists(DataDir + "\\hosts.grp"))
		 SaveToFile(DataDir + "\\hosts.grp", "");

	   AddrBook = new TRecpientItemCollection(DataDir + "\\hosts.grp");

	   if (!AddrBook->FindGroup("public"))
		 {
		   AddrBook->Add(0, AddrList->Items->Add(AddrList->Selected, "public"), "public");
           AddrBook->Save();
		 }

	   AddrBook->CreateSortedTree(AddrList);

	   AddrBookChecker = new TRecpientCollectionThread(true);
	   AddrBookChecker->Collection = AddrBook;
	   AddrBookChecker->CheckInterval = 1000;
	   AddrBookChecker->Resume();

	   if (FileExists(DataDir + "\\config.links"))
		 ConfigManager = new ClientConfigManager(DataDir + "\\config.links");
	   else
         ConfigManager = new ClientConfigManager();

	   StartServer();

       StatusChecker->Interval = ActivityCheckInterval * 60000;
       StatusChecker->Enabled = true;
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::FormClose(TObject *Sender, TCloseAction &Action)
{
  try
	 {
	   WriteSettings();

	   StatusChecker->Enabled = false;

	   StopServer();

	   AddrBookChecker->Terminate();

	   while (!AddrBookChecker->Finished)
		 Sleep(100);

	   delete AddrBookChecker;
	   delete AddrBook;
	   delete ConfigManager;

	   WriteLog("Кінець роботи");
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::AddrListClick(TObject *Sender)
{
  try
	 {
	   RecipientItem *itm = AddrBook->Find(AddrList->Selected);

	   if (itm && !itm->ParentNodeID) //обрано групу
		 {
		   ClientConfigLinks *links = ConfigManager->GetLinks(itm->Name, true);

		   try
			  {
				ConfigList->Clear();

				String file_sz, file_dt;
				unsigned int sz;

				for (int i = 0; i < links->Count(); i++)
				   {
                     sz = static_cast<unsigned int>(GetFileSize(links->Links[i]->FileName));

					 if (sz < 1024)
					   file_sz = String(sz) + " B";
					 else if ((sz >= 1024) && (sz < (10 * 1024 * 1024)))
					   file_sz = String(sz / 1024) + " KB";
					 else
					   file_sz = String(sz / (1024 * 1024)) + " MB";

					 file_dt = GetFormattedDate(GetFileDateTime(links->Links[i]->FileName), '.', ':', "dd.mm.yyy", "hh:nn:ss");

					 TListItem * li = ConfigList->Items->Add();

					 li->Caption = links->Links[i]->FileName;
					 li->SubItems->Add(file_sz);
					 li->SubItems->Add(file_dt);
				   }
			  }
		   __finally{delete links;}
		 }
	   else if (itm && itm->ParentNodeID) //обрано запис
		 {
		   RecipientItem *grp = AddrBook->FindGroup(itm->ParentNodeID);
		   ClientConfigLinks *links = ConfigManager->GetLinks(grp->Name, itm->Name);

		   try
			  {
				ConfigList->Clear();

				String file_sz, file_dt;
				unsigned int sz;

				for (int i = 0; i < links->Count(); i++)
				   {
					 sz = static_cast<unsigned int>(GetFileSize(links->Links[i]->FileName));

					 if (sz < 1024)
					   file_sz = String(sz) + " B";
					 else if ((sz >= 1024) && (sz < (10 * 1024 * 1024)))
					   file_sz = String(sz / 1024) + " KB";
					 else
					   file_sz = String(sz / (1024 * 1024)) + " MB";

					 file_dt = GetFormattedDate(GetFileDateTime(links->Links[i]->FileName), '.', ':', "dd.mm.yyy", "hh:nn:ss");

					 TListItem * li = ConfigList->Items->Add();

					 li->Caption = links->Links[i]->FileName;
					 li->SubItems->Add(file_sz);
					 li->SubItems->Add(file_dt);
				   }
			  }
		   __finally{delete links;}

		   ItemParams->Cells[1][1] = grp->Name;
		   ItemParams->Cells[1][2] = itm->Name;
		   ItemParams->Cells[1][3] = itm->Host;
		   ItemParams->Cells[1][4] = itm->Port;
		 }
	   else
		 throw new Exception("Невідомий ID");
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
     }
}
//---------------------------------------------------------------------------

int __fastcall TServerForm::AskToClient(const wchar_t *host, int port, TStringStream *rw_bufer)
{
  TIdTCPClient *sender;
  int res = 0;

  try
	 {
	   sender = CreateSender(host, port);

	   try
		  {
			sender->Connect();
			rw_bufer->Position = 0;
			sender->IOHandler->Write(rw_bufer, rw_bufer->Size, true);
		  }
	   catch (Exception &e)
		  {
			WriteLog(String(host) + ":" +
					 IntToStr(port) + " " +
					 "помилка відправки даних: " +
					 e.ToString());
			res = -1;
		  }

       try
		  {
			rw_bufer->Clear();
			sender->IOHandler->ReadStream(rw_bufer);
		  }
	   catch (Exception &e)
		  {
			WriteLog(String(host) + ":" +
					 IntToStr(port) + " " +
					 "помилка отримання даних: " +
					 e.ToString());
			res = -1;
		  }

	   rw_bufer->Position = 0;
	 }
  __finally
	 {
	   if (sender)
		 FreeSender(sender);
	 }

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TServerForm::SendToClient(const wchar_t *host, int port, TStringStream *rw_bufer)
{
  TIdTCPClient *sender;
  int res = 0;

  try
	 {
	   try
		  {
            sender = CreateSender(host, port);
			sender->Connect();
			sender->IOHandler->Write(rw_bufer, rw_bufer->Size, true);
		  }
	   catch (Exception &e)
		  {
			WriteLog(String(host) + ":" +
					 IntToStr(port) + " " +
					 "помилка відправки даних: " +
					 e.ToString());
			res = -1;
		  }

	   rw_bufer->Clear();
	 }
  __finally
	 {
	   if (sender)
		 FreeSender(sender);
	 }

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TServerForm::SendToClient(const wchar_t *host, int port, const String &data)
{
  TIdTCPClient *sender;
  int res = 0;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);

  try
	 {
	   try
		  {
			sender = CreateSender(host, port);
			sender->Connect();
            ms->Position = 0;
			sender->IOHandler->Write(ms, ms->Size, true);
		  }
	   catch (Exception &e)
		  {
			WriteLog(String(host) + ":" +
					 IntToStr(port) + " " +
					 "помилка відправки даних: " +
					 e.ToString());
			res = -1;
		  }
	 }
  __finally
	 {
	   delete ms;

	   if (sender)
		 FreeSender(sender);
	 }

  return res;
}
//---------------------------------------------------------------------------

TIdTCPClient* __fastcall TServerForm::CreateSender(const wchar_t *host, int port)
{
  TIdTCPClient *sender;

  try
	 {
	   sender = new TIdTCPClient(ServerForm);

	   sender->Host = host;
	   sender->Port = port;
	   sender->IPVersion = Id_IPv4;
	   sender->ConnectTimeout = 500;
	   sender->ReadTimeout = 5000;
	 }
  catch (Exception &e)
	 {
	   sender = NULL;
	   WriteLog("CreateSender: " + e.ToString());
	 }

  return sender;
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::FreeSender(TIdTCPClient *sender)
{
  try
	 {
	   if (sender)
		 {
		   if (sender->Connected())
			 {
			   sender->Disconnect();
			   sender->Socket->Close();
			 }

		   delete sender;
		 }
	 }
  catch (Exception &e)
	 {
	   sender = NULL;
	   WriteLog("FreeSender: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::AddToBookClick(TObject *Sender)
{
  try
	 {
	   AddRecordForm->Caption = "Створення запису";
	   AddRecordForm->Show();
	   AddRecordForm->Left = Left + ClientWidth / 2 - AddRecordForm->ClientWidth / 2;
	   AddRecordForm->Top = Top + ClientHeight / 2 - AddRecordForm->ClientHeight / 2;
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::AddGroupBookClick(TObject *Sender)
{
  try
	 {
	   AddGroupForm->Caption = "Створення групи";
	   AddGroupForm->Show();
	   AddGroupForm->Left = Left + ClientWidth / 2 - AddGroupForm->ClientWidth / 2;
	   AddGroupForm->Top = Top + ClientHeight / 2 - AddGroupForm->ClientHeight / 2;
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::DeleteFromBookClick(TObject *Sender)
{
  try
	 {
	   RecipientItem *itm = AddrBook->Find(AddrList->Selected);

	   if (itm && !itm->ParentNodeID)
		 {
		   if (MessageBox(Application->Handle,
						  L"Дійсно видалити групу записів?",
						  L"Підтвердьте дію",
						  MB_YESNO|MB_ICONWARNING) == mrYes)
			 {
			   RecipientItem *grp = AddrBook->FindGroup(AddrList->Selected);
			   ConfigManager->RemoveLinks(grp->Name);
			   ConfigManager->SaveToFile(DataDir + "\\config.links");
			   AddrBook->DeleteRecipientsInGroup(grp->ID);
			   AddrBook->Remove(AddrList->Selected);
			   AddrBook->CreateSortedTree(AddrList);
			   AddrBookChecker->CollectionChanged = true;
			 }
		 }
	   else if (MessageBox(Application->Handle, L"Видалити запис?", L"Підтвердьте дію", MB_YESNO|MB_ICONWARNING) == mrYes)
		 {
		   int grp_id = itm->ParentNodeID;

		   RecipientItem *grp = AddrBook->FindGroup(grp_id);
		   ConfigManager->RemoveLinks(grp->Name, itm->Name);
		   ConfigManager->SaveToFile(DataDir + "\\config.links");
		   AddrBook->Remove(itm->ID);
           AddrBook->CreateSortedTree(AddrList);
		   AddrBookChecker->CollectionChanged = true;

		   grp->Node->Expand(true);
		 }
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::EditBookClick(TObject *Sender)
{
  RecipientItem *itm = AddrBook->Find(AddrList->Selected);

  if (itm && !itm->ParentNodeID)
	{
	  AddGroupForm->Name->Text = itm->Name;
	  AddGroupForm->Caption = "Редагування групи";
	  AddGroupForm->Show();
      AddGroupForm->Left = Left + ClientWidth / 2 - AddGroupForm->ClientWidth / 2;
	  AddGroupForm->Top = Top + ClientHeight / 2 - AddGroupForm->ClientHeight / 2;
	}
  else if (itm && itm->ParentNodeID)
	{
      AddRecordForm->Caption = "Редагування запису";
	  AddRecordForm->Station->Text = itm->Name;
	  AddRecordForm->Host->Text = itm->Host;
	  AddRecordForm->Port->Text = itm->Port;

	  AddRecordForm->Show();
      AddRecordForm->Left = Left + ClientWidth / 2 - AddRecordForm->ClientWidth / 2;
	  AddRecordForm->Top = Top + ClientHeight / 2 - AddRecordForm->ClientHeight / 2;
	}
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ImportInAddrBookClick(TObject *Sender)
{
  String old_mask = OpenCfgDialog->Filter;
  OpenCfgDialog->FileName = "";
  OpenCfgDialog->Filter = "адресна книга + файли|*.afl|адресні книги (нові)|*.grp|адресні книги (старі)|*.book";

  if (OpenCfgDialog->Execute())
	{
	  try
		 {
		   String file_ext = OpenCfgDialog->FileName;
		   int pos = file_ext.LastDelimiter("\\");
		   file_ext.Delete(1, pos);
		   pos = file_ext.LastDelimiter(".");

           if (!pos)
			 {
			   if (OpenCfgDialog->FilterIndex == 3)
				 file_ext = "book";
			   else if (OpenCfgDialog->FilterIndex == 2)
				 file_ext = "grp";
			   else if (OpenCfgDialog->FilterIndex == 1)
				 file_ext = "afl";

			   OpenCfgDialog->FileName = OpenCfgDialog->FileName + "." + file_ext;
			 }
		   else
			 file_ext.Delete(1, pos);

		   if (file_ext == "book")
			 {
			   RecipientItem *grp = AddrBook->FindGroup("Несортоване");

			   if (!grp)
				 {
				   AddrBook->Add(0, NULL, "Несортоване"); //додаємо групу у нову книгу
				   grp = AddrBook->FindGroup("Несортоване");
				 }

			   TStringList *old_book = new TStringList();
			   TStringList *lst = new TStringList();

			   old_book->LoadFromFile(OpenCfgDialog->FileName, TEncoding::UTF8);

			   try
				  {
					for (int i = 0; i < old_book->Count; i++)
					   {
						 lst->Clear();
						 StrToList(lst, old_book->Strings[i], ";");
                         grp = AddrBook->FindGroup("Несортоване");
						 AddrBook->Add(grp->ID,
									   grp->Node,
									   lst->Strings[0],
									   lst->Strings[1],
									   lst->Strings[2]);

						 WriteLog("Імпортовано запис: " + lst->Strings[0]);
					   }

                    WriteLog("Імпорт книги завершено");
				  }
			   __finally {delete lst; delete old_book;}

			   AddrBook->Save();

			   AddrBook->CreateSortedTree(AddrList);
			   grp = AddrBook->FindGroup("Несортоване");
			   grp->Node->Expand(true);
			 }
		   else if (file_ext == "grp")
			 {
			   TRecpientItemCollection *ImportBook = new TRecpientItemCollection(OpenCfgDialog->FileName);

			   try
				  {
					AddrBook->ImportData(ImportBook);
					AddrBook->CreateSortedTree(AddrList);
					AddrBook->Save();
					WriteLog("Імпорт книги завершено");
				  }
			   __finally{delete ImportBook;}
			 }
           else if (file_ext == "afl")
			 ImportAddrAndLinks(OpenCfgDialog->FileName);
		 }
	  catch (Exception &e)
		 {
		   WriteLog("Імпорт книги: " + e.ToString());
		 }
	}

  OpenCfgDialog->Filter = old_mask;
  OpenCfgDialog->FileName = "";
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ExportFromAddrBookClick(TObject *Sender)
{
  String old_mask = SaveCfgDialog->Filter;
  SaveCfgDialog->FileName = "";
  SaveCfgDialog->Filter = "адресна книга + файли|*.afl|адресні книги (нові)|*.grp|адресні книги (старі)|*.book";

  if (SaveCfgDialog->Execute())
	{
	  try
		 {
		   String file_ext = SaveCfgDialog->FileName;
		   int pos = file_ext.LastDelimiter("\\");
		   file_ext.Delete(1, pos);
		   pos = file_ext.LastDelimiter(".");

		   if (!pos)
			 {
			   if (SaveCfgDialog->FilterIndex == 3)
				 file_ext = "book";
			   else if (SaveCfgDialog->FilterIndex == 2)
				 file_ext = "grp";
			   else if (SaveCfgDialog->FilterIndex == 1)
				 file_ext = "afl";

			   SaveCfgDialog->FileName = SaveCfgDialog->FileName + "." + file_ext;
			 }
		   else
			 file_ext.Delete(1, pos);

		   if (file_ext == "book")
			 {
			   DeleteFile(SaveCfgDialog->FileName);

			   for (int i = 0; i < AddrBook->Count; i++)
				  {
					if (AddrBook->Items[i]->ParentNodeID > 0)
					  {
						AddToFile(SaveCfgDialog->FileName,
								  AddrBook->Items[i]->Name + ";" +
								  AddrBook->Items[i]->Host + ";" +
							  	  AddrBook->Items[i]->Port + "\r\n");
					  }
				  }
			 }
		   else if (file_ext == "grp")
			 AddrBook->SaveToFile(SaveCfgDialog->FileName);
		   else if (file_ext == "afl")
			 ExportAddrAndLinks(SaveCfgDialog->FileName);

           WriteLog("Експорт книги завершено");
		 }
	  catch (Exception &e)
		 {
		   WriteLog("Експорт книги: " + e.ToString());
		 }
	}

  SaveCfgDialog->Filter = old_mask;
  SaveCfgDialog->FileName = "";
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::StartServer()
{
  try
	 {
	   Listener->DefaultPort = ListenPort;
	   Listener->Active = true;
	   WriteLog("Сервер запущено");

	   SwServerOn->Show();
	   SwServerOff->Hide();
	 }
  catch (Exception &e)
	 {
	   SwServerOn->Hide();
	   SwServerOff->Show();

	   WriteLog("Запуск серверу: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::StopServer()
{
  try
	 {
	   Listener->Active = false;
	   WriteLog("Сервер зупинено");

	   SwServerOff->Show();
	 }
  catch (Exception &e)
	 {
	   SwServerOff->Show();

	   WriteLog("Зупинка серверу: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ReadSettings()
{
  try
	 {
       TRegistry *reg = new TRegistry(KEY_READ);

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (reg->OpenKey("Software\\AFAConfigServer\\Form", false))
			  {
				if (reg->ValueExists("FullScreen"))
				  FullScreen = reg->ReadBool("FullScreen");
				else
                  FullScreen = false;

				if (reg->ValueExists("HideWindow"))
				  HideWnd = reg->ReadBool("HideWindow");
				else
				  HideWnd = false;

				if (reg->ValueExists("Height"))
				  ClientHeight = reg->ReadInteger("Height");
				else
				  ClientHeight = 600;

				if (reg->ValueExists("Width"))
				  ClientWidth = reg->ReadInteger("Width");
				else
				  ClientWidth = 800;

				reg->CloseKey();
			  }

			if (reg->OpenKey("Software\\AFAConfigServer\\Params", false))
			  {
				if (reg->ValueExists("ListenPort"))
				  ListenPort = reg->ReadInteger("ListenPort");
				else
				  reg->WriteInteger("ListenPort", 7896);

				if (reg->ValueExists("ActivityCheckInterval"))
				  ActivityCheckInterval = reg->ReadInteger("ActivityCheckInterval");
				else
				  ActivityCheckInterval = 1;

				reg->CloseKey();
			  }
		  }
	   __finally {delete reg;}
	 }
  catch (Exception &e)
	 {
	   WriteLog("ReadSettings: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::WriteSettings()
{
  try
	 {
       TRegistry *reg = new TRegistry();

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (!reg->KeyExists("Software\\AFAConfigServer\\Form"))
			  reg->CreateKey("Software\\AFAConfigServer\\Form");

			if (!reg->KeyExists("Software\\AFAConfigServer\\Params"))
			  reg->CreateKey("Software\\AFAConfigServer\\Params");

			if (reg->OpenKey("Software\\AFAConfigServer\\Form", false))
			  {
				reg->WriteInteger("Height", ClientHeight);
				reg->WriteInteger("Width", ClientWidth);

				if (WindowState == wsMaximized)
				  reg->WriteBool("FullScreen", FullScreen);
				else
				  reg->WriteBool("FullScreen", false);

				reg->CloseKey();
			  }

			if (reg->OpenKey("Software\\AFAConfigServer\\Params", false))
			  {
				reg->WriteInteger("ListenPort", ListenPort);
				reg->WriteInteger("ActivityCheckInterval", ActivityCheckInterval);

				reg->CloseKey();
			  }
		  }
	   __finally {delete reg;}
	 }
  catch (Exception &e)
	 {
	   WriteLog("WriteSettings: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::LogFilterDropDown(TObject *Sender)
{
  TStringList *logs = new TStringList();

  try
	 {
       LogFilter->Clear();
	   GetFileList(logs, LogDir, "*.log", WITHOUT_DIRS, WITHOUT_FULL_PATH);
	   LogFilter->Items->AddStrings(logs);
	 }
  __finally {delete logs;}
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::LogFilterChange(TObject *Sender)
{
  Log->Lines->LoadFromFile(LogDir + "\\" + LogFilter->Items->Strings[LogFilter->ItemIndex],
						   TEncoding::UTF8);
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ListenerExecute(TIdContext *AContext)
{
  String msg;
  TStringStream *ms = new TStringStream("", TEncoding::UTF8, true);
  TStringList *list = new TStringList();

  AContext->Connection->IOHandler->ReadStream(ms);

  try
	 {
	   try
		  {
			ms->Position = 0;
			msg = ms->ReadString(ms->Size);

			if (msg == "")
              throw new Exception("Відсутні дані");

			StrToList(list, msg, "%");

			String host = AContext->Binding->PeerIP;

			if (list->Strings[0] == "#auth")
			  {
				String station = list->Strings[1],
					   index = list->Strings[2],
				   	   port = list->Strings[3];
				int grp_id = -1;

//тепер перевіряємо чи є відповідні група і хост
				RecipientItem *grp = AddrBook->FindGroup(index);

				if (!grp)
				  {
					grp_id = AddrBook->Add(0, AddrList->Items->Add(AddrList->Selected, index), index);
					AddrBookChecker->CollectionChanged = true;
					grp = AddrBook->FindGroup(grp_id);
				  }

				RecipientItem *itm = AddrBook->FindRecipientInGroup(grp->ID, station, host, port);

				if (!itm)
				  {
					itm = AddrBook->FindItem(AddrBook->Add(grp->ID, grp->Node, station, host, port));
					AddrBook->CreateSortedTree(AddrList);
					AddrBookChecker->CollectionChanged = true;
				  }
				else if ((itm->Host != host) || (itm->Port != port))
				  {
					itm->Host = host;
					itm->Port = port;
					AddrBookChecker->CollectionChanged = true;
				  }

                itm->Node->StateIndex = 3;

//надсилаємо хосту перелік файлів
				ms->Clear();
				ms->WriteString(CreateClientFileList(index, station));
                ms->Position = 0;
				AContext->Connection->IOHandler->Write(ms, ms->Size, true);
			  }
			else if (list->Strings[0] == "#request")
			  {
				TFileStream *fs = new TFileStream(list->Strings[1], fmOpenRead);

				try
				   {
					 fs->Position = 0;
					 AContext->Connection->IOHandler->Write(fs, fs->Size, true);
				   }
				__finally {delete fs;}
			  }
			else if (list->Strings[0] == "#getaddrbook")
			  {
				TFileStream *fs = new TFileStream(DataDir + "\\hosts.grp",
												  fmOpenRead|fmShareDenyNone);

				try
				   {
					 fs->Position = 0;
					 AContext->Connection->IOHandler->Write(fs, fs->Size, true);
				   }
				__finally {delete fs;}
			  }
		  }
	   catch (Exception &e)
		  {
			WriteLog("Listener: " + e.ToString());
		  }
	 }
  __finally {delete list; delete ms;}
}
//---------------------------------------------------------------------------

String __fastcall TServerForm::CreateClientFileList(const String &index, const String &station)
{
  String res;

  try
	 {
	   std::vector<ClientConfigLink*> links;
	   String file, size, change, ver;

	   for (int i = 0; i < ConfigManager->Count; i++)
		  {
			ClientConfigLink *lnk = ConfigManager->Items->Links[i];

			if (lnk->IndexVZ == "public")
			  links.push_back(ConfigManager->Items->Links[i]);
			else if	((lnk->IndexVZ == index) && (lnk->StationID == ""))
			  links.push_back(ConfigManager->Items->Links[i]);
			else if ((lnk->IndexVZ == index) && (lnk->StationID == station))
			  links.push_back(ConfigManager->Items->Links[i]);
          }

	   res = "<Data type = 'filelist'>";

	   for (int i = 0; i < links.size(); i++)
		  {
			file = links[i]->FileName;
			size = String(GetFileSize(file));

			change = GetFormattedDate(GetFileDateTime(file), '.', ':', "dd.mm.yyy", "hh:nn:ss");

			ver = GetVersionInString(file.c_str());

			res += "<File size = '" + size +
				   "' version = '" + ver +
				   "' change = '" + change +
				   "'>" + file +
				   "</File>";
		  }

	   res += "</Data>";
	 }
  catch (Exception &e)
	 {
	   res = "";
	   WriteLog("CreateClientFileList(" + index + ", " + station + "): " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ExportAddrAndLinks(const String &file)
{
  try
	 {
	   StatusChecker->Enabled = false;
       AddrBook->Save();
	   ConfigManager->SaveToFile(DataDir + "\\config.links");

	   TFileStream *result_file = new TFileStream(file, fmOpenWrite|fmCreate);

	   try
		  {
			__int64 sz;
			Byte *buf;

			TFileStream *addr_file = new TFileStream(DataDir + "\\hosts.grp", fmOpenRead|fmShareDenyNone);

			try
			   {
				 sz = addr_file->Size;

				 buf = new Byte[sz];

				 addr_file->Read(buf, sz);
				 result_file->Position += result_file->Write(&sz, sizeof(__int64));
				 result_file->Position += result_file->Write(buf, sz);
			   }
			__finally {delete addr_file; delete[] buf;}

			TFileStream *links_file = new TFileStream(DataDir + "\\config.links", fmOpenRead|fmShareDenyNone);

			try
			   {
				 sz = links_file->Size;

				 buf = new Byte[sz];

				 addr_file->Read(buf, sz);
				 result_file->Position += result_file->Write(&sz, sizeof(sz));
				 result_file->Position += result_file->Write(buf, sz);
			   }
			__finally {delete links_file; delete[] buf;}
		  }
	   __finally {delete result_file; StatusChecker->Enabled = true;}
	 }
  catch (Exception &e)
	 {
	   WriteLog("ExportAddrAndLinks: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ImportAddrAndLinks(const String &file)
{
  try
	 {
	   StatusChecker->Enabled = false;

	   TFileStream *result_file = new TFileStream(file, fmOpenRead|fmShareDenyNone);

	   try
		  {
			__int64 sz;
			Byte *buf;

			TFileStream *addr_file = new TFileStream(DataDir + "\\hosts.grp", fmOpenWrite|fmCreate);

			try
			   {
				 result_file->Position += result_file->Read(&sz, sizeof(__int64));

				 buf = new Byte[sz];

				 result_file->Position += result_file->Read(buf, sz);
                 addr_file->Write(buf, sz);
			   }
			__finally {delete addr_file; delete[] buf;}

			TFileStream *links_file = new TFileStream(DataDir + "\\config.links", fmOpenWrite|fmCreate);

			try
			   {
				 result_file->Position += result_file->Read(&sz, sizeof(__int64));

				 buf = new Byte[sz];

				 result_file->Position += result_file->Read(buf, sz);
				 links_file->Write(buf, sz);
			   }
			__finally {delete links_file; delete[] buf;}

			AddrBook->LoadFromFile(DataDir + "\\hosts.grp");
			ConfigManager->LoadFromFile(DataDir + "\\config.links");
		  }
	   __finally {delete result_file; StatusChecker->Enabled = true;}
	 }
  catch (Exception &e)
	 {
	   WriteLog("ImportAddrAndLinks: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ListenerConnect(TIdContext *AContext)
{
  //клієнт під'єднався
  WriteLog("Підключився клієнт: " + AContext->Binding->PeerIP);
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ListenerDisconnect(TIdContext *AContext)
{
  //клієнт від'єднався
  WriteLog("Відключився клієнт: " + AContext->Binding->PeerIP);
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::SettingsClick(TObject *Sender)
{
  SettingsForm->Show();
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::AddConfigClick(TObject *Sender)
{
  try
	 {
	   RecipientItem *itm = AddrBook->Find(AddrList->Selected);

	   if (itm)
		 {
           if (OpenCfgDialog->Execute())
			 {
			   if (itm->ParentNodeID == 0) //вибрана група
				 {
				   ConfigManager->AddLink(itm->Name, "", OpenCfgDialog->FileName);
				   ConfigManager->SaveToFile(DataDir + "\\config.links");
				   AddrListClick(AddrList);
				 }
			   else
				 {
				   RecipientItem *grp = AddrBook->FindGroup(itm->ParentNodeID);

				   if (grp)
					 {
					   ConfigManager->AddLink(grp->Name, itm->Name, OpenCfgDialog->FileName);
                       ConfigManager->SaveToFile(DataDir + "\\config.links");
                       AddrListClick(AddrList);
					 }
				   else
					 throw new Exception("Невідомий ID групи");
				 }
			 }
		 }
	   else
		 throw new Exception("Невідомий ID отримувача");
	 }
  catch (Exception &e)
	 {
	   WriteLog("Додання конфігу: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::RemoveConfigClick(TObject *Sender)
{
  try
	 {
	   if (!ConfigList->Selected)
		 return;

	   String file = ConfigList->Items->Item[ConfigList->Selected->Index]->Caption;

	   RecipientItem *itm = AddrBook->Find(AddrList->Selected);

	   if (itm)
		 {
		   if (itm && !itm->ParentNodeID) //обрана група
			 {
			   ConfigManager->RemoveLink(itm->Name, "", file);
			   ConfigManager->SaveToFile(DataDir + "\\config.links");
               AddrListClick(AddrList);
			 }
		   else if (itm && itm->ParentNodeID) //обрано запис
			 {
			   RecipientItem *grp = AddrBook->FindGroup(itm->ParentNodeID);

			   if (grp)
				 {
				   ConfigManager->RemoveLink(grp->Name, itm->Name, file);
                   ConfigManager->SaveToFile(DataDir + "\\config.links");
				   AddrListClick(AddrList);
				 }
			   else
				 throw new Exception("Невідомий ID групи");
			 }
		 }
	   else
		 throw new Exception("Невідомий ID");
	 }
  catch (Exception &e)
	 {
	   WriteLog("Вилучення конфігу: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::StatusCheckerTimer(TObject *Sender)
{
  try
	 {
	   std::vector<int> id_list;

	   id_list.clear();

	   AddrBook->SelectRecipients(&id_list);

	   TStringStream *ms = new TStringStream("#status", TEncoding::UTF8, true);

	   try
		  {
			int status;

			for (int i = 0; i < id_list.size(); i++)
			   {
				 RecipientItem *itm = AddrBook->FindItem(id_list[i]);

				 if (itm)
				   {
					 status = AskToClient(itm->Host.c_str(), itm->Port.ToInt(), ms);

					 if (status < 0)
					   itm->Node->StateIndex = 4;
					 else if (ms->ReadString(ms->Size) == "#ok")
					   itm->Node->StateIndex = 3;
				   }
			   }
		  }
	   __finally {delete ms;}

	   if (Date().CurrentDate() > DateStart) //уточнюємо дату для логу
		 {
		   DateStart = Date().CurrentDate();
		   LogFile = DateToStr(DateStart) + ".log";
		 }
	 }
  catch (Exception &e)
	 {
	   WriteLog("StatusCheckerTimer:" + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::MainPopupMenuPopup(TObject *Sender)
{
  if (Listener->Active)
	PPStatus->ImageIndex = 3;
  else
    PPStatus->ImageIndex = 4;
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::PPShowWindowClick(TObject *Sender)
{
  if ((WindowState == wsMaximized) || (WindowState == wsNormal))
	{
	  Hide();
	  WindowState = wsMinimized;
	}
  else if (FullScreen)
	{
	  WindowState = wsMaximized;
	  Show();
	}
  else
	{
	  WindowState = wsNormal;
      Show();
	}
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::PPCloseClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::TrayIconDblClick(TObject *Sender)
{
  PPShowWindow->Click();
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::RefreshLogClick(TObject *Sender)
{
  Log->Lines->LoadFromFile(LogDir + "\\" + LogFilter->Items->Strings[LogFilter->ItemIndex],
						   TEncoding::UTF8);
}
//---------------------------------------------------------------------------

