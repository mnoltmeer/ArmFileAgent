//---------------------------------------------------------------------------

#ifndef ScriptEditorH
#define ScriptEditorH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.Buttons.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.Clipbrd.hpp>
//---------------------------------------------------------------------------
class TScriptForm : public TForm
{
__published:	// IDE-managed Components
	TPanel *ControlPanel;
	TRichEdit *Editor;
	TOpenDialog *OpenScriptDialog;
	TBitBtn *LoadScript;
	TBitBtn *SendScript;
	TPopupMenu *EditorPopup;
	TMenuItem *PPCopy;
	TMenuItem *PPCut;
	TMenuItem *PPPaste;
	TMenuItem *PPSelectAll;
	TComboBox *ScriptPath;
	void __fastcall ScriptPathChange(TObject *Sender);
	void __fastcall PPCopyClick(TObject *Sender);
	void __fastcall PPCutClick(TObject *Sender);
	void __fastcall PPPasteClick(TObject *Sender);
	void __fastcall PPSelectAllClick(TObject *Sender);
	void __fastcall EditorKeyDown(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall EditorKeyUp(TObject *Sender, WORD &Key, TShiftState Shift);
	void __fastcall EditorMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
          int X, int Y);
	void __fastcall FormShow(TObject *Sender);
	void __fastcall LoadScriptClick(TObject *Sender);
	void __fastcall SendScriptClick(TObject *Sender);
	void __fastcall EditorKeyPress(TObject *Sender, System::WideChar &Key);
private:	// User declarations
	void __fastcall ActivateCodeInsight();
    void __fastcall CreateCodeInsightMenu(String entity_name);
	void __fastcall ClearCodeInsightMenu();
	void __fastcall OpenCodeInsightMenu();
    String __fastcall ExtractFragmentLexemeFromLine(int line_ind, int cursor_pos);
	String __fastcall ReplaceFragmentLexemeInLine(const String &lexeme, int line_ind, int cursor_pos);
	int __fastcall FindLexemBorder(int line_ind, int cursor_pos);

public:		// User declarations
	__fastcall TScriptForm(TComponent* Owner);

    void __fastcall PPCodeInsightMenuClick(TObject *Sender);
};
//---------------------------------------------------------------------------
extern PACKAGE TScriptForm *ScriptForm;
//---------------------------------------------------------------------------
#endif
