//---------------------------------------------------------------------------

#include <vcl.h>
#include <memory>
#pragma hdrstop

#include "Unit.h"
#include "ScriptEditor.h"
#include "ELISourceHighlighter.h"
#include "ELICodeInsight.h"
#include "..\..\work-functions\TCPRequester.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TScriptForm *ScriptForm;

TPopupMenu *CodeInsightPopup;
TColor CurrColor;

extern TAURAForm *AURAForm;
//---------------------------------------------------------------------------
__fastcall TScriptForm::TScriptForm(TComponent* Owner)
	: TForm(Owner)
{
  InitLexems();
  Editor->Font->Color = clWhite;
  CurrColor = clWhite;
  InitExprColors(ESH_DARK_THEME);
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::FormShow(TObject *Sender)
{
  SendMessage(Editor->Handle, WM_KEYUP, 0, NULL);
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::ActivateCodeInsight()
{
  try
	 {
	   String fragment = ExtractFragmentLexemeFromLine(Editor->CaretPos.Y, Editor->CaretPos.X);
	   CreateCodeInsightMenu(fragment);
	   OpenCodeInsightMenu();
	   HighlightSource(Editor, Editor->CaretPos.Y);
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("ActivateCodeInsight: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::CreateCodeInsightMenu(String entity_name)
{
  try
	 {
	   std::vector<Lexeme> *results;

	   try
		  {
			if (CodeInsightPopup)
			  delete CodeInsightPopup;

			CodeInsightPopup = new TPopupMenu(this);
            CodeInsightPopup->AutoHotkeys = maManual;

			results = GetResults(entity_name);

			for (int i = 0; i < results->size(); i++)
			   {
				 TMenuItem *menu_item = new TMenuItem(CodeInsightPopup);

				 menu_item->OnClick = PPCodeInsightMenuClick;
				 menu_item->Caption = results->at(i).Type +
                                      "  " +
									  results->at(i).Signature +
									  " : " +
									  results->at(i).Description;
				 menu_item->Hint = results->at(i).InsertText;

				 CodeInsightPopup->Items->Add(menu_item);
			   }
		  }
	   __finally {delete results;}
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("CreateCodeInsightMenu: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::ClearCodeInsightMenu()
{
  try
	 {
	   delete CodeInsightPopup;
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("ClearCodeInsightMenu: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::OpenCodeInsightMenu()
{
  try
	 {
	   TPoint cursor;
	   GetCaretPos(&cursor);
	   cursor = Editor->ClientToScreen(cursor);
	   cursor = this->ScreenToClient(cursor);
	   CodeInsightPopup->Popup(cursor.X, cursor.Y + 40);
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("OpenCodeInsightMenu: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

String __fastcall TScriptForm::ExtractFragmentLexemeFromLine(int line_ind, int cursor_pos)
{
  String res;

  try
	 {
	   res = Editor->Lines->Strings[line_ind]; //беремо рядок
	   res = res.SubString(1, cursor_pos); //обрізаємо до позиції курсору

	   int lex_border = FindLexemBorder(line_ind, cursor_pos);

       res = res.Delete(1, lex_border); //залишаємо саму лексему
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("ExtractFragmentFromLine: " + e.ToString());
	   res = "";
	 }

  return res;
}
//---------------------------------------------------------------------------

String __fastcall TScriptForm::ReplaceFragmentLexemeInLine(const String &lexeme,
															  int line_ind,
															  int cursor_pos)
{
  String res;

  try
	 {
	   res = Editor->Lines->Strings[line_ind]; //беремо рядок

	   int lex_border = res.LastDelimiter(lexeme[1]);

	   res = res.Delete(lex_border, cursor_pos - lex_border + 1); //видаляємо фрагмент лексеми з рядка
	   res = res.Insert(lexeme, lex_border); //вставляємо повну версію лексеми
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("ExtractFragmentFromLine: " + e.ToString());
	   res = "";
	 }

  return res;
}
//---------------------------------------------------------------------------

int __fastcall TScriptForm::FindLexemBorder(int line_ind, int cursor_pos)
{
  int res = 0;

  try
	 {
	   String operstr = Editor->Lines->Strings[line_ind]; //беремо рядок

	   for (int i = cursor_pos; i >= 1; i--)
		  {
			if ((operstr[i] == '_') || (operstr[i] == '#'))
			  {
				res = i - 1;
				break;
			  }
			else if (operstr[i] == '.')
			  {
				for (int j = i; j >= 1; j--)
				   {
					 if (operstr[j] == ' ')
					   break;
					 else if (operstr[j] == '&')
					   {
						 res = i - 1;
						 break;
                       }
				   }

                break;
              }
          }
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("FindLexemBorder: " + e.ToString());
	   res = -1;
	 }

  return res;
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::ScriptPathChange(TObject *Sender)
{
  if (FileExists(ScriptPath->Text))
	{
	  Editor->Lines->LoadFromFile(ScriptPath->Text, TEncoding::UTF8);
      SendMessage(Editor->Handle, WM_KEYUP, 0, NULL);
    }
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::PPCopyClick(TObject *Sender)
{
  Editor->CopyToClipboard();
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::PPCutClick(TObject *Sender)
{
  Editor->CutToClipboard();
  SendMessage(Editor->Handle, WM_KEYUP, 0, NULL);
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::PPPasteClick(TObject *Sender)
{
  Editor->PlainText = true;
  Editor->PasteFromClipboard();
  Editor->PlainText = false;
  SendMessage(Editor->Handle, WM_KEYUP, 0, NULL);
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::PPSelectAllClick(TObject *Sender)
{
  Editor->SelStart = 0;
  Editor->SelLength = Editor->Text.Length();
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::EditorKeyDown(TObject *Sender, WORD &Key, TShiftState Shift)

{
  if (Key == 32)
	{
	  if (Shift.Contains(ssCtrl))
		ActivateCodeInsight();
	}
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::EditorKeyUp(TObject *Sender, WORD &Key, TShiftState Shift)

{
  if (Key == 120)
	return;

  if (Key == 13) //enter
	{
	  LockWindowUpdate(Editor->Handle);
	  HighlightSource(Editor, Editor->CaretPos.Y - 1);
	  LockWindowUpdate(NULL);
	}
  else if (Key == 0) //пустий символ для активації підсвітки
	{
	  LockWindowUpdate(Editor->Handle);
	  HighlightSourceFull(Editor);
      LockWindowUpdate(NULL);
	}
  else if (Key == 190) //'.' для активації CodeInsight
    {
	  ActivateCodeInsight();
	}
  else if ((Key == 189) && Shift.Contains(ssShift)) //'_' для активації CodeInsight
    {
	  ActivateCodeInsight();
	}
  else if ((Key == 51) && Shift.Contains(ssShift)) //'#' для активації CodeInsight
	{
	  ActivateCodeInsight();
	}

  if (!(Shift.Contains(ssCtrl) || Shift.Contains(ssShift) ||
	  (Key == 16) || (Key == 17) || (Key == 18)))
	{
	  LockWindowUpdate(Editor->Handle);
	  HighlightSource(Editor, Editor->CaretPos.Y);
	  LockWindowUpdate(NULL);
	}
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::EditorKeyPress(TObject *Sender, System::WideChar &Key)
{
  switch (Key)
	{
	  case 22: //ctrl+v
		{
          SendMessage(Editor->Handle, WM_KEYUP, 0, NULL);
		  break;
		}

	  default: break;
	}
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::EditorMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y)
{
  if (Button == mbRight)
	{
	  if (Editor->SelText == "") //не вибрано текст - блокуємо пункти "Вирізати" та "Скопіювати"
		{
		  EditorPopup->Items->Items[0]->Enabled = false;
		  EditorPopup->Items->Items[1]->Enabled = false;
		}
	  else
		{
		  EditorPopup->Items->Items[0]->Enabled = true;
		  EditorPopup->Items->Items[1]->Enabled = true;
		}

	  if (Clipboard()->AsText == "") //нема тексту для вставки, блокуємо "Вставити"
		EditorPopup->Items->Items[2]->Enabled = false;
	  else
		EditorPopup->Items->Items[2]->Enabled = true;

	  EditorPopup->Popup(Left + Editor->Left + X + 18,
						 Top + Editor->Top + Y - 10);
	}
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::PPCodeInsightMenuClick(TObject *Sender)
{
  try
	 {
	   TMenuItem *menu = dynamic_cast<TMenuItem*>(Sender);

	   String lexem = ReplaceFragmentLexemeInLine(menu->Hint, Editor->CaretPos.Y, Editor->CaretPos.X);

	   if (lexem != "")
		 Editor->Lines->Strings[Editor->CaretPos.Y] = lexem;

	   HighlightSource(Editor, Editor->CaretPos.Y);
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("PPCodeInsightMenuClick: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::LoadScriptClick(TObject *Sender)
{
  try
	 {
	   if (OpenScriptDialog->Execute())
		 {
		   Editor->Lines->LoadFromFile(OpenScriptDialog->FileName, TEncoding::UTF8);

		   int ind = ScriptPath->Items->IndexOf(OpenScriptDialog->FileName);

		   if (ind < 0)
			 {
			   ScriptPath->Items->Add(OpenScriptDialog->FileName);
			   ScriptPath->ItemIndex = ScriptPath->Items->Count - 1;
			 }
		   else
			 ScriptPath->ItemIndex = ind;

		   SendMessage(Editor->Handle, WM_KEYUP, 0, NULL);
		 }
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("Відкриття файла скрипту: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TScriptForm::SendScriptClick(TObject *Sender)
{
  try
	 {
	   AURAForm->AddActionLog("Відправка скрипту до " +
							  AURAForm->Host->Text + ":" +
							  AURAForm->Port->Text);

	   auto ms = std::make_unique<TStringStream>(Editor->Text, TEncoding::UTF8, true);
	   auto sender = std::make_unique<TTCPRequester>(AURAForm->Host->Text,
													 AURAForm->Port->Text.ToInt());
	   ms->Position = 0;
	   sender->SendData(ms.get());

       Close();
	 }
  catch (Exception &e)
	 {
	   AURAForm->AddActionLog("Відправка скрипту: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------




