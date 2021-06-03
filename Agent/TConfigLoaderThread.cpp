/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#include <System.hpp>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "..\..\work-functions\ThreadSafeLog.h"
#include "TConfigLoaderThread.h"
#include "Main.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

extern TMainForm *MainForm;
extern TThreadSafeLog *Log;
extern String StationID, IndexVZ, ConfigServerHost, AppName, UpdAppName;
extern int RemAdmPort, ConfigServerPort;

//---------------------------------------------------------------------------
__fastcall TConfigLoaderThread::TConfigLoaderThread(bool CreateSuspended)
	: TThread(CreateSuspended)
{
}
//---------------------------------------------------------------------------

int __fastcall TConfigLoaderThread::GetConfigurationFromServer()
{
  int res = 1;

  try
	 {
	   auto ms = std::make_unique<TStringStream>("", TEncoding::UTF8, true);

	   ms->WriteString("#auth%" + StationID + "%" + IndexVZ + "%" + IntToStr(RemAdmPort));
	   ms->Position = 0;

	   if (AskFromHost(ConfigServerHost.c_str(), ConfigServerPort, ms.get()))
		 {
		   ms->Position = 0;

		   if (ms->Size == 0)
			 throw Exception("Відсутні дані");

		   WorkWithFileList(ms->ReadString(ms->Size));
		 }
	   else
		 throw Exception("Помилка зв'язку");
	 }
  catch (Exception &e)
	 {
	   res = 0;
	   Log->Add("Отримання даних від сервера конфігурацій: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::WorkWithFileList(const String &xml_str)
{
  try
	 {
	   if (xml_str == "")
		 throw Exception("Перелік файлів не надійшов");

	   Log->Add("Надійшов перелік файлів від сервера конфігурацій. Обробка");

       std::vector<ReceivedFile> remote_files = XMLImportFileList(xml_str);

	   RemoveUnnecessaryFiles(&remote_files);

	   for (int i = 0; i < remote_files.size(); i++)
		  {
			if (CompareLocalAndRemoteFiles(remote_files[i]))
			  LoadFileFromConfigurationServer(&remote_files[i]);
			else
			  CheckAndRunExistConfig(remote_files[i].Name);
		  }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Обробка переліку файлів від сервера конфігурацій: " + e.ToString());
       throw e;
	 }
}
//---------------------------------------------------------------------------

std::vector<ReceivedFile> __fastcall TConfigLoaderThread::XMLImportFileList(String xml_text)
{
  std::vector<ReceivedFile> res;
  auto ixml = std::make_unique<TXMLDocument>(Application);
  auto lst = std::make_unique<TStringList>();

  try
	 {
	   if (CoInitializeEx(NULL, COINIT_MULTITHREADED) != S_OK)
		 throw Exception("Помилка CoInitializeEx");

	   try
		  {
			if (xml_text == "")
			  throw Exception("Відсутні дані");

			ixml->DOMVendor = GetDOMVendor("MSXML");
            ixml->LoadFromXML(xml_text);
			ixml->Active = true;
			ixml->Encoding = "UTF-8";
			ixml->Options = ixml->Options << doNodeAutoIndent;

			_di_IXMLNode List = ixml->DocumentElement;
			_di_IXMLNode File;

			String text, ver;
			ReceivedFile listfile;

			for (int i = 0; i < List->ChildNodes->Count; i++)
			   {
				 File = List->ChildNodes->Nodes[i];

				 listfile.Name = File->NodeValue;
				 listfile.Size = static_cast<int>(File->GetAttribute("size"));
				 listfile.Changed = GetFormattedDate(static_cast<const wchar_t*>(File->GetAttribute("change")),
													 '.',
													 ':',
													 "dd.mm.yyy",
													 "hh:nn:ss");

				 ver = File->GetAttribute("version");

				 if (ver == "no_data")
				   ver = "0.0.0.0";

				 lst->Clear();
				 StrToList(lst.get(), ver, ".");

				 listfile.Version[0] = lst->Strings[0].ToInt();
				 listfile.Version[1] = lst->Strings[1].ToInt();
				 listfile.Version[2] = lst->Strings[2].ToInt();
				 listfile.Version[3] = lst->Strings[3].ToInt();

				 res.push_back(listfile);
			   }
		  }
	   catch (Exception &e)
		  {
			Log->Add("Імпорт переліку файлів з XML: " + e.ToString());
		  }
	 }
  __finally {CoUninitialize();}

  return res;
}
//---------------------------------------------------------------------------

bool __fastcall TConfigLoaderThread::CompareLocalAndRemoteFiles(ReceivedFile &recvd_file)
{
  bool res;

  try
	 {
	   String name = GetFileNameFromFilePath(recvd_file.Name);
	   String local_file = DataPath + "\\" + name;

	   int local_version[4] = {0,0,0,0};
	   GetAppVersion(local_file.c_str(), local_version);

	   if (!FileExists(local_file, false))
		 res = true;
	   else if (CompareVersions(local_version, recvd_file.Version) == 2)
		 res = true;
	   else if (GetFileDateTime(local_file) < recvd_file.Changed)
		 res = true;
	   else if (GetFileSize(local_file) < recvd_file.Size)
		 res = true;
	   else
		 res = false;
	 }
  catch (Exception &e)
	 {
	   res = false;

	   Log->Add("Порівняння віддаленого та локального файлів: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::LoadFileFromConfigurationServer(ReceivedFile *file)
{
  String local_file = GetFileNameFromFilePath(file->Name);

  try
	{
	  auto ms = std::make_unique<TStringStream>("#request%" + file->Name, TEncoding::UTF8, true);

	   if (AskFromHost(ConfigServerHost.c_str(), ConfigServerPort, ms.get()))
		 {
		   String ext = UpperCase(GetFileExtensionFromFileName(local_file));

		   if (UpperCase(local_file) == "ELI.DLL")
			 MainForm->ReleaseELI();
		   else if ((ext == "EXE") && (UpperCase(local_file) != UpperCase(AppName)))
		     StopRunningModule(local_file);

		   ms->Position = 0;
		   ms->SaveToFile(DataPath + "\\" + local_file);

//змінюємо дату та час файла на ті, що були у оригінала на сервері
		   if (SetFileDateTime(DataPath + "\\" + local_file, file->Changed) <= 0)
			 Log->Add("Не вдалось змінити дату/час файлу: " + DataPath + "\\" + local_file);

		   Log->Add("З серверу конфігурацій завантажено файл: " + local_file);

		   ProcessLoadedFile(local_file);
		 }
	}
  catch (Exception &e)
	{
	  Log->Add("Помилка завантаження файлу " + local_file + ": " + e.ToString());
	}
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::ProcessLoadedFile(const String &file_name)
{
  try
	 {
	   Log->Add("Обробка файлу " + file_name);

	   String ext = UpperCase(GetFileExtensionFromFileName(file_name));

	   if (ext == "ES") //керуючий скрипт
		 MainForm->ExecuteScript(file_name);
	   else if ((ext == "CFG") && (UpperCase(file_name) != "MAIN.CFG")) //конфіг з'єднання
		 MainForm->RebuildConnection(file_name);
	   else if (UpperCase(file_name) == UpperCase(AppName))
		 MainForm->UpdateRequest();
	   else if (UpperCase(file_name) == "MAIN.CFG")
		 MainForm->ReadConfig();
	   else if ((ext == "EXE") || (ext == "BAT") || (ext == "CMD"))
		 RunModule(file_name);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Обробка файлу " + file_name + ": " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::CheckAndRunExistConfig(const String &remote_file)
{
  try
	 {
	   String name = GetFileNameFromFilePath(remote_file);
	   String ext = UpperCase(GetFileExtensionFromFileName(name));

	   if (ext != "CFG")
		 return;
	   else if (UpperCase(name) == "MAIN.CFG")
		 return;
	   else
		 MainForm->CheckAndStartConnection(name);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Обробка файлу з серверу конфігурацій: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

bool __fastcall TConfigLoaderThread::IsModuleRunning(const String &file_name)
{
  bool res;

  try
	 {
	   DWORD pid = GetProcessByExeName(file_name.c_str());

	   if (pid)
		 res = true;
	   else
		 res = false;
	 }
  catch (Exception &e)
	 {
	   res = false;
	   Log->Add("Перевірка стану модуля: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::StopRunningModule(const String &file_name)
{
  try
	 {
	   DWORD pid = GetProcessByExeName(file_name.c_str());

	   if (pid)
		 {
		   Log->Add("Модуль " + file_name + " в роботі. Вимкнення модуля");

		   HWND handle = FindHandleByPID(pid);

		   if (handle)
			 PostMessage(handle, WM_QUIT, 0, 0);

		   Sleep (1000);

		   if (FindHandleByPID(pid))
			 {
			   HANDLE proc = OpenProcess(PROCESS_TERMINATE, 0, pid);

			   try
				  {
					Log->Add("Знищення процесу модуля " + file_name);
					TerminateProcess(proc, 0);
				  }
			   __finally {if (proc) CloseHandle(proc);}
			 }
		   else
			 Log->Add("Модуль " + file_name + " вимкнено");
		 }
	   else
		 Log->Add("Модуль " + file_name + " не запущено");
	 }
  catch (Exception &e)
	 {
	   Log->Add("Зупинка допоміжного модуля: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::RunModule(const String &file_name)
{
  try
	 {
	   Log->Add("Запуск допоміжного модуля: " + file_name);
	   StartProcessByExeName(DataPath + "\\" + file_name);
	 }
  catch (Exception &e)
	 {
	   Log->Add("Запуск допоміжного модуля: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::RemoveUnnecessaryFiles(std::vector<ReceivedFile> *remote_files)
{
  try
	 {
	   Log->Add("Видалення зайвих файлів");

	   auto check_list = std::make_unique<TStringList>();

	   for (int i = 0; i < remote_files->size(); i++)
		  check_list->Add(GetFileNameFromFilePath(remote_files->at(i).Name));

	   auto modules = std::make_unique<TStringList>();

	   GetFileList(modules.get(), DataPath, "*", WITHOUT_DIRS, WITHOUT_FULL_PATH);

	   bool in_list;

	   for (int i = 0; i < modules->Count; i++)
		  {
			if (UpperCase(modules->Strings[i]) == UpperCase(AppName))
			  continue;

			in_list = false;

			for (int j = 0; j < check_list->Count; j++)
			   {
				 if (UpperCase(modules->Strings[i]) == UpperCase(check_list->Strings[j]))
				   in_list = true;
			   }

			if (!in_list)
			  StopRunningModule(modules->Strings[i]);
		  }

	   DeleteFilesExceptList(DataPath, check_list.get());
	 }
  catch (Exception &e)
	{
	  Log->Add("Видалення зайвих файлів: " + e.ToString());
	}
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::CheckAndRunExistModules()
{
  auto files = std::make_unique<TStringList>();
  String str;

  try
	 {
	   GetFileList(files.get(), DataPath, "*.bat", WITHOUT_DIRS, WITHOUT_FULL_PATH);
	   GetFileList(files.get(), DataPath, "*.cmd", WITHOUT_DIRS, WITHOUT_FULL_PATH);
	   GetFileList(files.get(), DataPath, "*.exe", WITHOUT_DIRS, WITHOUT_FULL_PATH);

	   for (int i = 0; i < files->Count; i++)
		  {
			if (UpperCase(files->Strings[i]) != UpperCase(AppName))
			  {
                try
				   {
					 if (!IsModuleRunning(files->Strings[i]))
					   RunModule(files->Strings[i]);
				   }
				catch (Exception &e)
				   {
					 Log->Add("Перевірка та запуск допоміжного модуля " +
							  files->Strings[i] +
							  ": " + e.ToString());
				   }
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Перевірка та запуск допоміжного модуля: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TConfigLoaderThread::Execute()
{
  FInterval = 10 * 60000;
  FPassed = FInterval;

  Log->Add("Перевірка локальних файлів для оновлення");
  MainForm->UpdateRequest();

  Log->Add("Старт наявних допоміжних модулів");
  CheckAndRunExistModules();

  try
	 {
       while (!Terminated)
		 {
		   if (FPassed >= FInterval)
			 {
			   FConnected = GetConfigurationFromServer();
			   FPassed = 0;
			 }
		   else
			 {
			   this->Sleep(100);
			   FPassed += 100;
			 }
		 }
	 }
  catch (Exception &e)
	 {
	   Log->Add("Помилка потоку TConfigLoaderThread, ID =" +
				IntToStr((int)this->ThreadID) + ": " + e.ToString());
	 }
}
//---------------------------------------------------------------------------
