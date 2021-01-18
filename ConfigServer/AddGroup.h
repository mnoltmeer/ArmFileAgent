//---------------------------------------------------------------------------

#ifndef AddGroupH
#define AddGroupH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TAddGroupForm : public TForm
{
__published:	// IDE-managed Components
	TLabeledEdit *Name;
	TBitBtn *Apply;
	void __fastcall ApplyClick(TObject *Sender);
	void __fastcall FormShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TAddGroupForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TAddGroupForm *AddGroupForm;
//---------------------------------------------------------------------------
#endif
