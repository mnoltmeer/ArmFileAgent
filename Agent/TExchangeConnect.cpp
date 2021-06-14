/*!
Copyright 2019-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "TExchangeConnect.h"

TExchangeConnect::TExchangeConnect(int ID, TThreadSafeLog *Log)
{
  this->Log = Log;
  this->ID = FID;
  this->Running = false;
  this->FthID = 0;

  SuccessFiles = new TStringList();

  try
	 {
	   FtpLoader = new TIdFTP();
	 }
  catch (Exception &e)
	 {
	   WriteLog("Не вдалося створити об'єкт TIdFTP");
	 }

  Init = false;
}
//---------------------------------------------------------------------------

TExchangeConnect::TExchangeConnect(String cfg_file, int ID, TThreadSafeLog *Log)
{
  this->Log = Log;
  this->FID = ID;
  this->Running = false;
  this->FthID = 0;
  FCfgPath = cfg_file;

  SuccessFiles = new TStringList();

  try
	 {
	   FtpLoader = new TIdFTP();
	 }
  catch (Exception &e)
	 {
	   WriteLog("Не вдалося створити об'єкт TIdFTP");
	 }

  try
	 {
	   Initialize();
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка під час ініціалізації");
	 }
}
//---------------------------------------------------------------------------

TExchangeConnect::~TExchangeConnect()
{
  if (Working())
  	Stop();

  if (FtpLoader)
	delete FtpLoader;

  if (SuccessFiles)
	delete SuccessFiles;
}
//---------------------------------------------------------------------------

void TExchangeConnect::SetFtpLoader()
{
  FtpLoader->Port = FConfig.Port;

  if (FtpLoader->Port == 0)
	FtpLoader->Port = 21;

  FtpLoader->Host = FConfig.Host;
  FtpLoader->Username = FConfig.User;
  FtpLoader->Password = FConfig.Pwd;
  FtpLoader->Passive = true;
  FtpLoader->AutoLogin = true;
  FtpLoader->TransferType = TIdFTPTransferType(FConfig.TransType);
}
//---------------------------------------------------------------------------

void TExchangeConnect::CheckConfig(String cfg_file)
{
  const int CfgPrmCnt = 30;

  const wchar_t *CfgParams[CfgPrmCnt] = {L"Caption",
										 L"FtpHost",
                                         L"FtpPort",
										 L"FtpUser",
										 L"FtpPass",
										 L"RemDirDl",
										 L"RemDirUl",
										 L"LocDirDl",
										 L"LocDirUl",
										 L"BackUpDirDl",
										 L"BackUpDirUl",
										 L"TransferType",
										 L"MonitoringInterval",
										 L"UploadFilesMask",
										 L"DownloadFilesMask",
										 L"LeaveRemoteFiles",
										 L"LeaveLocalFiles",
										 L"EnableDownload",
										 L"EnableUpload",
										 L"BackUpDl",
										 L"BackUpUl",
										 L"RunOnce",
										 L"StartAtTime",
										 L"DownloadFromSubDirs",
										 L"SaveWithSubDirs",
										 L"BackUpKeepDays",
										 L"RegExDL",
										 L"RegExUL",
										 L"AppendModeDL",
										 L"AppendModeUL"};

  try
	 {
	   for (int i = 0; i < CfgPrmCnt; i++)
		  {
			if (GetConfigLine(cfg_file, CfgParams[i]) == "^no_line")
			  AddConfigLine(cfg_file, CfgParams[i], "0");
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка перевірки конфігу з " + cfg_file +
				" помилка: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int TExchangeConnect::ReadConfig(String cfg_file)
{
  int result = 0;

  CheckConfig(cfg_file);

  try
	 {
	   if (ReadParameter(cfg_file, "Caption", &FConfig.Caption, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру Caption: " + String(GetLastReadParamResult()));
		   result = -1;
         }

	   if (ReadParameter(cfg_file, "FtpHost", &FConfig.Host, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру FtpHost: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "FtpPort", &FConfig.Port, TT_TO_INT) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру FtpPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "FtpUser", &FConfig.User, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру FtpUser: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "FtpPass", &FConfig.Pwd, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру FtpPass: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   String type;

	   if (ReadParameter(cfg_file, "TransferType", &type, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру TransferType: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 {
		   type = UpperCase(type);

		   if (type == "ASCII")
			 FConfig.TransType = ftASCII;
		   else if (type == "BINARY")
			 FConfig.TransType = ftBinary;
		   else
		 	 FConfig.TransType = ftBinary;
         }

	   if (ReadParameter(cfg_file, "LeaveRemoteFiles", &FConfig.LeaveRemoteFiles, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру LeaveRemoteFiles: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "LeaveLocalFiles", &FConfig.LeaveLocalFiles, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру LeaveLocalFiles: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "MonitoringInterval", &FConfig.MonitoringInterval, TT_TO_INT) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру MonitoringInterval: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 {
		   FConfig.MonitoringInterval = FConfig.MonitoringInterval * 60000;
         }

	   if (ReadParameter(cfg_file, "RunOnce", &FConfig.RunOnce, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру RunOnce: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

       String str;

	   if (ReadParameter(cfg_file, "StartAtTime", &str, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру StartAtTime: " +
					String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (str != "")
		 {
		   short h, m;
		   std::unique_ptr<TStringList> lst(new TStringList());
		   //auto lst = std::make_unique<TStringList>();

		   StrToList(lst.get(), str, ":");

		   try
			  {
				h = lst->Strings[0].ToInt();
				m = lst->Strings[1].ToInt();
				FConfig.StartAtTime = TTime(h, m, 0, 0);
			  }
		   catch (Exception &e)
			  {
				FConfig.StartAtTime = -1;
				WriteLog("Помилка створення параметру StartAtTime: " + e.ToString());
			  }
		 }
	   else
		 {
		   FConfig.StartAtTime = -1;
		 }

	   if (ReadParameter(cfg_file, "DownloadFromSubDirs", &FConfig.SubDirsDl, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру DownloadFromSubDirs: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "SaveWithSubDirs", &FConfig.SubDirsCrt, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру SaveWithSubDirs: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "BackUpKeepDays", &FConfig.BackUpKeepDays, TT_TO_INT) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру BackUpKeepDays: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "EnableDownload", &FConfig.EnableDownload, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру EnableDownload: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (FConfig.EnableDownload)
		 {
		   if (ReadParameter(cfg_file, "RemDirDl", &FConfig.RemDirDl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RemDirDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "LocDirDl", &FConfig.LocDirDl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру LocDirDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpDirDl", &FConfig.BackUpDirDl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpDirDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "DownloadFilesMask", &FConfig.DownloadFilesMask, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру DownloadFilesMask: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "RegExDL", &FConfig.RegExDL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RegExDL: " + String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpDl", &FConfig.BackUpDl, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "AppendModeDL", &FConfig.AppendModeDL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру AppendModeDL: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }
		 }

	   if (ReadParameter(cfg_file, "EnableUpload", &FConfig.EnableUpload, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру EnableUpload: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (FConfig.EnableUpload)
		 {
		   if (ReadParameter(cfg_file, "RemDirUl", &FConfig.RemDirUl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RemDirUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "LocDirUl", &FConfig.LocDirUl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру LocDirUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpDirUl", &FConfig.BackUpDirUl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpDirUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "UploadFilesMask", &FConfig.UploadFilesMask, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру UploadFilesMask: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "RegExUL", &FConfig.RegExUL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RegExUL: " + String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpUl", &FConfig.BackUpUl, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "AppendModeUL", &FConfig.AppendModeUL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру AppendModeUL: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }
		 }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка читання конфігу з " + cfg_file +
				" помилка: " + e.ToString());

	   return -1;
	 }

  return result;
}
//---------------------------------------------------------------------------

int TExchangeConnect::CreateServerCfgDirs()
{
  try
	{
	  if (FConfig.EnableDownload && !DirectoryExists(FConfig.LocDirDl))
		if (!ForceDirectories(FConfig.LocDirDl))
		  {
			WriteLog("Не вдалося створити директорію: " + FConfig.LocDirDl);
		  }

	  if (FConfig.EnableUpload && !DirectoryExists(FConfig.LocDirUl))
		if (!ForceDirectories(FConfig.LocDirUl))
		  {
			WriteLog("Не вдалося створити директорію: " + FConfig.LocDirUl);
		  }

	  if (FConfig.BackUpDl && (FConfig.BackUpDirDl != ""))
		if (!DirectoryExists(FConfig.BackUpDirDl))
		  if (!ForceDirectories(FConfig.BackUpDirDl))
			{
			  WriteLog("Не вдалося створити директорію: " + FConfig.BackUpDirDl);
			}

	  if (FConfig.BackUpUl && (FConfig.BackUpDirUl != ""))
		if (!DirectoryExists(FConfig.BackUpDirUl))
		  if (!ForceDirectories(FConfig.BackUpDirUl))
			{
			  WriteLog("Не вдалося створити директорію: " + FConfig.BackUpDirUl);
			}
	}
  catch(Exception &e)
	{
	  WriteLog("Помилка створення директорій: " + e.ToString());

	  return -1;
	}

  return 0;
}
//---------------------------------------------------------------------------

void TExchangeConnect::Initialize()
{
  try
	{
	  if (ReadConfig(FCfgPath) == 0)
		{
		  ParsingParamsForVars();

		  if (CreateServerCfgDirs() == 0)
			{
			  SetFtpLoader();
			  Init = true;
			}
		  else
			Init = false;
		}
	  else
		Init = false;
	}
  catch(Exception &e)
	{
	  Init = false;
	}
}
//---------------------------------------------------------------------------

void TExchangeConnect::Stop()
{
  Running = false;

  if (FtpLoader->Connected())
	{
	  try
		 {
           FtpLoader->Abort();
		   FtpLoader->DisconnectNotifyPeer();
		 }
	  catch(Exception &e)
		 {
		   WriteLog("Зупинка з'єднання: " + e.ToString());
		 }
	}

  Status = "Зупинено";
  WriteLog(Status);
}
//---------------------------------------------------------------------------

void TExchangeConnect::Start()
{
  if (Init)
	{
	  Running = true;
	  Status = "В роботі";
	  WriteLog(Status);
    }
  else
	{
	  Running = false;
	  Status = "Помилка ініціалізації";
	  WriteLog(Status);
    }
}
//---------------------------------------------------------------------------

int TExchangeConnect::Exchange()
{
  if (!Running)
	return 0;

  int try_cnt = 0, result = 0;
  bool dl_res = false, ul_res = false;

  if (FConfig.BackUpDl || FConfig.BackUpUl)
  	DeleteOldBackUpDirs();

  if (!ConnectToFTP())
	{
	  Status = "Сервер недоступний";
	  WriteLog(Status);

	  return -1;
	}

  if (FConfig.EnableDownload)
	{
	  std::unique_ptr<TStringList> dl_dirs(new TStringList());
	  //auto dl_dirs = std::make_unique<TStringList>();

	  try
		 {
		   SuccessFiles->Clear();
		   ExchageExitCode dl;

		   if (FConfig.RemDirDl == "")
			 {
			   dl = DownLoad("",
							 FConfig.DownloadFilesMask,
							 FConfig.LocDirDl,
							 FConfig.BackUpDirDl);

			   if ((dl == EE_NO_FILES) || (dl == EE_ERROR))
				 dl_res = false;
			   else if (dl_res && ((dl == EE_NO_FILES) || (dl == EE_ERROR)))
				 dl_res = true;
			   else
				 dl_res = true;

			   if (!FConfig.LeaveRemoteFiles)
				 DeleteFilesFromServer(SuccessFiles);
			 }
		   else
			 {
			   StrToList(dl_dirs.get(), FConfig.RemDirDl, ";");

			   for (int i = 0; i < dl_dirs->Count; i++)
				  {
					int lc = SubDirLevelCount(dl_dirs->Strings[i]);

					FtpLoader->ChangeDir(dl_dirs->Strings[i]);

					dl = DownLoad(dl_dirs->Strings[i],
								  FConfig.DownloadFilesMask,
								  FConfig.LocDirDl,
								  FConfig.BackUpDirDl);

					if ((dl == EE_NO_FILES) || (dl == EE_ERROR))
					  dl_res = false;
					else if (dl_res && ((dl == EE_NO_FILES) || (dl == EE_ERROR)))
					  dl_res = true;
					else
					  dl_res = true;

					ReturnToRoot(lc);
				  }

			   if (!FConfig.LeaveRemoteFiles)
				 DeleteFilesFromServer(SuccessFiles);
			 }
		 }
	  catch (Exception &e)
		 {
		   WriteLog("Обмін з сервером: " + e.ToString());
		   dl_res = false;
		 }
	}

  if (FConfig.EnableUpload)
	{
      std::unique_ptr<TStringList> ul_dirs(new TStringList());
	  //auto ul_dirs = std::make_unique<TStringList>();

	  try
		 {
		   SuccessFiles->Clear();
		   ExchageExitCode ul;

		   if (FConfig.RemDirUl == "")
			 {
			   ul = UpLoad(FConfig.LocDirUl,
						   FConfig.UploadFilesMask,
						   "",
						   FConfig.BackUpDirUl);

			   if ((ul == EE_NO_FILES) || (ul == EE_ERROR))
				 ul_res = false;
			   else if (ul_res && ((ul == EE_NO_FILES) || (ul == EE_ERROR)))
				 ul_res = true;
			   else
				 ul_res = true;
			 }
		   else
			 {
			   StrToList(ul_dirs.get(), FConfig.RemDirUl, ";");

			   for (int i = 0; i < ul_dirs->Count; i++)
				  {
					int lc = SubDirLevelCount(ul_dirs->Strings[i]);

					FtpLoader->ChangeDir(ul_dirs->Strings[i]);

					ul = UpLoad(FConfig.LocDirUl,
								FConfig.UploadFilesMask,
								ul_dirs->Strings[i],
								FConfig.BackUpDirUl);

					if ((ul == EE_NO_FILES) || (ul == EE_ERROR))
					  ul_res = false;
					else if (ul_res && ((ul == EE_NO_FILES) || (ul == EE_ERROR)))
					  ul_res = true;
					else
					  ul_res = true;

					ReturnToRoot(lc);
				  }
			 }

		   if (!FConfig.LeaveLocalFiles)
			 DeleteFiles(SuccessFiles);
		 }
	  catch (Exception &e)
		 {
		   WriteLog("Обмін з сервером: " + e.ToString());
		 }
	}

  if (dl_res && ul_res)
	result = 3;
  else if (dl_res)
	result = 1;
  else if (ul_res)
	result = 2;
  else if (!dl_res || !ul_res)
	result = -1;

  if (FtpLoader->Connected())
	{
	  try
		 {
		   FtpLoader->Disconnect();
		   WriteLog("Відключення від серверу: " + FConfig.Host + ":" + FConfig.Port);
		 }
	  catch (Exception &e)
		 {
		   WriteLog("Відключення від серверу: " + e.ToString());
		   result = -1;
		 }
	}

  return result;
}
//-------------------------------------------------------------------------

void TExchangeConnect::DeleteFiles(TStringList *files)
{
  for (int i = 0; i < files->Count; i++)
	 {
	   if (FileExists(files->Strings[i]))
		 {
		   DeleteFile(files->Strings[i]);
		   WriteLog("Видалено файл: " + files->Strings[i]);
		 }
	 }
}
//---------------------------------------------------------------------------

void TExchangeConnect::DeleteFilesFromServer(TStringList *files)
{
  String dir, file;

  for (int i = 0; i < files->Count; i++)
	 {
	   try
		  {
			dir = ExtractDirNameFromPath(files->Strings[i]);
			file = ExtractFileNameFromPath(files->Strings[i]);
			int lc = SubDirLevelCount(dir);

			FtpLoader->ChangeDir(dir);

			if (FtpLoader->Size(file) > -1)
			  {
				FtpLoader->Delete(file);
				WriteLog("Видалено файл: " +
						 FtpLoader->Host + ":/" + dir + "/" + file);
			  }

			ReturnToRoot(lc);
		  }
	   catch (Exception &e)
		  {
			WriteLog("Видалення файлу: " +
					 FtpLoader->Host + ":/" + dir + "/" + file);
		  }
	 }
}
//---------------------------------------------------------------------------

int TExchangeConnect::BackUpFiles(TStringList *files, String destin)
{
  String dir_at_time, dir_at_date, file_name;
  int res = 0;
  TFormatSettings tf;
  String bckp_dir;

  dir_at_date = DateToStr(Date());
  GetLocaleFormatSettings(LANG_UKRAINIAN, tf);
  tf.TimeSeparator = '_';
  dir_at_time = TimeToStr(Time(), tf);
  bckp_dir = destin + "\\" +
			 dir_at_date + "\\" +
             IntToStr(this->ID) + "_" +
			 FConfig.Caption + "\\" +
			 dir_at_time;

  if (!DirectoryExists(bckp_dir))
	ForceDirectories(bckp_dir);

  for (int i = 0; i < files->Count; i++)
	 {
	   file_name = files->Strings[i];
	   file_name.Delete(1, file_name.LastDelimiter("\\"));

	   if (!FileExists(bckp_dir + "\\" + file_name))
		 {
		   if (CopyFile(files->Strings[i].c_str(),
						String(bckp_dir + "\\" + file_name).c_str(), 1) == 0)
			 {
			   WriteLog("Помилка бекапу: " +
						files->Strings[i] + " до " + bckp_dir + "\\" + file_name);
		   	   res = -1;
			 }
		 }
	 }

  return res;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFile(String source, String destin, int list_index)
{
  std::unique_ptr<TMemoryStream> ms(new TMemoryStream());
  //auto ms = std::make_unique<TMemoryStream>();
  int res;

  String src_name = source;
  int pos = src_name.LastDelimiter("/");
  src_name.Delete(1, pos);

  try
	 {
	   FtpLoader->Get(src_name, ms.get());
	   ms->SaveToFile(destin);
//змінюємо дату та час файла на ті, що були у оригінала на сервері
	   OFSTRUCT of;
	   FILETIME ft;
	   SYSTEMTIME st;
	   AnsiString name = destin;
	   HANDLE hFile = (HANDLE)OpenFile(name.c_str(), &of, OF_READWRITE);

	   if (hFile)
		 {
		   FtpLoader->DirectoryListing->Items[list_index]->ModifiedDate.DecodeDate(&st.wYear,
																				   &st.wMonth,
																				   &st.wDay);
		   FtpLoader->DirectoryListing->Items[list_index]->ModifiedDate.DecodeTime(&st.wHour,
																				   &st.wMinute,
																				   &st.wSecond,
																				   &st.wMilliseconds);
		   TzSpecificLocalTimeToSystemTime(NULL, &st, &st);
		   SystemTimeToFileTime(&st, &ft);
		   SetFileTime(hFile, &ft, &ft, &ft);
		 }
	   else
		 {
		   WriteLog("Не вдалось змінити дату/час файлу: " + destin);
		   res = -1;
		 }

	   CloseHandle(hFile);
	   WriteLog("Завантажено файл: " +
				FtpLoader->Host + ":/" + source +
				" до: " + destin);
	   res = 1;
	 }
  catch (Exception &e)
	 {
	   WriteLog("GetFTPFile: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int TExchangeConnect::LoadFilesFromServer(String source,
										  String mask,
										  String destin,
										  String backup)
{
  if (!FtpLoader->Connected())
	return 0;

  int result = 0;
  std::unique_ptr<TStringList> files(new TStringList());
  //auto files = std::make_unique<TStringList>();

  try
	 {
	   if (FConfig.RegExDL)
		 GetFTPFileListRegEx(files.get(), mask);
	   else
		 GetFTPFileList(files.get(), mask);
     }
   catch (Exception &e)
	 {
	   WriteLog("Помилка читання переліку файлів, " + source + "/" + mask + " :" + e.ToString());
	   result = 0;
	 }

  std::unique_ptr<TStringList> ok_files(new TStringList()); //перелік успішно завантажених файлів
  //auto ok_files = std::make_unique<TStringList>();

  String src_name, remote_file;

  for (int i = 0; i < files->Count; i++)
	 {
	   src_name = files->Strings[i];

	   if (destin == "")
		 remote_file = src_name;
	   else
		 remote_file = source + "/" + src_name;

	   try
		  {
			bool downloaded = false;

			if (FileExists(destin + "\\" + src_name) && SupportsVerification)
			  {
				bool equal = VerifyFile(src_name, destin + "\\" + src_name);

				if (!equal)
				  {
					if (GetFTPFile(remote_file, destin + "\\" + src_name, i))
					  downloaded = true;
				  }
			  }
			else if (FConfig.AppendModeDL)
			  {
				if (!FileExists(destin + "\\" + src_name))
				  {
					if (GetFTPFile(remote_file, destin + "\\" + src_name, i))
					  downloaded = true;
				  }
			  }
			else
			  {
				if (GetFTPFile(remote_file, destin + "\\" + src_name, i))
				  downloaded = true;
			  }

			if (downloaded)
			  {
				ok_files->Add(destin + "\\" + src_name);
				AddSuccessFile(remote_file);
			  }

			result = 1;
		  }
	   catch (Exception &e)
		  {
			WriteLog("Помилка завантаження: " +
					 FtpLoader->Host + ":/" + remote_file +
					 " до " + destin + "\\" + src_name +
					 " (" + e.ToString() + ")");

			result = 0;
		  }
	 }

  if (result)
	{
	  if (ok_files->Count > 0)
		result = 1;
	  else
		result = 2;
	}

  if (FConfig.BackUpDl && (backup != ""))
	BackUpFiles(ok_files.get(), backup);

  return result;
}
//---------------------------------------------------------------------------

int TExchangeConnect::LoadFilesFromServerSubDirs(String source,
												 String mask,
												 String destin,
												 String backup)
{
  if (!FtpLoader->Connected())
	return 0;

  std::unique_ptr<TStringList> DirList(new TStringList());
  //auto DirList = std::make_unique<TStringList>();
  int result = 0;

  if (GetFullDirList(DirList.get(), "") < 0)
	return 0;

  result = LoadFilesFromServer(source, mask, destin, backup);

  for (int i = 0; i < DirList->Count; i++)
	 {
	   FtpLoader->ChangeDir(DirList->Strings[i]);

	   if (FConfig.SubDirsCrt)
		 {
		   if (!DirectoryExists(destin + "\\" + DirList->Strings[i]))
			 CreateDirectory(String(destin + "\\" + DirList->Strings[i]).c_str(), NULL);

		   result += LoadFilesFromServer(source + "/" + DirList->Strings[i],
										 mask,
										 destin + "\\" + DirList->Strings[i],
										 backup);
		 }
	   else
		 {
		   result += LoadFilesFromServer(source + "/" + DirList->Strings[i],
										 mask,
										 destin,
										 backup);
		 }

	   FtpLoader->ChangeDirUp();
	 }

  return result;
}
//---------------------------------------------------------------------------

bool TExchangeConnect::IsFileLocked(const String &file)
{
  bool locked;

  try
	 {
	   int file_handle;

	   try
		  {
			file_handle = FileOpen(file, fmOpenRead);

			if (file_handle > -1)
			  locked = false;
			else
              locked = true;
		  }
	   __finally
		  {
			FileClose(file_handle);
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Перевірка блокування файлу " + file + " : " + e.ToString());
	   locked = true;
	 }

  return locked;
}
//---------------------------------------------------------------------------

void TExchangeConnect::AddSuccessFile(String file)
{
  try
	 {
	   if (SuccessFiles->IndexOf(file) == -1)
		 SuccessFiles->Add(file);
	 }
  catch (Exception &e)
	 {
	   WriteLog("AddSuccessFile: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int TExchangeConnect::SubDirLevelCount(const String &source)
{
  int levels;
  String operstr = source;

  if (operstr != "")
	levels = 1;

  try
	 {
	   int pos;

	   while (pos = operstr.Pos("/"))
		 {
		   levels++;
		   operstr.Delete(1, pos);
		 }
	 }
  catch (Exception &e)
	 {
	   WriteLog("SubDirLevelCount: " + e.ToString());
	   levels = -1;
	 }

  return levels;
}
//---------------------------------------------------------------------------

void TExchangeConnect::ReturnToRoot(int level)
{
  try
	 {
	   for (int i = level; i >= 1; i--)
		  {
            FtpLoader->ChangeDirUp();
          }
	 }
  catch (Exception &e)
	 {
	   WriteLog("ReturnToRoot: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int TExchangeConnect::SendFilesToServer(String source,
										String mask,
										String destin,
										String backup)
{
  if (!FtpLoader->Connected())
	return 0;

  std::unique_ptr<TStringList> files(new TStringList());
  std::unique_ptr<TStringList> ok_files(new TStringList());
  //auto files = std::make_unique<TStringList>();
  //auto ok_files = std::make_unique<TStringList>();

  try
	 {
	   if (FConfig.RegExUL)
		 GetFileListRegEx(files.get(), source, mask, false, true);
	   else
		 GetFileList(files.get(), source, mask, false, true);
     }
  catch (Exception &e)
	 {
	   WriteLog("Запит переліку файлів з " + source + "\\" + mask + " : " + e.ToString());
	 }

  String file_name, remote_file;
  int result = 0;

  for (int i = 0; i < files->Count; i++)
	 {
	   try
		  {
			file_name = files->Strings[i];
			file_name.Delete(1, file_name.LastDelimiter("\\"));

			if (destin == "")
			  remote_file = file_name;
			else
			  remote_file = destin + "/" + file_name;

			bool uploaded = false;

			if (IsFileLocked(files->Strings[i]))
			  {
				WriteLog("Заблоковано файл " + files->Strings[i] + " Вивантаження неможливе");
			  }
			else if (FConfig.AppendModeUL)
			  {
				if (!IsFtpFileExist(destin, file_name))
				  {
					FtpLoader->Put(files->Strings[i], file_name, false);
					uploaded = true;
				  }
			  }
			else
			  {
				FtpLoader->Put(files->Strings[i], file_name, false);
				uploaded = true;
			  }

			if (uploaded)
			  {
				WriteLog(files->Strings[i] +
						 " вивантажено до: " + FtpLoader->Host + ":/" + remote_file);

				ok_files->Add(files->Strings[i]);
				AddSuccessFile(files->Strings[i]);
			  }

			result = 1;
		  }
	   catch (Exception &e)
		  {
			WriteLog("Помилка вивантаження: " + files->Strings[i] +
					 " до: " + FtpLoader->Host + ":/" + remote_file +
					 " (" + e.ToString() + ")");

			result = 0;
		  }
	 }

  if (result)
	{
	  if (ok_files->Count > 0)
		result = 1;
	  else
		result = 2;
	}

  if (FConfig.BackUpUl && (backup != ""))
	BackUpFiles(ok_files.get(), backup);

  return result;
}
//---------------------------------------------------------------------------

int TExchangeConnect::SendFilesToServerSubDirs(String source,
											   String mask,
											   String destin,
											   String backup)
{
  if (!FtpLoader->Connected())
	return 0;

  std::unique_ptr<TStringList> DirList(new TStringList());
  //auto DirList = std::make_unique<TStringList>();
  int result = 0;
  String remote_dir;

  result = SendFilesToServer(source, mask, destin, backup);

  GetDirList(DirList.get(), source, WITHOUT_FULL_PATH);

  for (int i = 0; i < DirList->Count; i++)
	 {
	   if (FConfig.SubDirsCrt)
		 {
		   if (destin == "")
			 remote_dir = DirList->Strings[i];
		   else
			 remote_dir = destin + "/" + DirList->Strings[i];

		   if (!IsFtpDirExist(destin, DirList->Strings[i]))
			 FtpLoader->MakeDir(DirList->Strings[i]);

		   FtpLoader->ChangeDir(DirList->Strings[i]);

		   result += SendFilesToServer(source + "\\" + DirList->Strings[i],
									   mask,
									   remote_dir,
									   backup);

		   FtpLoader->ChangeDirUp();
		 }
	   else
		 {
		   result += SendFilesToServer(source + "\\" + DirList->Strings[i],
									   mask,
									   destin,
									   backup);
		 }
	 }

  return result;
}
//---------------------------------------------------------------------------

bool TExchangeConnect::IsFtpDirExist(String source, String dir_name)
{
  std::unique_ptr<TStringList> DirList(new TStringList());
  //auto DirList = std::make_unique<TStringList>();
  bool result = false;

  if (GetFullDirList(DirList.get(), "") < 0)
	return false;

  for (int i = 0; i < DirList->Count; i++)
	 {
	   if (DirList->Strings[i] == dir_name)
		 {
		   result = true;
		   break;
		 }
	 }

  return result;
}
//---------------------------------------------------------------------------

String TExchangeConnect::ExtractDirNameFromPath(const String &filepath)
{
  String res;

  try
	 {
	   res = filepath;
	   int pos = res.LastDelimiter("/");
	   res.Delete(pos, res.Length() - (pos - 1));
	 }
   catch (Exception &e)
	 {
	   WriteLog("ExtractDirNameFromPath :" + e.ToString());
	   res = "";
	 }

   return res;
}
//---------------------------------------------------------------------------

String TExchangeConnect::ExtractFileNameFromPath(const String &filepath)
{
  String res;

  try
	 {
	   res = filepath;
	   int pos = res.LastDelimiter("/");
	   res.Delete(1, pos);
	 }
   catch (Exception &e)
	 {
	   WriteLog("ExtractFileNameFromPath :" + e.ToString());
	   res = "";
	 }

   return res;
}
//---------------------------------------------------------------------------

bool TExchangeConnect::IsFtpFileExist(String source, String file_name)
{
  bool result = false;

  if (GetFTPFileCount(source, file_name) > 0)
    result = true;

  return result;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFileCount(String source, String mask)
{
  if (!FtpLoader->Connected())
	return -1;

  try
	 {
	   FtpLoader->List(mask, true);
	 }
   catch (Exception &e)
	 {
	   WriteLog("Помилка читання переліку файлів, " + source + "/" + mask + " :" + e.ToString());

	   return 0;
	 }

  String src_name;
  int FileCount = 0;

  for (int i = 0; i < FtpLoader->DirectoryListing->Count; i++)
	 {
	   src_name = FtpLoader->DirectoryListing->Items[i]->FileName;

	   if (src_name == "." || src_name == "..")
		 {
		   continue;
		 }
	   else if (FtpLoader->DirectoryListing->Items[i]->ItemType != ditDirectory)
		 FileCount++;
	 }

  return FileCount;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFileCountRegEx(String source, String reg_exp)
{
  if (!FtpLoader->Connected())
	return -1;

  try
	 {
	   FtpLoader->List("", true);
	 }
   catch (Exception &e)
	 {
	   WriteLog("Помилка читання переліку файлів, " + source + "/*" + " :" + e.ToString());

	   return 0;
	 }

  String src_name;
  int FileCount = 0;

  for (int i = 0; i < FtpLoader->DirectoryListing->Count; i++)
	 {
	   src_name = FtpLoader->DirectoryListing->Items[i]->FileName;

	   if (src_name == "." || src_name == "..")
		 {
		   continue;
		 }
	   else if ((FtpLoader->DirectoryListing->Items[i]->ItemType != ditDirectory) &&
				(TRegEx::IsMatch(src_name, reg_exp)))
		 {
		   FileCount++;
		 }
	 }

  return FileCount;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFileCountWithSubDirs(String source, String mask)
{
  if (!FtpLoader->Connected())
	return -1;

  std::unique_ptr<TStringList> DirList(new TStringList());
  //auto DirList = std::make_unique<TStringList>();
  String src_name;
  int FileCount = 0;


  try
	 {
	   FtpLoader->List("", true);
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка читання переліку директорій, " + source + "/" + mask + ": " + e.ToString());

	   return 0;
	 }

  try
	 {
	   for (int i = 0; i < FtpLoader->DirectoryListing->Count; i++)
		  {
			src_name = FtpLoader->DirectoryListing->Items[i]->FileName;

			if (src_name == "." || src_name == "..")
			  {
				continue;
			  }
			else if (FtpLoader->DirectoryListing->Items[i]->ItemType == ditDirectory)
			  {
				DirList->Add(src_name);
			  }
		  }

	   FileCount = GetFTPFileCount(source, mask);

	   for (int i = 0; i < DirList->Count; i++)
		  {
			FtpLoader->ChangeDir(DirList->Strings[i]);
			FileCount += GetFTPFileCount(source + "/" + DirList->Strings[i], mask);
			FtpLoader->ChangeDirUp();
		  }
	 }
  catch (Exception &e)
	{
	  WriteLog("GetFTPFileCountWithSubDirs: " + e.ToString());

	  return 0;
	}

  return FileCount;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFileCountWithSubDirsRegEx(String source, String reg_exp)
{
  if (!FtpLoader->Connected())
	return -1;

  std::unique_ptr<TStringList> DirList(new TStringList());
  //auto DirList = std::make_unique<TStringList>();
  String src_name;
  int FileCount = 0;

  try
	 {
	   FtpLoader->List("", true);
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка читання переліку директорій, " + source + "/*"+ " :" + e.ToString());

	   return 0;
	 }

  try
	 {
	   for (int i = 0; i < FtpLoader->DirectoryListing->Count; i++)
		  {
			src_name = FtpLoader->DirectoryListing->Items[i]->FileName;

			if (src_name == "." || src_name == "..")
			  {
				continue;
			  }
			else if (FtpLoader->DirectoryListing->Items[i]->ItemType == ditDirectory)
			  {
				DirList->Add(src_name);
			  }
		  }

	   FileCount = GetFTPFileCountRegEx(source, reg_exp);

	   for (int i = 0; i < DirList->Count; i++)
		  {
			FtpLoader->ChangeDir(DirList->Strings[i]);
			FileCount += GetFTPFileCountRegEx(source + "/" + DirList->Strings[i], reg_exp);
			FtpLoader->ChangeDirUp();
		  }
	 }
  catch (Exception &e)
	{
	  WriteLog("GetFTPFileCountWithSubDirsRegEx: " + e.ToString());

	  return 0;
	}

  return FileCount;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFileList(TStringList *list, String mask)
{
  int file_cnt = 0;
  String src_name;

  if (!list)
	return -1;

  try
	 {
	   FtpLoader->List(mask, true);

       for (int i = 0; i < FtpLoader->DirectoryListing->Count; i++)
		  {
			src_name = FtpLoader->DirectoryListing->Items[i]->FileName;

			if (src_name == "." || src_name == "..")
			  {
				continue;
			  }
			else if (FtpLoader->DirectoryListing->Items[i]->ItemType != ditDirectory)
			  {
				list->Add(src_name);
				file_cnt++;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка отримання списку файлів: " + e.ToString());
	 }

  return file_cnt;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFileListRegEx(TStringList *list, String reg_exp)
{
  int file_cnt = 0;
  String src_name;

  if (!list)
	return -1;

  try
	 {
	   FtpLoader->List("", true);

	   for (int i = 0; i < FtpLoader->DirectoryListing->Count; i++)
		  {
			src_name = FtpLoader->DirectoryListing->Items[i]->FileName;

			if (src_name == "." || src_name == "..")
			  {
				continue;
			  }
			else if ((FtpLoader->DirectoryListing->Items[i]->ItemType != ditDirectory) &&
					 (TRegEx::IsMatch(src_name, reg_exp)))
			  {
				list->Add(src_name);
				file_cnt++;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка отримання списку директорій: " + e.ToString());
	 }

  return file_cnt;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFullDirList(TStringList *list, String mask)
{
  int dir_cnt = 0;
  String src_name;

  if (!list)
	return -1;

  try
	 {
	   FtpLoader->List(mask, true);

	   for (int i = 0; i < FtpLoader->DirectoryListing->Count; i++)
		  {
			src_name = FtpLoader->DirectoryListing->Items[i]->FileName;

			if (src_name == "." || src_name == "..")
			  {
				continue;
			  }
			else if (FtpLoader->DirectoryListing->Items[i]->ItemType == ditDirectory)
			  {
				list->Add(src_name);
				dir_cnt++;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Помилка отримання списку директорій: " + e.ToString());
	 }

  return dir_cnt;
}
//---------------------------------------------------------------------------

bool TExchangeConnect::ConnectToFTP()
{
  if (FtpLoader->Connected())
	return true;

  try
	{
	  FtpLoader->Connect();
	}
  catch (Exception &e)
	{
	  WriteLog("FTP помилка: " + e.ToString());
	}

  return FtpLoader->Connected();
}
//---------------------------------------------------------------------------

ExchageExitCode TExchangeConnect::DownLoad(String source,
										   String mask,
										   String destin,
										   String backup)
{
  int fcnt;
  ExchageExitCode result;
  std::unique_ptr<TStringList> d_mask_lst(new TStringList());
  //auto d_mask_lst = std::make_unique<TStringList>();
  StrToList(d_mask_lst.get(), mask, ";");

  Status = "Завантаження...";
  WriteLog(Status);

  for (int i = 0; i < d_mask_lst->Count; i++)
	 {
	   if (FConfig.SubDirsDl)
		 {
		   if (FConfig.RegExDL)
			 fcnt = GetFTPFileCountWithSubDirsRegEx(source, d_mask_lst->Strings[i]);
		   else
			 fcnt = GetFTPFileCountWithSubDirs(source, d_mask_lst->Strings[i]);
		 }
	   else
		 {
		   if (FConfig.RegExDL)
			 fcnt = GetFTPFileCountRegEx(source, d_mask_lst->Strings[i]);
		   else
			 fcnt = GetFTPFileCount(source, d_mask_lst->Strings[i]);
         }

	   if (fcnt > 0)
		 {
		   int res;

		   if (FConfig.SubDirsDl)
			 {
			   res = LoadFilesFromServerSubDirs(source, d_mask_lst->Strings[i], destin, backup);

			   if (res > 0)
				  {
					Status = "Завантажені нові файли за маскою " + d_mask_lst->Strings[i];
					WriteLog(Status);
					result = EE_ALL_FILES;
				  }
			 }
		   else
			 {
			   res = LoadFilesFromServer(source, d_mask_lst->Strings[i], destin, backup);

			   if (res == 1)
				 {
				   Status = "Завантажені нові файли за маскою " + d_mask_lst->Strings[i];
				   WriteLog(Status);
				   result = EE_ALL_FILES;
				 }
			   else if (res == 2)
				 {
                   Status = "Файли за маскою " + d_mask_lst->Strings[i] + " не потребують завантаження";
				   WriteLog(Status);

				   if ((result == EE_ALL_FILES) || (result == EE_SOME_FILES))
					 result = EE_SOME_FILES;
				   else
				   	 result = EE_NO_FILES;
				 }
			   else
				 {
                   Status = "Під час завантаження файлів за маскою " + d_mask_lst->Strings[i] + " виникли помилки";
				   WriteLog(Status);

				   if ((result == EE_ALL_FILES) || (result == EE_SOME_FILES))
					 result = EE_SOME_FILES_WITH_ERR;
				   else
				 	 result = EE_ERROR;
				 }
			 }
		 }
	   else
		 {
		   Status = "Відсутні файли за маскою " + d_mask_lst->Strings[i] + " для завантаження";
		   WriteLog(Status);

		   if ((result == EE_ALL_FILES) || (result == EE_SOME_FILES))
			 result = EE_SOME_FILES;
		   else
			 result = EE_NO_FILES;
		 }
	}

  return result;
}
//-------------------------------------------------------------------------

ExchageExitCode TExchangeConnect::UpLoad(String source,
										 String mask,
										 String destin,
										 String backup)
{
  Status = "Вивантаження...";
  WriteLog(Status);

  std::unique_ptr<TStringList> u_mask_lst(new TStringList());
  //auto u_mask_lst = std::make_unique<TStringList>();

  StrToList(u_mask_lst.get(), mask, ";");

  int cnt;
  ExchageExitCode result = EE_NO_FILES;

  for (int i = 0; i < u_mask_lst->Count; i++)
	 {
	   if (FConfig.SubDirsDl)
		 {
		   try
			  {
				if (FConfig.RegExUL)
				  cnt = GetFileCountSubDirsRegEx(source, u_mask_lst->Strings[i]);
				else
                  cnt = GetFileCountSubDirs(source, u_mask_lst->Strings[i]);
			  }
		   catch (Exception &e)
			  {
				Status = "Помилка запиту файлів з " + source + "\\" + u_mask_lst->Strings[i];
				WriteLog(Status);
				result = EE_ERROR;
			  }
		 }
	   else
		 {
           try
			  {
				if (FConfig.RegExUL)
				  cnt = GetFileCountRegEx(source, u_mask_lst->Strings[i]);
				else
				  cnt = GetFileCount(source, u_mask_lst->Strings[i]);
			  }
		   catch (Exception &e)
			  {
                Status = "Помилка запиту файлів з " + source + "\\" + u_mask_lst->Strings[i];
				WriteLog(Status);
				result = EE_ERROR;
			  }
		 }

	   if (cnt > 0)
		 {
		   int res;

		   if (FConfig.SubDirsDl)
			 {
			   if (SendFilesToServerSubDirs(source, u_mask_lst->Strings[i], destin, backup) > 0)
				 {
				   Status = "Вивантажені файли за маскою " + u_mask_lst->Strings[i];
				   WriteLog(Status);
				   result = EE_ALL_FILES;
				 }
			 }
		   else
			 {
			   res = SendFilesToServer(source, u_mask_lst->Strings[i], destin, backup);

			   if (res == 1)
				 {
				   Status = "Вивантажені файли за маскою " + u_mask_lst->Strings[i];
				   WriteLog(Status);
				   result = EE_ALL_FILES;
				 }
			   else if (res == 2)
				 {
				   Status = "Файли за маскою " + u_mask_lst->Strings[i] + " не потребують вивантаження";
				   WriteLog(Status);

				   if ((result == EE_ALL_FILES) || (result == EE_SOME_FILES))
					 result = EE_SOME_FILES;
				   else
					 result = EE_NO_FILES;
				 }
			   else
				 {
				   Status = "Під час вивантаження файлів за маскою " + u_mask_lst->Strings[i] + " виникли помилки";
				   WriteLog(Status);

				   if (result == EE_ALL_FILES)
					 result = EE_SOME_FILES_WITH_ERR;
				   else
					 result = EE_ERROR;
				 }
			 }
		 }
	   else
		 {
		   Status = "Відсутні файли за маскою " + u_mask_lst->Strings[i] + " для вивантаження";
		   WriteLog(Status);

		   if ((result == EE_ALL_FILES) || (result == EE_SOME_FILES))
			 result = EE_SOME_FILES;
		   else
			 result = EE_NO_FILES;
		 }
	 }

  return result;
}
//-------------------------------------------------------------------------

void TExchangeConnect::WriteLog(String text)
{
  text = "{id: " + IntToStr(this->ID) + "} " + this->Caption + ": " + text;

  Log->Add(text);
}
//---------------------------------------------------------------------------

String TExchangeConnect::ParsingVariables(String str, std::vector<VARPAIR> *vars)
{
  for (unsigned int i = 0; i < vars->size(); i++)
	 {
	   if (str.Pos(vars->at(i).Var) > 0)
		 str = ParseString(str, vars->at(i).Var, vars->at(i).Val);
	 }

  return str;
}
//---------------------------------------------------------------------------

void TExchangeConnect::ParsingParamsForVars()
{
  std::vector<VARPAIR> vars;
  VARPAIR prm;

  prm.Var = "$IndexVZ";
  prm.Val = IndexVZ;
  vars.push_back(prm);

  prm.Var = "$StationID";
  prm.Val = StationID;
  vars.push_back(prm);

  prm.Var = "$Date";
  prm.Val = DateToStr(Date());
  vars.push_back(prm);

  prm.Var = "$AppPath";
  prm.Val = AppPath;
  vars.push_back(prm);

  prm.Var = "$DataPath";
  prm.Val = DataPath;
  vars.push_back(prm);

  FConfig.Caption = ParsingVariables(FConfig.Caption, &vars);
  FConfig.Host = ParsingVariables(FConfig.Host, &vars);
  FConfig.User = ParsingVariables(FConfig.User, &vars);
  FConfig.Pwd = ParsingVariables(FConfig.Pwd, &vars);
  FConfig.RemDirDl = ParsingVariables(FConfig.RemDirDl, &vars);
  FConfig.LocDirDl = ParsingVariables(FConfig.LocDirDl, &vars);
  FConfig.BackUpDirDl = ParsingVariables(FConfig.BackUpDirDl, &vars);
  FConfig.DownloadFilesMask = ParsingVariables(FConfig.DownloadFilesMask, &vars);
  FConfig.RemDirUl = ParsingVariables(FConfig.RemDirUl, &vars);
  FConfig.LocDirUl = ParsingVariables(FConfig.LocDirUl, &vars);
  FConfig.BackUpDirUl = ParsingVariables(FConfig.BackUpDirUl, &vars);
  FConfig.UploadFilesMask = ParsingVariables(FConfig.UploadFilesMask, &vars);
}
//---------------------------------------------------------------------------

void TExchangeConnect::DeleteOldBackUpDirs()
{
  std::unique_ptr<TStringList> dir_list(new TStringList());
  std::unique_ptr<TStringList> del_list(new TStringList());
  //auto dir_list = std::make_unique<TStringList>();
  //auto del_list = std::make_unique<TStringList>();

  try
	 {
	   if (DirectoryExists(FConfig.BackUpDirDl))
		 {
		   GetDirList(dir_list.get(), FConfig.BackUpDirDl, WITHOUT_FULL_PATH);

		   for (int i = 0; i < dir_list->Count; i++)
			  {
				TDate dir_date = TDate(dir_list->Strings[i]);

				if (dir_date < (Date().CurrentDate() - FConfig.BackUpKeepDays))
				  del_list->Add(dir_list->Strings[i]);
			  }
         }

	   dir_list->Clear();

	   for (int i = 0; i < del_list->Count; i++)
		  {
			DeleteAllFromDir(FConfig.BackUpDirDl + "\\" + del_list->Strings[i]);
			RemoveDir(FConfig.BackUpDirDl + "\\" + del_list->Strings[i]);
		  }

       del_list->Clear();

	   if (DirectoryExists(FConfig.BackUpDirUl))
		 {
		   GetDirList(dir_list.get(), FConfig.BackUpDirUl, WITHOUT_FULL_PATH);

		   TDate dir_date;

		   for (int i = 0; i < dir_list->Count; i++)
			  {
				try
				   {
					 dir_date = TDate(dir_list->Strings[i]);

					 if (dir_date < (Date().CurrentDate() - FConfig.BackUpKeepDays))
					   del_list->Add(dir_list->Strings[i]);
				   }
				catch (Exception &e)
				   {
					 WriteLog("Видалення каталогів старих бекапів: " + e.ToString());
				   }
			  }
		 }

       for (int i = 0; i < del_list->Count; i++)
		  {
			DeleteAllFromDir(FConfig.BackUpDirUl + "\\" + del_list->Strings[i]);
			RemoveDir(FConfig.BackUpDirUl + "\\" + del_list->Strings[i]);
		  }
	 }
  catch (Exception &e)
	 {
	   WriteLog("Видалення каталогів старих бекапів: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

bool TExchangeConnect::GetVerification()
{
  bool res;

  try
	 {
	   if (FtpLoader->Capabilities->IndexOf("XCRC") >= 0)
		 res = true;
	   else
		 res = false;
	 }
  catch (Exception &e)
	 {
	   res = false;

	   WriteLog("GetVerification: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

bool TExchangeConnect::VerifyFile(const String &remote_file, const String &local_file)
{
  bool res;

  try
	 {
	   long remote_crc = FtpLoader->CRC(remote_file);
	   long local_crc = 0;
	   std::unique_ptr<TIdHashCRC32> local_hash(new TIdHashCRC32());
	   std::unique_ptr<TFileStream> local_stream(new TFileStream(local_file, fmOpenRead));
	   //auto local_hash = std::make_unique<TIdHashCRC32>();
	   //auto local_stream = std::make_unique<TFileStream>(local_file, fmOpenRead);

	   local_crc = local_hash->HashValue(local_stream.get());

	   if (remote_crc == local_crc)
		 res = true;
	   else
         res = false;
	 }
  catch (Exception &e)
	 {
	   res = false;

	   WriteLog("VerifyFile: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
