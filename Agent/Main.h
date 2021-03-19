/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
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
#include "TExchangeConnect.h"

#define INIT_DIALOG 0
#define INIT_CMDLINE 1
//---------------------------------------------------------------------------

class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TIdSMTP *MailSender;
	TTrayIcon *TrayIcon;
	TPopupMenu *PopupMenu;
	TMenuItem *IconPP1;
	TIdTCPServer *AURAServer;
	TImageList *ImageList;
	TLabel *LbStatus;
	TTimer *SaveLogTimer;
	TMenuItem *IconPP5;
	TLabel *Label1;
	TLabel *ModuleVersion;
	void __fastcall IconPP1Click(TObject *Sender);
	void __fastcall AURAServerExecute(TIdContext *AContext);
	void __fastcall SaveLogTimerTimer(TObject *Sender);
	void __fastcall IconPPConnClick(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);

private:	// User declarations
	TList *MenuItemList;

	bool __fastcall ConnectToSMTP();
	void __fastcall SendMsg(String mail_addr, String subject, String from, String log);
	void __fastcall ShowInfoMsg(String textr);
	void __fastcall CheckConfig(String cfg_file);

//передстартові дії. Перехоплення виключень у цій функції не проводиться
	void __fastcall StartApplication();

	void __fastcall RunWork(TExchangeConnect *server);
	void __fastcall EndWork(TExchangeConnect *server);

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
    int __fastcall ASendStatusAnswer(TIdContext *AContext);

	void __fastcall LoadFunctionsToELI();
	void __fastcall StopApplication();

	TExchangeConnect* __fastcall CreateConnection(String file);
	void __fastcall FirstStartInitialisation(int type);
	void __fastcall Migration();
	void __fastcall Unregistration();

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);

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
	void __fastcall ShutdownAgent();
	void __fastcall ShutdownGuardian();
	int __fastcall StartGuardian();
	int __fastcall RestartGuardian();

	int __fastcall ReadSettings();
	void __fastcall WriteSettings();
    void __fastcall WriteModulePath();
	void __fastcall AddFirewallRule();
	void __fastcall RemoveFirewallRule();

    int __fastcall ConnectELI();
	int __fastcall ReleaseELI();
	void __fastcall ExecuteScript(String ctrl_script_name);
	void __fastcall ExecuteScript(const wchar_t *ctrl_script_text);
	void __fastcall UpdateRequest();
    void __fastcall SendStartUpdateMessage();
	int __fastcall ReadConfig();
	void __fastcall RebuildConnection(String file);
	void __fastcall CheckAndStartConnection(String file);
	void __fastcall CreateExistConnections();
    bool __fastcall WaitForLoadFromServer(long millisec);

    int __fastcall SendLog(String mail_addr, String subject, String from, String log);

    void __fastcall WndProc(Messages::TMessage& Msg);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------

void __stdcall eConnectionsCount(void *p);

//створення з'єднання через передачу імені файлу, що описує конфіг
//повертає ІД з'єднання
//pFile - шлях до файлу, що описує конфіг
//pAddMenu - вказує, чи треба створювати елемент контекстного меню
void __stdcall eCreateConnection(void *p);

//знищення з'єднання
//pID - ІД з'єднання
void __stdcall eDestroyConnection(void *p);

//знищення з'єднання та видалення файлу конфігу
//pID - ІД з'єднання
void __stdcall eRemoveConnection(void *p);

//запуск з'єднання
//pID - ІД з'єднання
void __stdcall eStartConnection(void *p);

//зупинка з'єднання
//pID - ІД з'єднання
void __stdcall eStopConnection(void *p);

//шукає ІД з'єднання за описом
//повертає ІД, або 0, якщо з'єднання не знайдене
//pCap - Caption з'єднання
void __stdcall eConnectionID(void *p);

//шукає ІД з'єднання за індексом у переліку з'єднань
//повертає ІД, або 0, якщо з'єднання не знайдене
//pIndex - індекс у списку з'єднання
void __stdcall eConnectionIDInd(void *p);

//повертає статус з'єднання
//1 - в роботі,
//0 - зупинено,
//-1 - помилка, не ініціалізоване
//pID - ІД з'єднання
void __stdcall eConnectionStatus(void *p);

//повертає шлях до конфігу з'єднання
//pID - ІД з'єднання
void __stdcall eConnectionCfgPath(void *p);

//перечитування параметрів з'єднаннь
void __stdcall eReloadConfig(void *p);

//читання певного параметру з конфігу
//pFile - шлях до файлу, що описує конфіг
//pPrm - ім'я параметру
void __stdcall eReadFromCfg(void *p);

//видалення певного параметру з конфігу
//pFile - шлях до файлу, що описує конфіг
//pPrm - ім'я параметру
void __stdcall eRemoveFromCfg(void *p);

//запис певного параметру у конфіг
//pFile - шлях до файлу, що описує конфіг
//pPrm - ім'я параметру
//pVal - значення параметру
void __stdcall eWriteToCfg(void *p);
//теж саме, тільки конфіг визначається по ІД з'єднання
void __stdcall eWriteToCfgByID(void *p);

void __stdcall eWriteMsgToLog(void *p);
void __stdcall eGetAppPath(void *p);
void __stdcall eGetDataPath(void *p);
void __stdcall eShutdownAgent(void *p);
void __stdcall eShutdownGuardian(void *p);
void __stdcall eStartGuardian(void *p);
void __stdcall eRestartGuardian(void *p);
#endif
