/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef GuardMainH
#define GuardMainH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Registry.hpp>
#include <Vcl.AppEvnts.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TGuardian : public TForm
{
__published:	// IDE-managed Components
	TTimer *SaveLogTimer;
	void __fastcall SaveLogTimerTimer(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);

private:	// User declarations
	String __fastcall GetAgentPath();

public:		// User declarations
	__fastcall TGuardian(TComponent* Owner);

    void __fastcall WndProc(Messages::TMessage& Msg);
};
//---------------------------------------------------------------------------
extern PACKAGE TGuardian *Guardian;
//---------------------------------------------------------------------------
#endif
