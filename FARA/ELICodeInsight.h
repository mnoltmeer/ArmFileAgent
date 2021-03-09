/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)

This file is part of ELI IDE.

	ELI IDE is free software: you can redistribute it and/or modify
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

#ifndef ELICodeInsightH
#define ELICodeInsightH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.ComCtrls.hpp>

#include <vector>

#define ESH_LIGHT_THEME 1
#define ESH_DARK_THEME 0

struct Lexeme
{
  String Type;        //тип лексеми (directive, function, method etc)
  String Signature;   //сигнатура лексеми, типу _foo(num $x, sym $y)
  String Description; //опис та додаткова інфа
  String InsertText;  //текст, що вставляється в рядок скрипту при виборі лексеми

  Lexeme(String type, String sign, String descr, String intext)
  {
    Type = type;
	Signature = sign;
	Description = descr;
    InsertText = intext;
  }

  bool Contain(String fragment)
  {
	return ((fragment != "") && (Signature.SubString(1, fragment.Length()).UpperCase() == fragment.UpperCase()));
  }
};

void InitLexems();
std::vector<Lexeme> *GetResults(String fragment);
Lexeme *GetLexeme(const String &sign);
int AddLexeme(const String &type,
			  const String &sign,
			  const String &descr,
			  const String &insert_text);
#endif
