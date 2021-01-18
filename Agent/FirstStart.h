//---------------------------------------------------------------------------

#ifndef FirstStartH
#define FirstStartH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Buttons.hpp>
//---------------------------------------------------------------------------
class TFirstStartForm : public TForm
{
__published:	// IDE-managed Components
	TRadioGroup *InitKind;
	TPanel *ManualInitPanel;
	TPanel *MigrationInitPanel;
	TLabeledEdit *Index;
	TLabeledEdit *RemoteAdminPort;
	TCheckBox *EnableAutoStart;
	TCheckBox *AllUsersAutoStart;
	TOpenDialog *OpenCfgDialog;
	TLabeledEdit *ManagerCfgPath;
	TButton *OpenManagerCfg;
	TBitBtn *Cancel;
	TBitBtn *Apply;
	TLabeledEdit *CfgServerHost;
	TLabeledEdit *CfgServerPort;
	TLabel *LbStationID;
	TLabel *Label1;
	TLabel *Label2;
	TLabel *Label3;
	TLabel *Label4;
	TLabel *Label5;
	TLabel *Label6;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *Label9;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall InitKindClick(TObject *Sender);
	void __fastcall ApplyClick(TObject *Sender);
	void __fastcall CancelClick(TObject *Sender);
	void __fastcall EnableAutoStartClick(TObject *Sender);
private:	// User declarations
	void __fastcall AddFirewallRule();
	void __fastcall WriteSettings();
public:		// User declarations
	__fastcall TFirstStartForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFirstStartForm *FirstStartForm;
//---------------------------------------------------------------------------
#endif
