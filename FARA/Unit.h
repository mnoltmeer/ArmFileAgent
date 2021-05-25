/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef UnitH
#define UnitH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Grids.hpp>
#include <Vcl.ValEdit.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdTCPClient.hpp>
#include <IdTCPConnection.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Menus.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ImgList.hpp>

//---------------------------------------------------------------------------
class TAURAForm : public TForm
{
__published:	// IDE-managed Components
	TOpenDialog *OpenCfgDialog;
	TSaveDialog *SaveCfgDialog;
	TPanel *LogPanel;
	TMemo *Log;
	TPanel *MainPanel;
	TPanel *AddrBookPanel;
	TTreeView *AddrList;
	TPanel *ActionLogPanel;
	TMemo *ActionLog;
	TPanel *Panel6;
	TLabel *Label6;
	TButton *SaveLog;
	TPanel *ControlsPanel;
	TValueListEditor *CfgList;
	TPanel *AdrBookBtPanel;
	TBitBtn *AddGroupBook;
	TBitBtn *AddToBook;
	TBitBtn *DeleteFromBook;
	TBitBtn *EditBook;
	TBitBtn *ImportInAddrBook;
	TBitBtn *ExportFromAddrBook;
	TPanel *LogFilterPanel;
	TLabel *Label9;
	TComboBox *LogFilter;
	TPopupMenu *ConnPopupMenu;
	TMenuItem *PPConfig;
	TMenuItem *PPConnection;
	TMenuItem *PPConfigShow;
	TMenuItem *PPConnectionStart;
	TMenuItem *PPConnectionStop;
	TPanel *ControlHeaderPanel;
	TLabel *Label2;
	TEdit *Host;
	TLabel *Label3;
	TEdit *Port;
	TLabel *Label1;
	TButton *Connect;
	TLabel *Label7;
	TLabel *ModuleVersion;
	TPanel *ControlFooterPanel;
	TButton *SaveCfgList;
	TButton *GetStatus;
	TButton *GetThreadList;
	TButton *GetLog;
	TLabel *Label5;
	TLabel *Label8;
	TComboBox *CfgKind;
	TComboBox *ServList;
	TButton *ReadCfg;
	TButton *CmdRun;
	TButton *CmdStop;
	TButton *Shutdown;
	TButton *RestartGuard;
	TButton *SendScript;
	TPanel *CfgServerConnPanel;
	TLabel *Label4;
	TEdit *CfgServerHost;
	TLabel *Label10;
	TLabel *Label11;
	TEdit *CfgServerPort;
	TBitBtn *LoadAddrBookFromServer;
	TButton *SendCheckUpds;
	TImageList *MenuImages;
	void __fastcall ConnectClick(TObject *Sender);
	void __fastcall PortClick(TObject *Sender);
	void __fastcall ReadCfgClick(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall HostKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall PortKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall GetLogClick(TObject *Sender);
	void __fastcall GetStatusClick(TObject *Sender);
	void __fastcall CmdRunClick(TObject *Sender);
	void __fastcall CmdStopClick(TObject *Sender);
	void __fastcall GetThreadListClick(TObject *Sender);
	void __fastcall AddrListClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall AddrListDblClick(TObject *Sender);
	void __fastcall ShutdownClick(TObject *Sender);
	void __fastcall RestartGuardClick(TObject *Sender);
	void __fastcall EditBookClick(TObject *Sender);
	void __fastcall AddToBookClick(TObject *Sender);
	void __fastcall DeleteFromBookClick(TObject *Sender);
	void __fastcall AddGroupBookClick(TObject *Sender);
	void __fastcall ImportInAddrBookClick(TObject *Sender);
	void __fastcall SaveCfgListClick(TObject *Sender);
	void __fastcall ExportFromAddrBookClick(TObject *Sender);
	void __fastcall SaveLogClick(TObject *Sender);
	void __fastcall LogFilterChange(TObject *Sender);
	void __fastcall PPConfigShowClick(TObject *Sender);
	void __fastcall PPConnectionStartClick(TObject *Sender);
	void __fastcall PPConnectionStopClick(TObject *Sender);
	void __fastcall CfgListMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall CfgListMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall LoadAddrBookFromServerClick(TObject *Sender);
	void __fastcall SendScriptClick(TObject *Sender);
	void __fastcall SendCheckUpdsClick(TObject *Sender);

private:	// User declarations
	int __fastcall ReadTmpCfg(String cfg);
	int __fastcall ReadServerList();
	int __fastcall ReadRemoteVersion();
	int __fastcall AskToServer(const wchar_t *host, int port, TStringStream *rw_bufer);
	void __fastcall CreateNewForm();
	void __fastcall CreateEditForm();
	void __fastcall RequestLog(int conn_id);
	int __fastcall GetConnectionID(const String &str_with_id);
    void __fastcall WriteSettings();
	void __fastcall ReadSettings();
    TIdTCPClient* __fastcall CreateSender(const wchar_t *host, int port);
	void __fastcall FreeSender(TIdTCPClient *sender);
    void __fastcall SenderConnected(TObject *Sender);
	void __fastcall SenderDisconnected(TObject *Sender);
    void __fastcall ImportHostStatus(const String &file);

public:		// User declarations
	__fastcall TAURAForm(TComponent* Owner);

	void __fastcall AddActionLog(String status);
    int __fastcall SendToServer(const wchar_t *host, int port, TStringStream *rw_bufer);

	void __fastcall EditFormShow(TObject *Sender);
	void __fastcall EditApplyClick(TObject *Sender);
	void __fastcall EditCancelClick(TObject *Sender);

	void __fastcall NewFormShow(TObject *Sender);
	void __fastcall NewApplyClick(TObject *Sender);
	void __fastcall NewCancelClick(TObject *Sender);
};
//---------------------------------------------------------------------------
extern PACKAGE TAURAForm *AURAForm;

//---------------------------------------------------------------------------
#endif
