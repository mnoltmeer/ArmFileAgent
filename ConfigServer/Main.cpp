/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "MyFunc.h"
#include "RecpOrganizer.h"
#include "RecpThread.h"
#include "ClientConfigLinks.h"
#include "StatusChecker.h"
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
TStatusCheckThread *StatusChecker;
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

	   if (!DirectoryExists(DataDir))
		 CreateDir(DataDir);

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
  if (Date().CurrentDate() > DateStart) //уточнюємо дату для логу
	{
	  DateStart = Date().CurrentDate();
	  LogFile = DateToStr(DateStart) + ".log";
	}

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

       StatusChecker = new TStatusCheckThread(true);
	   StatusChecker->Collection = AddrBook;
	   StatusChecker->CheckInterval = ActivityCheckInterval * 60000;
	   StatusChecker->Resume();
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

	   if (StatusChecker)
		 {
		   if (StatusChecker->Started)
			 {
			   StatusChecker->Terminate();

			   HANDLE hThread = reinterpret_cast<HANDLE>(StatusChecker->Handle);

			   DWORD wait = WaitForSingleObject(hThread, 300);

			   if (wait == WAIT_TIMEOUT)
				 TerminateThread(hThread, 0);
			 }

		   delete StatusChecker;
		 }

	   StopServer();

	   if (AddrBookChecker)
		 {
		   if (AddrBookChecker->Started)
			 {
			   AddrBookChecker->Terminate();

			   HANDLE hThread = reinterpret_cast<HANDLE>(AddrBookChecker->Handle);

			   DWORD wait = WaitForSingleObject(hThread, 300);

			   if (wait == WAIT_TIMEOUT)
				 TerminateThread(hThread, 0);
			 }

		   delete AddrBookChecker;
		 }

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
				ClearClientInfo();

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

		   ShowClientInfo(itm->Name, grp->Name, itm->Host, itm->Port);
		 }
	   else
		 throw Exception("Невідомий ID");
	 }
  catch (Exception &e)
	 {
	   WriteLog(e.ToString());
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

			   std::unique_ptr<TStringList> old_book(new TStringList());
			   std::unique_ptr<TStringList> lst(new TStringList());

			   old_book->LoadFromFile(OpenCfgDialog->FileName, TEncoding::UTF8);

			   for (int i = 0; i < old_book->Count; i++)
				  {
					lst->Clear();
					StrToList(lst.get(), old_book->Strings[i], ";");
					grp = AddrBook->FindGroup("Несортоване");
					AddrBook->Add(grp->ID, grp->Node, lst->Strings[0], lst->Strings[1], lst->Strings[2]);

					WriteLog("Імпортовано запис: " + lst->Strings[0]);
				  }

			   WriteLog("Імпорт книги завершено");

			   AddrBook->Save();

			   AddrBook->CreateSortedTree(AddrList);
			   grp = AddrBook->FindGroup("Несортоване");
			   grp->Node->Expand(true);
			 }
		   else if (file_ext == "grp")
			 {
			   std::unique_ptr<TRecpientItemCollection> ImportBook(new TRecpientItemCollection(OpenCfgDialog->FileName));

			   AddrBook->ImportData(ImportBook.get());
			   AddrBook->CreateSortedTree(AddrList);
			   AddrBook->Save();
			   WriteLog("Імпорт книги завершено");
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

void __fastcall TServerForm::ExportHostStatus(const String &file)
{
  try
	 {
	   std::unique_ptr<TFileStream> fs(new TFileStream(file, fmOpenWrite|fmCreate));
	   unsigned int id, status;

	   for (int i = 0; i < AddrBook->Count; i++)
		  {
			if (AddrBook->Items[i]->ParentNodeID != 0)
			  {
				id = AddrBook->Items[i]->ID;

				if (AddrBook->Items[i]->Node->StateIndex == 3)
				  status = 1;
				else
				  status = 0;

				fs->Position += fs->Write(&id, sizeof(unsigned int));
				fs->Position += fs->Write(&status, sizeof(unsigned int));
			  }
		  }

	   WriteLog("Експорт статусів завершено");
	 }
  catch (Exception &e)
	 {
	   WriteLog("Експорт статусів: " + e.ToString());
	 }
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
	   std::unique_ptr<TRegistry> reg(new TRegistry(KEY_READ));

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
	   std::unique_ptr<TRegistry> reg(new TRegistry());

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
  catch (Exception &e)
	 {
	   WriteLog("WriteSettings: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::LogFilterDropDown(TObject *Sender)
{
  std::unique_ptr<TStringList> logs(new TStringList());

  LogFilter->Clear();
  GetFileList(logs.get(), LogDir, "*.log", WITHOUT_DIRS, WITHOUT_FULL_PATH);
  LogFilter->Items->AddStrings(logs.get());
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

  std::unique_ptr<TStringStream> ms(new TStringStream("", TEncoding::UTF8, true));
  std::unique_ptr<TStringList> list(new TStringList());

  AContext->Connection->IOHandler->ReadStream(ms.get());

  try
	 {
	   ms->Position = 0;
	   msg = ms->ReadString(ms->Size);

	   if (msg == "")
		 throw Exception("Відсутні дані");

	   StrToList(list.get(), msg, "%");

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

		   RecipientItem *itm = AddrBook->FindRecipientInGroup(grp->ID, station);

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

			   if (itm->Node == AddrList->Selected)
			     ShowClientInfo(itm->Name, grp->Name, itm->Host, itm->Port);
			 }

		   itm->Node->StateIndex = 3;

//надсилаємо хосту перелік файлів
		   ms->Clear();
		   ms->WriteString(CreateClientFileList(index, station));
		   ms->Position = 0;
		   AContext->Connection->IOHandler->Write(ms.get(), ms->Size, true);
		 }
	   else if (list->Strings[0] == "#request")
		 {
		   std::unique_ptr<TFileStream> fs(new TFileStream(list->Strings[1],
														   fmOpenRead|fmShareDenyNone));

		   fs->Position = 0;
		   AContext->Connection->IOHandler->Write(fs.get(), fs->Size, true);
		 }
	   else if (list->Strings[0] == "#getaddrbook")
		 {
		   std::unique_ptr<TFileStream> fs(new TFileStream(DataDir + "\\hosts.grp",
														   fmOpenRead|fmShareDenyNone));

		   fs->Position = 0;
		   AContext->Connection->IOHandler->Write(fs.get(), fs->Size, true);
		 }
	   else if (list->Strings[0] == "#gethoststatus")
		 {
		   ExportHostStatus(DataDir + "\\hosts.sts");

		   std::unique_ptr<TFileStream> fs(new TFileStream(DataDir + "\\hosts.sts",
														   fmOpenRead|fmShareDenyNone));

		   fs->Position = 0;
		   AContext->Connection->IOHandler->Write(fs.get(), fs->Size, true);
		 }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Listener: " + e.ToString());
	 }
  catch (std::exception &e)
	 {
       WriteLog("Listener: " + String(e.what()));
	 }
  catch (...)
	 {
      ShowMessage("Unexpected error");
	 }
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
	   StatusChecker->Suspend();
       AddrBook->Save();
	   ConfigManager->SaveToFile(DataDir + "\\config.links");

	   std::unique_ptr<TFileStream> result_file(new TFileStream(file, fmOpenWrite|fmCreate));

	   try
		  {
			__int64 sz;

			std::unique_ptr<TFileStream> addr_file(new TFileStream(DataDir + "\\hosts.grp",
																   fmOpenRead|fmShareDenyNone));

			sz = addr_file->Size;

			std::unique_ptr<Byte[]> buf(new Byte[sz]);

			addr_file->Read(buf.get(), sz);
			result_file->Position += result_file->Write(&sz, sizeof(__int64));
			result_file->Position += result_file->Write(buf.get(), sz);
			buf.release();

			std::unique_ptr<TFileStream> links_file(new TFileStream(DataDir + "\\config.links",
																   fmOpenRead|fmShareDenyNone));

			sz = links_file->Size;

			std::unique_ptr<Byte[]> buf2(new Byte[sz]);

			addr_file->Read(buf2.get(), sz);
			result_file->Position += result_file->Write(&sz, sizeof(sz));
			result_file->Position += result_file->Write(buf2.get(), sz);
		  }
	   __finally {StatusChecker->Resume();}
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
	   StatusChecker->Suspend();

	   std::unique_ptr<TFileStream> result_file(new TFileStream(file, fmOpenRead|fmShareDenyNone));

	   try
		  {
			__int64 sz;
			std::unique_ptr<TFileStream> addr_file(new TFileStream(DataDir + "\\hosts.grp",
																   fmOpenWrite|fmCreate));

			result_file->Position += result_file->Read(&sz, sizeof(__int64));

			std::unique_ptr<Byte[]> buf(new Byte[sz]);

			result_file->Position += result_file->Read(buf.get(), sz);
			addr_file->Write(buf.get(), sz);
			buf.release();

            std::unique_ptr<TFileStream> links_file(new TFileStream(DataDir + "\\config.links",
																	fmOpenWrite|fmCreate));

			result_file->Position += result_file->Read(&sz, sizeof(__int64));

			std::unique_ptr<Byte[]> buf2(new Byte[sz]);

			result_file->Position += result_file->Read(buf2.get(), sz);
			links_file->Write(buf2.get(), sz);

			AddrBook->LoadFromFile(DataDir + "\\hosts.grp");
			ConfigManager->LoadFromFile(DataDir + "\\config.links");
		  }
	   __finally {StatusChecker->Resume();}
	 }
  catch (Exception &e)
	 {
	   WriteLog("ImportAddrAndLinks: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ShowClientInfo(const String &station, const String &index,
							   				const String &host, const String &port)
{
  ItemParams->Cells[1][1] = index;
  ItemParams->Cells[1][2] = station;
  ItemParams->Cells[1][3] = host;
  ItemParams->Cells[1][4] = port;
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::ClearClientInfo()
{
  ItemParams->Cells[1][1] = "";
  ItemParams->Cells[1][2] = "";
  ItemParams->Cells[1][3] = "";
  ItemParams->Cells[1][4] = "";
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
  SettingsForm->Left = Left + ClientWidth / 2 - SettingsForm->ClientWidth / 2;
  SettingsForm->Top = Top + ClientHeight / 2 - SettingsForm->ClientHeight / 2;
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::AddConfigClick(TObject *Sender)
{
  try
	 {
	   RecipientItem *itm = AddrBook->Find(AddrList->Selected);

	   if (itm)
		 {
           OpenCfgDialog->FileName = "";

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
					 throw Exception("Невідомий ID групи");
				 }
			 }
		 }
	   else
		 throw Exception("Невідомий ID отримувача");
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
				 throw Exception("Невідомий ID групи");
			 }
		 }
	   else
		 throw Exception("Невідомий ID");
	 }
  catch (Exception &e)
	 {
	   WriteLog("Вилучення конфігу: " + e.ToString());
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

void __fastcall TServerForm::ExpandAllClick(TObject *Sender)
{
  try
	 {
	   for (int i = 0; i < AddrBook->Count; i++)
		  {
			AddrBook->Items[i]->Node->Expand(true);
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("ExpandAllClick: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TServerForm::CollapseAllClick(TObject *Sender)
{
  try
	 {
	   for (int i = 0; i < AddrBook->Count; i++)
		  {
			AddrBook->Items[i]->Node->Collapse(true);
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("CollapseAllClick: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

