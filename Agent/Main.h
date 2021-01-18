/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef MainH
#define MainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdExplicitTLSClientServerBase.hpp>
#include <IdFTP.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <IdAllFTPListParsers.hpp>
#include <Tlhelp32.h>
#include <IdMessageClient.hpp>
#include <IdSMTP.hpp>
#include <IdSMTPBase.hpp>
#include <IdIntercept.hpp>
#include <IdLogBase.hpp>
#include <IdLogFile.hpp>
#include <IdIOHandler.hpp>
#include <IdIOHandlerSocket.hpp>
#include <IdIOHandlerStack.hpp>
#include <IdIcmpClient.hpp>
#include <IdRawBase.hpp>
#include <IdRawClient.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Menus.hpp>
#include <IdSimpleServer.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdTCPServer.hpp>
#include <IdContext.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Registry.hpp>

#define BUILD_APP

#include "eli_interface.h"
#include "TAMEliThread.h"

#define INIT_DIALOG 0
#define INIT_MANUAL 1
//---------------------------------------------------------------------------

String AppPath;

TDate DateStart;

const int MainPrmCnt = 16;

const wchar_t *MainParams[MainPrmCnt] = {L"StationID",
										 L"IndexVZ",
										 L"MailFrom",
										 L"MailTo",
										 L"MailCodePage",
										 L"SmtpHost",
										 L"SmtpPort",
										 L"RemAdmPort",
										 L"SendReportToMail",
										 L"EnableAutoStart",
										 L"MailSubjectErr",
										 L"MailSubjectOK",
										 L"ControlScriptName",
										 L"ScriptLog",
										 L"ScriptInterval",
										 L"AutoStartForAllUsers"};

class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TIdSMTP *MailSender;
	TTrayIcon *TrayIcon1;
	TPopupMenu *PopupMenu1;
	TMenuItem *IconPP1;
	TIdTCPServer *AURAServer;
	TImageList *ImageList1;
	TImage *SwitchOn;
	TImage *SwitchOff;
	TLabel *LbStatus;
	TTimer *SaveLogTimer;
	TMenuItem *IconPP5;
	TLabel *Label1;
	TLabel *ModuleVersion;
	void __fastcall IconPP1Click(TObject *Sender);
	void __fastcall AURAServerExecute(TIdContext *AContext);
	void __fastcall SwitchOnClick(TObject *Sender);
	void __fastcall SwitchOffClick(TObject *Sender);
	void __fastcall SaveLogTimerTimer(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall IconPPConnClick(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);

private:	// User declarations
	TList *MenuItemList;

	TList *GetSrvList(){return SrvList;}
    void SetSrvList(TList *val){if (val){SrvList = val;}}
	TList *GetThList(){return ThreadList;}
	void SetThList(TList *val){if (val){ThreadList = val;}}

    bool __fastcall ConnectToSMTP();
	void __fastcall SendMsg(String mail_addr, String subject, String from, String log);
	void __fastcall ShowInfoMsg(String textr);
	int __fastcall SendLog(String mail_addr, String subject, String from, String log);
	void __fastcall CheckConfig(String cfg_file);
	int __fastcall ReadConfig();

	void __fastcall CreateServers();
	TExchangeConnect* __fastcall FindServer(int id);

	TAMThread* __fastcall FindServerThread(unsigned int thread_id);
	void __fastcall DeleteServerThread(unsigned int id);
	void __fastcall DeleteServerThreads();

	void __fastcall RunWork(TExchangeConnect *server);
	void __fastcall ResumeWork(TExchangeConnect *server);
	void __fastcall EndWork(TExchangeConnect *server);

	void __fastcall StartWork();
	void __fastcall StopWork();

	bool __fastcall GuardianRunning();
	int __fastcall RunGuardian();

	int __fastcall AAnswerToClient(TIdContext *AContext, TStringStream *ms);

	int __fastcall ASendStatus(TIdContext *AContext);
	int __fastcall ASendConfig(TStringList *list, TIdContext *AContext);
	int __fastcall ASendLog(TIdContext *AContext);
	int __fastcall ASendConnList(TIdContext *AContext);
	int __fastcall ASendThreadList(TIdContext *AContext);
	int __fastcall ASendFile(TStringList *list, TIdContext *AContext);
	int __fastcall ASendVersion(TIdContext *AContext);

	int __fastcall ConnectELI();
	int __fastcall ReleaseELI();
	void __fastcall ExecuteScript(String ctrl_script_name);
	void __fastcall LoadFunctionsToELI();
	void __fastcall InitInstance();
	void __fastcall StopInstance();

	TExchangeConnect* __fastcall CreateConnection(String file);
	void __fastcall FirstStartInitialisation(int type);
    void __fastcall Migration();

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);

	int __fastcall CreateConnection(SERVCFG cfg, bool create_menu);
	int __fastcall CreateConnection(String file, bool create_menu);
	void __fastcall DestroyConnection(int id);
    void __fastcall RemoveConnection(int id);
	void __fastcall StartConnection(int id);
	void __fastcall StopConnection(int id);
	int __fastcall GetConnectionID(String caption);
	int __fastcall GetConnectionID(int index);
	int __fastcall ConnectionStatus(int id);
	String __fastcall ConnectionCfgPath(int id);
	void __fastcall ReloadConfig();
	bool __fastcall WriteToCfg(String file, String param, String val);
	bool __fastcall WriteToCfg(int id, String param, String val);
	bool __fastcall RemoveFromCfg(String file, String param);
	void __fastcall WriteToMngrLog(String msg);
	void __fastcall ShutdownManager();
	void __fastcall ShutdownGuardian();
	int __fastcall StartGuardian();
	int __fastcall RestartGuardian();
	int __fastcall ReadSettings();

	__property TList *Connections = {read = GetSrvList, write = SetSrvList};
	__property TList *Threads = {read = GetThList, write = SetThList};

    void __fastcall WndProc(Messages::TMessage& Msg);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------

