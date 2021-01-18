/*!
Copyright 2019-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef TExchangeConnectH
#define TExchangeConnectH

#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <IdFTP.hpp>
#include <IdHashCRC.hpp>
#include <vector>
#include "..\..\work-functions\ThreadSafeLog.h"

enum ExchageExitCode {EE_NO_FILES = 0,
					  EE_ALL_FILES = 1,
					  EE_SOME_FILES = 2,
                      EE_SOME_FILES_WITH_ERR = 3,
					  EE_ERROR = -2};

extern String AppPath, DataPath, IndexVZ, LogName, StationID;

struct VARPAIR
{
  String Var;
  String Val;
};

struct SERVCFG
{
  String Caption;
  String Host;
  int Port;
  String User;
  String Pwd;
  String RemDirDl;
  String RemDirUl;
  String LocDirDl;
  String LocDirUl;
  String BackUpDirDl;
  String BackUpDirUl;
  unsigned char TransType;
  int MonitoringInterval;
  int ConnInterval;
  int MaxTry;
  String UploadFilesMask;
  bool RegExUL;
  String DownloadFilesMask;
  bool RegExDL;
  bool LeaveRemoteFiles;
  bool LeaveLocalFiles;
  bool EnableDownload;
  bool EnableUpload;
  bool BackUpDl;
  bool BackUpUl;
  bool RunOnce;
  bool StartAtTime;
  bool SubDirsDl;
  bool SubDirsCrt;
  TTime TimeStart;
  int BackUpKeepDays;
  bool AppendModeUL;
  bool AppendModeDL;
};

class TExchangeConnect
{
  private:
	bool Running;
	bool Init;
	int ID;
	unsigned int thID;
	String Status;
	String CfgPath;
	SERVCFG Config;
	TIdFTP *FtpLoader;
	TThreadSafeLog *Log;
	bool EndThread;
	int ExchangeStatus;
	TStringList *SuccList;

	bool ConnectToFTP();

	void DeleteFiles(TStringList *files);

	void DeleteFilesFromServer(TStringList *files);

	int BackUpFiles(TStringList *files, String destin);

	int LoadFilesFromServer(String source,
							String mask,
							String destin,
							String backup);

	int LoadFilesFromServerSubDirs(String source,
								   String mask,
								   String destin,
								   String backup);

	int SendFilesToServer(String source,
						  String mask,
						  String destin,
						  String backup);

	int SendFilesToServerSubDirs(String source,
								 String mask,
								 String destin,
								 String backup);

	bool IsFileLocked(const String &file);
    int SubDirLevelCount(const String &source);
	void ReturnToRoot(int level);
	void AddSuccessFile(String file);

	int GetFTPFileCount(String source, String mask);
	int GetFTPFileCountRegEx(String source, String reg_exp);

	int GetFTPFileCountWithSubDirs(String source, String mask);
	int GetFTPFileCountWithSubDirsRegEx(String source, String reg_exp);

	int GetFTPFileList(TStringList *list, String mask);
	int GetFTPFileListRegEx(TStringList *list, String reg_exp);

    int GetFTPFile(String source, String destin, int list_index);

	int GetFullDirList(TStringList *list, String mask);

	bool IsFtpDirExist(String source, String dir_name);
	bool IsFtpFileExist(String source, String file_name);

	String ExtractDirNameFromPath(const String &filepath);
    String ExtractFileNameFromPath(const String &filepath);

	int CreateServerCfgDirs();

	ExchageExitCode DownLoad(String source, String mask, String destin, String backup);

	ExchageExitCode UpLoad(String source, String mask, String destin, String backup);

    SERVCFG *GetConfig(){return &Config;}

	void CheckConfig(String cfg_file);

	int ReadConfig(String cfg_file);

	void Initialize();

	void SetFtpLoader();

	String ParsingVariables(String str, std::vector<VARPAIR> *vars);

	void ParsingParamsForVars();

	void DeleteOldBackUpDirs();

	void WriteLog(String text);

	bool GetVerification();

	bool VerifyFile(const String &remote_file, const String &local_file);

  public:
	TExchangeConnect(int ID,
					 TTrayIcon *Icon,
					 TThreadSafeLog *Log);

	TExchangeConnect(String cfg_file,
					 int ID,
					 TTrayIcon *Icon,
					 TThreadSafeLog *Log);

	virtual ~TExchangeConnect();

	__property int ServerID = {read = ID, write = ID};
	__property unsigned int ServerThreadID = {read = thID, write = thID};
	__property String ServerCaption = {read = Config.Caption};
	__property String ServerStatus = {read = Status, write = Status};
	__property String ServerCfgPath = {read = CfgPath};
	__property SERVCFG *ConnectionConfig = {read = GetConfig};
	__property bool EndServerThread = {read = EndThread, write = EndThread};
	__property int CurrentExchangeStatus = {read = ExchangeStatus};
	__property TStringList *SuccessFiles = {read = SuccList, write = SuccList};
	__property bool SupportsVerification = {read = GetVerification};


	inline bool Working(){return Running;}

	inline bool Connected(){return FtpLoader->Connected();}

	inline bool Initialized(){return Init;}

	inline bool ReInitialize(){Initialize(); return Init;}

	void Stop();

	void Start();

	int Exchange();
};

//---------------------------------------------------------------------------
#endif
