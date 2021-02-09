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
	TLabel *MigrIndexVZ;
	TLabel *Label3;
	TLabel *MigrRemAdmPort;
	TLabel *Label5;
	TLabel *MigrAutoStart;
	TLabel *Label7;
	TLabel *Label8;
	TLabel *MigrAutoStartForAll;
	TCheckBox *ManageFirewall;
	void __fastcall FormShow(TObject *Sender);
	void __fastcall InitKindClick(TObject *Sender);
	void __fastcall ApplyClick(TObject *Sender);
	void __fastcall CancelClick(TObject *Sender);
	void __fastcall EnableAutoStartClick(TObject *Sender);
	void __fastcall OpenManagerCfgClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TFirstStartForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFirstStartForm *FirstStartForm;
//---------------------------------------------------------------------------
#endif