void __stdcall eConnectionsCount(void *p);

//��������� �'������� ����� �������� ����� �����, �� ����� ������
//������� �� �'�������
//pFile - ���� �� �����, �� ����� ������
//pAddMenu - �����, �� ����� ���������� ������� ������������ ����
void __stdcall eCreateConnection(void *p);

//�������� �'�������
//pID - �� �'�������
void __stdcall eDestroyConnection(void *p);

//�������� �'������� �� ��������� ����� �������
//pID - �� �'�������
void __stdcall eRemoveConnection(void *p);

//������ �'�������
//pID - �� �'�������
void __stdcall eStartConnection(void *p);

//������� �'�������
//pID - �� �'�������
void __stdcall eStopConnection(void *p);

//���� �� �'������� �� ������
//������� ��, ��� 0, ���� �'������� �� ��������
//pCap - Caption �'�������
void __stdcall eConnectionID(void *p);

//���� �� �'������� �� �������� � ������� �'������
//������� ��, ��� 0, ���� �'������� �� ��������
//pIndex - ������ � ������ �'�������
void __stdcall eConnectionIDInd(void *p);

//������� ������ �'�������
//1 - � �����,
//0 - ��������,
//-1 - �������, �� �������������
//pID - �� �'�������
void __stdcall eConnectionStatus(void *p);

//������� ���� �� ������� �'�������
//pID - �� �'�������
void __stdcall eConnectionCfgPath(void *p);

//������������� ��������� �'�������
void __stdcall eReloadConfig(void *p);

//������� ������� ��������� � �������
//pFile - ���� �� �����, �� ����� ������
//pPrm - ��'� ���������
void __stdcall eReadFromCfg(void *p);

//��������� ������� ��������� � �������
//pFile - ���� �� �����, �� ����� ������
//pPrm - ��'� ���������
void __stdcall eRemoveFromCfg(void *p);

//����� ������� ��������� � ������
//pFile - ���� �� �����, �� ����� ������
//pPrm - ��'� ���������
//pVal - �������� ���������
void __stdcall eWriteToCfg(void *p);
//��� ����, ����� ������ ����������� �� �� �'�������
void __stdcall eWriteToCfgByID(void *p);

void __stdcall eWriteMsgToLog(void *p);
void __stdcall eGetAppPath(void *p);
void __stdcall eGetDataPath(void *p);
void __stdcall eShutdownManager(void *p);
void __stdcall eShutdownGuardian(void *p);
void __stdcall eStartGuardian(void *p);
void __stdcall eRestartGuardian(void *p);
#endif
