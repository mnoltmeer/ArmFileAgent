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
#include <IdContext.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdTCPServer.hpp>
#include <System.ImageList.hpp>
#include <Vcl.Imaging.pngimage.hpp>
#include <Vcl.ImgList.hpp>

//---------------------------------------------------------------------------
class TServerForm : public TForm
{
__published:	// IDE-managed Components
	TOpenDialog *OpenCfgDialog;
	TSaveDialog *SaveCfgDialog;
	TPanel *LogPanel;
	TMemo *Log;
	TPanel *MainPanel;
	TPanel *AddrBookPanel;
	TTreeView *AddrList;
	TPanel *ControlsPanel;
	TValueListEditor *ItemParams;
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
	TPanel *ControlFooterPanel;
	TPanel *ControlHeaderPanel;
	TLabel *Label1;
	TImage *SwServerOff;
	TImage *SwServerOn;
	TIdTCPServer *Listener;
	TPopupMenu *MainPopupMenu;
	TMenuItem *PPStatus;
	TMenuItem *IconPP6;
	TMenuItem *PPShowWindow;
	TMenuItem *PPClose;
	TTrayIcon *TrayIcon;
	TImageList *MenuImages;
	TBitBtn *Settings;
	TPanel *ConfigManagePanel;
	TBitBtn *RemoveConfig;
	TBitBtn *AddConfig;
	TTimer *StatusChecker;
	TListBox *ConfigList;
	TLabel *LbVersion;
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall AddrListClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall EditBookClick(TObject *Sender);
	void __fastcall AddToBookClick(TObject *Sender);
	void __fastcall DeleteFromBookClick(TObject *Sender);
	void __fastcall AddGroupBookClick(TObject *Sender);
	void __fastcall ImportInAddrBookClick(TObject *Sender);
	void __fastcall ExportFromAddrBookClick(TObject *Sender);
	void __fastcall LogFilterChange(TObject *Sender);
	void __fastcall ListenerExecute(TIdContext *AContext);
	void __fastcall ListenerConnect(TIdContext *AContext);
	void __fastcall ListenerDisconnect(TIdContext *AContext);
	void __fastcall SettingsClick(TObject *Sender);
	void __fastcall AddConfigClick(TObject *Sender);
	void __fastcall RemoveConfigClick(TObject *Sender);
	void __fastcall PPShowWindowClick(TObject *Sender);
	void __fastcall PPCloseClick(TObject *Sender);
	void __fastcall StatusCheckerTimer(TObject *Sender);
	void __fastcall MainPopupMenuPopup(TObject *Sender);
	void __fastcall LogFilterDropDown(TObject *Sender);
	void __fastcall TrayIconDblClick(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);

private:	// User declarations
	int __fastcall ReadTmpCfg(String cfg);
	int __fastcall ReadServerList();
	int __fastcall ReadRemoteVersion();
	int __fastcall AskToClient(const wchar_t *host, int port, TStringStream *rw_bufer);
	int __fastcall SendToClient(const wchar_t *host, int port, TStringStream *rw_bufer);
    int __fastcall SendToClient(const wchar_t *host, int port, const String &data);
	TIdTCPClient* __fastcall CreateSender(const wchar_t *host, int port);
	void __fastcall FreeSender(TIdTCPClient *sender);
    void __fastcall StartServer();
	void __fastcall StopServer();
	String __fastcall CreateClientFileList(const String &index, const String &station);
    void __fastcall WriteSettings();
	void __fastcall ReadSettings();

public:		// User declarations
	__fastcall TServerForm(TComponent* Owner);

    void __fastcall WriteLog(String record);
};
//---------------------------------------------------------------------------
extern PACKAGE TServerForm *ServerForm;

//---------------------------------------------------------------------------
#endif
