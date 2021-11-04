/*!
Copyright 2019-2021  Maxim Noltmeer (m.noltmeer@gmail.com)

This file is part of ELI IDE.

	ELI Extended IDE is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	ELI IDE is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with ELI IDE.  If not, see <http://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#pragma hdrstop

#include "..\work-functions\MyFunc.h"
#include "ELISourceHighlighter.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

wchar_t EndSymb[] = {'=', '+', '-', '*',
					 '/', '(', ')', ',',
					 '.', '$', '&', '#',
					 ':', '?', ' ', '\0',
                     '_', '\''
					 };
int EndSymbSz = 18;

const wchar_t *VARSYM = L"$";
const wchar_t *OBJSYM = L"&";
const wchar_t *STRSYM = L"'";

TColor HlDirect, HlExpr, HlVarSym, HlObjSym, HlComment, HlBrace, HlEndl, HlFunc;

static std::vector<ExprColor> ExpColors; //перелік динамічних лексем, очищується
										 //перед кожною операцією підсвітки
static std::vector<ExprColor> PreDefinedColors; //перелік попереднь визначених лексем

static std::vector<MarkedFragment> vecSelFragments; //вектор, куди додаються параметри фрагментів,
													//які треба розфарбувати

extern String UsedAppLogDir;
//---------------------------------------------------------------------------

bool IsEndSym(wchar_t symb)
{
  for (int i = 0; i < EndSymbSz; i++)
	 {
	   if (symb == EndSymb[i])
		 return true;
	 }

  return false;
}
//---------------------------------------------------------------------------

void InitExprColors(int theme_style)
{
  ExpColors.clear();

  if (theme_style == ESH_LIGHT_THEME) //світла тема
	{
	  HlDirect = clGreen;
	  HlExpr = clBlue;
	  HlVarSym = clMaroon;
	  HlObjSym = clFuchsia;
	  HlComment = clSkyBlue;
	  HlBrace = clBlue;
	  HlEndl =  clMaroon;
	  HlFunc = clTeal;
	}
  else //темна тема
	{
	  HlDirect = clLime;
	  HlExpr = clSkyBlue;
	  HlVarSym = clYellow;
	  HlObjSym = clFuchsia;
	  HlComment = clSilver;
	  HlBrace = clAqua;
	  HlEndl =  clYellow;
	  HlFunc = clSkyBlue;
	}

//директиви
   PreDefinedColors.push_back(ExprColor(L"#endl", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#begin", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#end", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#include", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#exit", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#procedure", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#drop procedure", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#make", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#run", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#class", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#modify class", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#property", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#method", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#public property", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#public method", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#drop property", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#drop method", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#drop class", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#return", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#varstack", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#funcstack", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#prmstack", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#objstack", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#clstack", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#procstack", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#frgstack", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#protect", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#trigger", HlDirect));
   PreDefinedColors.push_back(ExprColor(L"#drop trigger", HlDirect));

//зарезервовані слова та вирази
   PreDefinedColors.push_back(ExprColor(L"if", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"else", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"for", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"count", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"while", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"select", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"when", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"sym", HlExpr));
   PreDefinedColors.push_back(ExprColor(L"num", HlExpr));

//спеціальні символи
   PreDefinedColors.push_back(ExprColor(L"/", HlComment));
   PreDefinedColors.push_back(ExprColor(L"{", HlBrace));
   PreDefinedColors.push_back(ExprColor(L"}", HlBrace));
   PreDefinedColors.push_back(ExprColor(L";", HlEndl));
}
//---------------------------------------------------------------------------

String RemoveTabs(String fragment, int spaces_cnt)
{
  String spaces_remover;

  for (int i = 1; i <= spaces_cnt; i++)
	 spaces_remover += " ";

  fragment = ParseString(fragment, String(wchar_t(VK_TAB)), spaces_remover);

  return fragment;
}
//---------------------------------------------------------------------------

int HighlightSource(TRichEdit *src, int line_ind)
{
  int res = 0;

  try
	 {
       src->PlainText = false;
	   TPoint user_caret = src->CaretPos;
	   src->SelAttributes = src->DefAttributes;

	   MarkFragmentsInLine(src, line_ind);

	   for (int i = 0; i < vecSelFragments.size(); i++)
		  {
			src->SelStart = vecSelFragments[i].SelPos;
			src->SelLength = vecSelFragments[i].SelLength;
			src->SelAttributes->Color = vecSelFragments[i].SelColor;
		  }

	   src->CaretPos = user_caret;
	   src->SelAttributes->Color = src->Font->Color;

	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::HighlightSource: " + e.ToString());
	   res = -1;
     }

  return res;
}
//---------------------------------------------------------------------------

int HighlightSourceFull(TRichEdit *src)
{
  int res = 0;

  try
	 {
	   for (int i = 0; i < src->Lines->Count; i++)
		  {
			HighlightSource(src, i);
		  }
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::HighlightSourceFull: " + e.ToString());
	   res = -1;
	 }

  return res;
}
//---------------------------------------------------------------------------

void MarkFragmentsInLine(TRichEdit *src, int line_ind)
{
  try
	 {
	   ExpColors.clear();
	   vecSelFragments.clear();

	   FindComments(src, line_ind);
	   FindFunctions(src, line_ind);
	   FindProcedures(src, line_ind);
	   FindObjects(src, line_ind);
	   FindVariables(src, line_ind);

	   String text = src->Lines->Strings[line_ind];
	   unsigned int cnt_sym_before = 0;

	   for (int j = 0; j < line_ind; j++)
		  cnt_sym_before += src->Lines->Strings[j].Length();

	   src->SelStart = cnt_sym_before + line_ind;
	   src->SelLength = src->Lines->Strings[line_ind].Length();
	   src->SelAttributes->Color = src->Font->Color;

	   int pos, expr_len;

	   for (int i = 0; i < ExpColors.size(); i++)
		  {
			while (pos = text.Pos(ExpColors[i].Expression))
			  {
				expr_len = wcslen(ExpColors[i].Expression);

				MarkedFragment frg = MarkedFragment(line_ind,
													cnt_sym_before + line_ind + pos - 1,
													expr_len,
													ExpColors[i].Color);
				vecSelFragments.push_back(frg);
				text.Delete(pos, expr_len);
				text.Insert(CreateDummyString(expr_len), pos);
			  }
		  }

	   for (int i = 0; i < PreDefinedColors.size(); i++)
		  {
			while (pos = text.Pos(PreDefinedColors[i].Expression))
			  {
				expr_len = wcslen(PreDefinedColors[i].Expression);

				MarkedFragment frg = MarkedFragment(line_ind,
													cnt_sym_before + line_ind + pos - 1,
													expr_len,
													PreDefinedColors[i].Color);
				vecSelFragments.push_back(frg);
				text.Delete(pos, expr_len);
				text.Insert(CreateDummyString(expr_len), pos);
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::MarkFragmentsInLine: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

String CreateDummyString(int length)
{
  String res;

  try
	 {
	   for (int i = 0; i < length; i++)
		  res += '~';
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::CreateDummyString: " + e.ToString());
       res = "";
	 }

  return res;
}
//---------------------------------------------------------------------------

void AddFunctionHighlight(const wchar_t *func_name)
{
  AddLexemeHighlight(func_name, false, HlFunc);
}
//---------------------------------------------------------------------------

void AddLexemeHighlight(const wchar_t *text, bool case_sensitive, TColor highlight_color)
{
  if (LexemeExists(text))
	return;

  ExpColors.push_back(ExprColor(text, highlight_color));

  if (!case_sensitive)
	{
	  ExpColors.push_back(ExprColor(UpperCase(text).c_str(), highlight_color));
	  ExpColors.push_back(ExprColor(LowerCase(text).c_str(), highlight_color));
    }
}
//---------------------------------------------------------------------------

bool LexemeExists(const wchar_t *text)
{
  for (int i = 0; i < ExpColors.size(); i++)
	 {
	   if (_wcscmpi(ExpColors[i].Expression, text) == 0)
		 {
		   MessageBox(NULL, ExpColors[i].Expression, L"", MB_OK);
		   return true;

         }
	 }

  return false;
}
//---------------------------------------------------------------------------

void FindLexeme(TRichEdit *src, int line_ind, const wchar_t *st_symb, const wchar_t *end_symb,
				bool case_sensitive, TColor highlight_color)
{
  try
	 {
	   String text = src->Lines->Strings[line_ind];

	   if (text.Pos(st_symb))
		 {
		   int op = 0, cl = 0;

		   for (int i = 1; i <= text.Length(); i++)
			  {
				if (text[i] == st_symb[0])
				  op = i;
				else if (op)
				  {
					if (end_symb)
					  {
						if (end_symb[0] == text[i])
                          cl = i;
					  }
					else if (IsEndSym(text[i]))
					  cl = i;
				  }

				if (op && cl)
				  {
					String lexeme = text.SubString(op, cl - op);
					AddLexemeHighlight(lexeme.c_str(), case_sensitive, highlight_color);
					op = 0;
					cl = 0;
                  }
			  }
         }
	 }
  catch (Exception &e)
	 {
	   throw e;
	 }
}
//---------------------------------------------------------------------------

void FindFunctions(TRichEdit *src, int line_ind)
{
  try
	 {
	   FindLexeme(src, line_ind, L"_", NULL, false, HlFunc);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::FindFunctions: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void FindProcedures(TRichEdit *src, int line_ind)
{
  try
	 {
	   FindLexeme(src, line_ind, L":", NULL, false, HlFunc);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::FindProcedures: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void FindObjects(TRichEdit *src, int line_ind)
{
  try
	 {
	   FindLexeme(src, line_ind, OBJSYM, NULL, true, HlObjSym);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::FindObjects: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void FindVariables(TRichEdit *src, int line_ind)
{
  try
	 {
	   FindLexeme(src, line_ind, VARSYM, NULL, true, HlVarSym);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::FindVariables: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void FindComments(TRichEdit *src, int line_ind)
{
  try
	 {
	   FindLexeme(src, line_ind, L"//", L";", false, HlComment);
	 }
  catch (Exception &e)
	 {
	   SaveLogToUserFolder("IDE.log", "ELI", "ELISourceHighlighter::FindComments: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

