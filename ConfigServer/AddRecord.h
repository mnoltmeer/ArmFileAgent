//---------------------------------------------------------------------------

#ifndef AddRecordH
#define AddRecordH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TAddRecordForm : public TForm
{
__published:	// IDE-managed Components
	TLabeledEdit *Station;
	TComboBox *NewGroupList;
	TLabel *Label1;
	TLabeledEdit *Host;
	TLabeledEdit *Port;
	TBitBtn *Apply;
	TBitBtn *Cancel;
	TBitBtn *NewGroup;
	void __fastcall ApplyClick(TObject *Sender);
	void __fastcall CancelClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall StationKeyPress(TObject *Sender, System::WideChar &Key);
	void __fastcall NewGroupClick(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TAddRecordForm(TComponent* Owner);

    void __fastcall ClearData();
};
//---------------------------------------------------------------------------
extern PACKAGE TAddRecordForm *AddRecordForm;
//---------------------------------------------------------------------------
#endif
