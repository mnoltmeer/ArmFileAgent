/*!
Copyright 2019-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "TExchangeConnect.h"

TExchangeConnect::TExchangeConnect(int ID, TTrayIcon *Icon, TThreadSafeLog *Log)
{
  this->Log = Log;
  this->ID = ID;
  this->Running = false;
  this->thID = 0;

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

TExchangeConnect::TExchangeConnect(String cfg_file,
								   int ID,
								   TTrayIcon *Icon,
								   TThreadSafeLog *Log)
{
  this->Log = Log;
  this->ID = ID;
  this->Running = false;
  this->thID = 0;
  CfgPath = cfg_file;

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
  FtpLoader->Port = Config.Port;

  if (FtpLoader->Port == 0)
	FtpLoader->Port = 21;

  FtpLoader->Host = Config.Host;
  FtpLoader->Username = Config.User;
  FtpLoader->Password = Config.Pwd;
  FtpLoader->Passive = true;
  FtpLoader->AutoLogin = true;
  FtpLoader->TransferType = TIdFTPTransferType(Config.TransType);
}
//---------------------------------------------------------------------------

void TExchangeConnect::CheckConfig(String cfg_file)
{
  const int CfgPrmCnt = 31;

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
										 L"TimeStart",
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
	   if (ReadParameter(cfg_file, "Caption", &Config.Caption, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру Caption: " + String(GetLastReadParamResult()));
		   result = -1;
         }

	   if (ReadParameter(cfg_file, "FtpHost", &Config.Host, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру FtpHost: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "FtpPort", &Config.Port, TT_TO_INT) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру FtpPort: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "FtpUser", &Config.User, TT_TO_STR) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру FtpUser: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "FtpPass", &Config.Pwd, TT_TO_STR) != RP_OK)
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
			 Config.TransType = ftASCII;
		   else if (type == "BINARY")
			 Config.TransType = ftBinary;
		   else
		 	 Config.TransType = ftBinary;
         }

	   if (ReadParameter(cfg_file, "LeaveRemoteFiles", &Config.LeaveRemoteFiles, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру LeaveRemoteFiles: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "LeaveLocalFiles", &Config.LeaveLocalFiles, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру LeaveLocalFiles: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "MonitoringInterval", &Config.MonitoringInterval, TT_TO_INT) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру MonitoringInterval: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else
		 {
		   Config.MonitoringInterval = Config.MonitoringInterval * 60000;
         }

	   if (ReadParameter(cfg_file, "RunOnce", &Config.RunOnce, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру RunOnce: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "StartAtTime", &Config.StartAtTime, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру StartAtTime: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "DownloadFromSubDirs", &Config.SubDirsDl, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру DownloadFromSubDirs: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "SaveWithSubDirs", &Config.SubDirsCrt, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру SaveWithSubDirs: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "BackUpKeepDays", &Config.BackUpKeepDays, TT_TO_INT) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру BackUpKeepDays: " + String(GetLastReadParamResult()));
		   result = -1;
		 }

	   if (ReadParameter(cfg_file, "EnableDownload", &Config.EnableDownload, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру EnableDownload: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (Config.EnableDownload)
		 {
		   if (ReadParameter(cfg_file, "RemDirDl", &Config.RemDirDl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RemDirDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "LocDirDl", &Config.LocDirDl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру LocDirDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpDirDl", &Config.BackUpDirDl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpDirDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "DownloadFilesMask", &Config.DownloadFilesMask, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру DownloadFilesMask: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "RegExDL", &Config.RegExDL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RegExDL: " + String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpDl", &Config.BackUpDl, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpDl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "AppendModeDL", &Config.AppendModeDL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру AppendModeDL: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }
		 }

	   if (ReadParameter(cfg_file, "EnableUpload", &Config.EnableUpload, TT_TO_BOOL) != RP_OK)
		 {
		   WriteLog("Помилка створення параметру EnableUpload: " + String(GetLastReadParamResult()));
		   result = -1;
		 }
	   else if (Config.EnableUpload)
		 {
		   if (ReadParameter(cfg_file, "RemDirUl", &Config.RemDirUl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RemDirUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "LocDirUl", &Config.LocDirUl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру LocDirUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpDirUl", &Config.BackUpDirUl, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpDirUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "UploadFilesMask", &Config.UploadFilesMask, TT_TO_STR) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру UploadFilesMask: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "RegExUL", &Config.RegExUL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру RegExUL: " + String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "BackUpUl", &Config.BackUpUl, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру BackUpUl: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }

		   if (ReadParameter(cfg_file, "AppendModeUL", &Config.AppendModeUL, TT_TO_BOOL) != RP_OK)
			 {
			   WriteLog("Помилка створення параметру AppendModeUL: " +
						String(GetLastReadParamResult()));
			   result = -1;
			 }
         }

	   if (Config.StartAtTime)
		 {
		   short h, m;
		   TStringList *lst = new TStringList();

		   try
			  {
				String str;

				if (ReadParameter(cfg_file, "TimeStart", &str, TT_TO_STR) != RP_OK)
				  {
					WriteLog("Помилка створення параметру TimeStart: " +
							 String(GetLastReadParamResult()));
					result = -1;
				  }
				else
				  {
					StrToList(lst, str, ":");

					try
					   {
						 h = lst->Strings[0].ToInt();
						 m = lst->Strings[1].ToInt();
						 Config.TimeStart = TTime(h, m, 0, 0);
					   }
                    catch (Exception &e)
					   {
						 WriteLog("Помилка створення параметру TimeStart: " + e.ToString());
					   }
				  }
			  }
			__finally {delete lst;}
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
	  if (Config.EnableDownload && !DirectoryExists(Config.LocDirDl))
		if (!ForceDirectories(Config.LocDirDl))
		  {
			WriteLog("Не вдалося створити директорію: " + Config.LocDirDl);
		  }

	  if (Config.EnableUpload && !DirectoryExists(Config.LocDirUl))
		if (!ForceDirectories(Config.LocDirUl))
		  {
			WriteLog("Не вдалося створити директорію: " + Config.LocDirUl);
		  }

	  if (Config.BackUpDl && (Config.BackUpDirDl != ""))
		if (!DirectoryExists(Config.BackUpDirDl))
		  if (!ForceDirectories(Config.BackUpDirDl))
			{
			  WriteLog("Не вдалося створити директорію: " + Config.BackUpDirDl);
			}

	  if (Config.BackUpUl && (Config.BackUpDirUl != ""))
		if (!DirectoryExists(Config.BackUpDirUl))
		  if (!ForceDirectories(Config.BackUpDirUl))
			{
			  WriteLog("Не вдалося створити директорію: " + Config.BackUpDirUl);
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
  if (ReadConfig(CfgPath) == 0)
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

  if (Config.BackUpDl || Config.BackUpUl)
  	DeleteOldBackUpDirs();

  if (!ConnectToFTP())
	{
	  Status = "Сервер недоступний";
	  WriteLog(Status);

	  return -2;
	}

  if (Config.EnableDownload)
	{
	  TStringList *dl_dirs = new TStringList();

	  try
		 {
		   try
			  {
				SuccessFiles->Clear();
				ExchageExitCode dl;

				if (Config.RemDirDl == "")
				  {
					dl = DownLoad("",
								  Config.DownloadFilesMask,
								  Config.LocDirDl,
								  Config.BackUpDirDl);

					if ((dl == EE_NO_FILES) || (dl == EE_ERROR))
					  dl_res = false;
					else if (dl_res && ((dl == EE_NO_FILES) || (dl == EE_ERROR)))
					  dl_res = true;
					else
					  dl_res = true;

                    if (!Config.LeaveRemoteFiles)
					  DeleteFilesFromServer(SuccessFiles);
				  }
				else
				  {
                    StrToList(dl_dirs, Config.RemDirDl, ";");

					for (int i = 0; i < dl_dirs->Count; i++)
					   {
						 int lc = SubDirLevelCount(dl_dirs->Strings[i]);

						 FtpLoader->ChangeDir(dl_dirs->Strings[i]);

						 dl = DownLoad(dl_dirs->Strings[i],
									   Config.DownloadFilesMask,
									   Config.LocDirDl,
									   Config.BackUpDirDl);

						 if ((dl == EE_NO_FILES) || (dl == EE_ERROR))
						   dl_res = false;
						 else if (dl_res && ((dl == EE_NO_FILES) || (dl == EE_ERROR)))
						   dl_res = true;
						 else
						   dl_res = true;

						 ReturnToRoot(lc);
					   }

					if (!Config.LeaveRemoteFiles)
					  DeleteFilesFromServer(SuccessFiles);
				  }
			  }
		   catch (Exception &e)
			  {
				WriteLog("Обмін з сервером: " + e.ToString());
				dl_res = false;
			  }
		 }
	  __finally {delete dl_dirs;}
	}

  if (Config.EnableUpload)
	{
	  TStringList *ul_dirs = new TStringList();

	  try
		 {
		   try
			  {
                SuccessFiles->Clear();
				ExchageExitCode ul;

				if (Config.RemDirUl == "")
				  {
					ul = UpLoad(Config.LocDirUl,
								Config.UploadFilesMask,
								"",
								Config.BackUpDirUl);

					if ((ul == EE_NO_FILES) || (ul == EE_ERROR))
					  ul_res = false;
					else if (ul_res && ((ul == EE_NO_FILES) || (ul == EE_ERROR)))
					  ul_res = true;
					else
					  ul_res = true;
				  }
				else
				  {
					StrToList(ul_dirs, Config.RemDirUl, ";");

					for (int i = 0; i < ul_dirs->Count; i++)
					   {
						 int lc = SubDirLevelCount(ul_dirs->Strings[i]);

						 FtpLoader->ChangeDir(ul_dirs->Strings[i]);

						 ul = UpLoad(Config.LocDirUl,
									 Config.UploadFilesMask,
									 ul_dirs->Strings[i],
									 Config.BackUpDirUl);

						 if ((ul == EE_NO_FILES) || (ul == EE_ERROR))
						   ul_res = false;
						 else if (ul_res && ((ul == EE_NO_FILES) || (ul == EE_ERROR)))
						   ul_res = true;
						 else
						   ul_res = true;

                         ReturnToRoot(lc);
					   }
				  }

                if (!Config.LeaveLocalFiles)
				  DeleteFiles(SuccessFiles);
			  }
		   catch (Exception &e)
			  {
				WriteLog("Обмін з сервером: " + e.ToString());
			  }
		 }
	  __finally {delete ul_dirs;}
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
		   WriteLog("Відключення від серверу: " + Config.Host + ":" + Config.Port);
		 }
	  catch (Exception &e)
		 {
		   WriteLog("Відключення від серверу: " + e.ToString());
		   result = -2;
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
			 Config.Caption + "\\" +
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
  TMemoryStream *ms = new TMemoryStream();
  int res;

  String src_name = source;
  int pos = src_name.LastDelimiter("/");
  src_name.Delete(1, pos);

  try
	 {
	   FtpLoader->Get(src_name, ms);
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
  __finally {delete ms;}

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
  TStringList *files = new TStringList();

  try
	 {
	   if (Config.RegExDL)
		 GetFTPFileListRegEx(files, mask);
	   else
		 GetFTPFileList(files, mask);
     }
   catch (Exception &e)
	 {
	   WriteLog("Помилка читання переліку файлів, " + source + "/" + mask + " :" + e.ToString());
	   result = 0;
     }


  TStringList *ok_files = new TStringList(); //перелік успішно завантажених файлів

  String src_name, remote_file;

  try
	 {
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
				else if (Config.AppendModeDL)
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

	   if (Config.BackUpDl && (backup != ""))
		 BackUpFiles(ok_files, backup);
	 }
   __finally {delete files; delete ok_files;}
   
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

  TStringList *DirList = new TStringList();
  int result = 0;

  if (GetFullDirList(DirList, "") < 0)
	{
	  delete DirList;

	  return 0;
	}

  try
	 {
	   result = LoadFilesFromServer(source, mask, destin, backup);

	   for (int i = 0; i < DirList->Count; i++)
		  {
			FtpLoader->ChangeDir(DirList->Strings[i]);

			if (Config.SubDirsCrt)
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
	 }
  __finally {delete DirList;}

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

  TStringList *files = new TStringList();
  TStringList *ok_files = new TStringList();

  try
	 {
	   if (Config.RegExUL)
		 GetFileListRegEx(files, source, mask, false, true);
	   else
		 GetFileList(files, source, mask, false, true);
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
			else if (Config.AppendModeUL)
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

  if (Config.BackUpUl && (backup != ""))
	BackUpFiles(ok_files, backup);

  delete files;
  delete ok_files;

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

  TStringList *DirList = new TStringList();
  int result = 0;
  String remote_dir;

  try
	 {
	   result = SendFilesToServer(source, mask, destin, backup);

	   GetDirList(DirList, source, WITHOUT_FULL_PATH);

	   for (int i = 0; i < DirList->Count; i++)
		  {
			if (Config.SubDirsCrt)
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
	 }
  __finally {delete DirList;}

  return result;
}
//---------------------------------------------------------------------------

bool TExchangeConnect::IsFtpDirExist(String source, String dir_name)
{
  TStringList *DirList = new TStringList();
  bool result = false;

  if (GetFullDirList(DirList, "") < 0)
	{
	  delete DirList;

	  return false;
	}

  try
	 {
	   for (int i = 0; i < DirList->Count; i++)
		  {
			if (DirList->Strings[i] == dir_name)
			  {
				result = true;
				break;
			  }
		  }
	 }
  __finally {delete DirList;}

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

  TStringList *DirList = new TStringList();
  String src_name;
  int FileCount = 0;

  try
	 {
       try
		  {
			FtpLoader->List("", true);
		  }
	   catch (Exception &e)
		  {
			WriteLog("Помилка читання переліку директорій, " + source + "/" + mask + " :" + e.ToString());

			delete DirList;

	   		return 0;
		  }

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
  __finally {delete DirList;}

  return FileCount;
}
//---------------------------------------------------------------------------

int TExchangeConnect::GetFTPFileCountWithSubDirsRegEx(String source, String reg_exp)
{
  if (!FtpLoader->Connected())
	return -1;

  TStringList *DirList = new TStringList();
  String src_name;
  int FileCount = 0;

  try
	 {
	   try
		  {
			FtpLoader->List("", true);
		  }
	   catch (Exception &e)
		  {
			WriteLog("Помилка читання переліку директорій, " + source + "/*"+ " :" + e.ToString());

			delete DirList;

	   		return 0;
		  }

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
  __finally {delete DirList;}

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
  TStringList *d_mask_lst = new TStringList();
  StrToList(d_mask_lst, mask, ";");

  Status = "Завантаження...";
  WriteLog(Status);

  for (int i = 0; i < d_mask_lst->Count; i++)
	 {
	   if (Config.SubDirsDl)
		 {
		   if (Config.RegExDL)
			 fcnt = GetFTPFileCountWithSubDirsRegEx(source, d_mask_lst->Strings[i]);
		   else
			 fcnt = GetFTPFileCountWithSubDirs(source, d_mask_lst->Strings[i]);
		 }
	   else
		 {
		   if (Config.RegExDL)
			 fcnt = GetFTPFileCountRegEx(source, d_mask_lst->Strings[i]);
		   else
			 fcnt = GetFTPFileCount(source, d_mask_lst->Strings[i]);
         }

	   if (fcnt > 0)
		 {
		   int res;

		   if (Config.SubDirsDl)
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
					 result == EE_SOME_FILES;
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

  delete d_mask_lst;

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

  TStringList *u_mask_lst = new TStringList();
  StrToList(u_mask_lst, mask, ";");
  int cnt;
  ExchageExitCode result = EE_NO_FILES;

  for (int i = 0; i < u_mask_lst->Count; i++)
	 {
	   if (Config.SubDirsDl)
		 {
		   try
			  {
				if (Config.RegExUL)
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
				if (Config.RegExUL)
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

		   if (Config.SubDirsDl)
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
					 result == EE_SOME_FILES;
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

  delete u_mask_lst;

  return result;
}
//-------------------------------------------------------------------------

void TExchangeConnect::WriteLog(String text)
{
  text = "{id: " + IntToStr(this->ID) + "} " + this->ServerCaption + ": " + text;

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

  Config.Caption = ParsingVariables(Config.Caption, &vars);
  Config.Host = ParsingVariables(Config.Host, &vars);
  Config.User = ParsingVariables(Config.User, &vars);
  Config.Pwd = ParsingVariables(Config.Pwd, &vars);
  Config.RemDirDl = ParsingVariables(Config.RemDirDl, &vars);
  Config.LocDirDl = ParsingVariables(Config.LocDirDl, &vars);
  Config.BackUpDirDl = ParsingVariables(Config.BackUpDirDl, &vars);
  Config.DownloadFilesMask = ParsingVariables(Config.DownloadFilesMask, &vars);
  Config.RemDirUl = ParsingVariables(Config.RemDirUl, &vars);
  Config.LocDirUl = ParsingVariables(Config.LocDirUl, &vars);
  Config.BackUpDirUl = ParsingVariables(Config.BackUpDirUl, &vars);
  Config.UploadFilesMask = ParsingVariables(Config.UploadFilesMask, &vars);
}
//---------------------------------------------------------------------------

void TExchangeConnect::DeleteOldBackUpDirs()
{
  TStringList *dir_list = new TStringList(),
			  *del_list = new TStringList();

  try
	 {
	   if (DirectoryExists(Config.BackUpDirDl))
		 {
		   GetDirList(dir_list, Config.BackUpDirDl, WITHOUT_FULL_PATH);

		   for (int i = 0; i < dir_list->Count; i++)
			  {
				TDate dir_date = TDate(dir_list->Strings[i]);

				if (dir_date < (Date().CurrentDate() - Config.BackUpKeepDays))
				  del_list->Add(dir_list->Strings[i]);
			  }
         }

	   dir_list->Clear();

	   for (int i = 0; i < del_list->Count; i++)
		  {
			DeleteAllFromDir(Config.BackUpDirDl + "\\" + del_list->Strings[i]);
			RemoveDir(Config.BackUpDirDl + "\\" + del_list->Strings[i]);
		  }

       del_list->Clear();

	   if (DirectoryExists(Config.BackUpDirUl))
		 {
		   GetDirList(dir_list, Config.BackUpDirUl, WITHOUT_FULL_PATH);

		   TDate dir_date;

		   for (int i = 0; i < dir_list->Count; i++)
			  {
				try
				   {
					 dir_date = TDate(dir_list->Strings[i]);

                     if (dir_date < (Date().CurrentDate() - Config.BackUpKeepDays))
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
			DeleteAllFromDir(Config.BackUpDirUl + "\\" + del_list->Strings[i]);
			RemoveDir(Config.BackUpDirUl + "\\" + del_list->Strings[i]);
		  }
	 }
  __finally {delete dir_list; delete del_list;}
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
	   TIdHashCRC32 *local_hash;
	   TFileStream *local_stream;

	   try
		  {
            local_hash = new TIdHashCRC32();
			local_stream = new TFileStream(local_file, fmOpenRead);
			local_crc = local_hash->HashValue(local_stream);
		  }
	   __finally
		  {
			if (local_hash) delete local_hash;
			if (local_stream) delete local_stream;
		  }

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
