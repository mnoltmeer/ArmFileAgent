/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)

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
#include "ELICodeInsight.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

static std::vector<Lexeme> Lexems;

extern String UsedAppLogDir;

void InitLexems()
{
//функції
  AddLexeme("function",
			"_random(num pArea)",
			"returns a pseudo-random value from the pArea range",
			"_random()");

  AddLexeme("function",
			"_round(num pNumber, num pPrecision)",
			"returns the result of rounding. The pPrecision argument indicates rounding accuracy (0 to 2 decimal places)",
			"_round(,)");

  AddLexeme("function",
			"_int(num pNumber)",
			"returns the value of the pNumber argument cast to an integer. The fractional part is ignored",
			"_int()");

  AddLexeme("function",
			"_strlen(sym pStr)",
			"returns the length of the string",
			"_strlen()");

  AddLexeme("function",
			"_streq(sym pStr1, sym pStr2)",
			"compares strings, returns 1 if the strings are equal, 0 if not. Case sensitive",
			"_streq(,)");

  AddLexeme("function",
			"_istreq(sym pStr1, sym pStr2)",
			"compares strings, returns 1 if the strings are equal, 0 if not. Not case sensitive",
			"_istreq(,)");

  AddLexeme("function",
			"_substr(sym pTargetStr, num pPos, num pCount)",
			"returns a substring",
			"_substr(,,)");

  AddLexeme("function",
			"_return(sym pReturnVal)",
			"returns a value from a script",
			"_return()");

  AddLexeme("function",
			"_throw(sym pException)",
			"end the translating and add a custom exception with arbitrary text to the log",
			"_throw()");

  AddLexeme("function",
			"_free(sym pVarName)",
			"removes a variable from the stack, returns 1 if successful, otherwise returns 0",
			"_free()");

  AddLexeme("function",
			"_LoadObjStack(sym pFilePath, num pClear)",
			"loads a stack of objects from a file, returns 1 if successful, 0 otherwise. If pClear > 0 the stack of objects is cleared before loading from the file",
			"_LoadObjStack(,)");

  AddLexeme("function",
			"_SaveObjStack(sym pFilePath)",
			"saves a stack of objects to a file, returns 1 if successful, 0 otherwise. The path to the file can be either absolute or conditional (\".\\<File name>\")",
			"_SaveObjStack()");

  AddLexeme("function",
			"_SaveObjects(sym pFilePath, sym pCathegory)",
			"saves all objects from the stack with the specified category to the file. The path to the file can be either absolute or conditional (\".\\<File name>\")",
			"_SaveObjects(,)");

  AddLexeme("function",
			"_Run(sym pVarName)",
			"executes the code contained in a variable named pVarName. Corresponds to calling a bunch of directives #make and #run",
			"_Run()");

  AddLexeme("function",
			"_GetParamAsNum(sym pParam)",
			"returns the value of the pParam parameter from the parameter stack converted to a number",
			"_GetParamAsNum()");

  AddLexeme("function",
			"_GetParamAsStr(sym pParam)",
			"returns the value of the pParam parameter from the parameter stack converted to a string",
			"_GetParamAsStr(sym pParam)");

  AddLexeme("function",
			"_SetParam(sym pParam, sym pVal)",
			"adds a parameter with the name pParam and pVal value to the stack. If a parameter with that name is already on the stack, its value will be changed",
			"_SetParam(,)");

  AddLexeme("function",
			"_LoadFileToVar(sym pFile, sym pTarget)",
			"loads the contents of a text file into a variable called pTarget. The path to the file can be either absolute or conditional (\".\\<File name>\")",
			"_LoadFileToVar(,)");

  AddLexeme("function",
			"_SaveVarToFile(sym pTarget, sym pFile)",
			"saves the value of the variable to the specified file. The path to the file can be either absolute or conditional (\".\\<File name>\")",
			"_SaveVarToFile(,)");

  AddLexeme("function",
			"_SaveFragmentToFile(sym pTarget, sym pFile)",
			"saves the pending fragment with the label contained in the pTarget variable to the specified file",
			"_SaveFragmentToFile(,)");

  AddLexeme("function",
			"_GetConfig(sym pFile, sym pLine)",
			"loads the value of the pLine parameter from the pFile file. The file must have the structure <parameter> = <value>",
			"_GetConfig(,)");

  AddLexeme("function",
			"_SaveState()",
			"saves the current state of all ELI stacks to the state.log file in the ELI directory",
			"_SaveState()");

  AddLexeme("function",
			"_SaveVarStack(num pLevel)",
			"saves the state of the variable stack to the varstack.log file. If pLevel is 0, the global stack are saved, if 1 is local (that is, currently in use)",
			"_SaveVarStack()");

  AddLexeme("function",
			"_WriteOut(sym pStr)",
			"outputs the contents of the pStr argument to standard output",
			"_WriteOut()");

  AddLexeme("function",
			"_ReadIn(sym pVar)",
			"reads characters from the standard input stream and writes them to the pVar variable. The maximum number of characters to enter: 4096",
			"_ReadIn()");

  AddLexeme("function",
			"_System(sym pCmd)",
			"executes the Windows command contained in the pCmd argument. An analogue of the system() function in C ++",
			"_System()");

  AddLexeme("function",
			"_LastError()",
			"returns a description of the last error recorded",
			"_LastError()");

  AddLexeme("function",
			"_ConnectLib(sym pPath)",
			"connects an external dll library to the interpreter. Returns the handle of the connected library. In case of failure, returns -1",
			"_ConnectLib()");

  AddLexeme("function",
			"_FreeLib(num pHandle)",
			"frees the connected external library",
			"_FreeLib()");

  AddLexeme("function",
			"_ImportFunc(num pHandle, sym pExtName, sym pInName, sym pArgList)",
			"imports function named pExtName from the external library with the pHandle descriptor and adds it to the interpreter stack under the name pInName",
			"_ImportFunc()");

  AddLexeme("function",
			"_DebugIntoFile()",
			"enables logging of interpreter actions, makes output to a file",
			"_DebugIntoFile()");

  AddLexeme("function",
			"_DebugIntoScreen()",
			"enables logging of interpreter actions, makes output to standard output stream",
			"_DebugIntoScreen()");

  AddLexeme("function",
			"_StopDebug()",
			"disables logging of interpreter actions",
			"_StopDebug()");

  AddLexeme("function",
			"_sleep(num pMsec))",
			"paused translation for pMsec milliseconds",
			"_sleep()");

//директиви
  AddLexeme("directive",
			"#begin",
			"marks the beginning of the script. Mandatory",
			"#begin");

  AddLexeme("directive",
			"#end",
			"marks the end of the script. Mandatory",
			"#end");

  AddLexeme("directive",
			"#include <file name>",
			"includes the code contained in a separate external file in the script",
			"#include");

  AddLexeme("directive",
			"#exit",
			"force script termination",
			"#exit");

  AddLexeme("directive",
			"#procedure <procedure name> (<arguments>) {<procedure body>}",
			"declares a user procedure",
			"#procedure () {}");

  AddLexeme("directive",
			"#drop procedure <procedure name>",
			"deletes the user procedure",
			"#drop procedure");

  AddLexeme("directive",
			"#make <variable name> {<some code>}",
			"assigns the identifier of the delayed code fragment to the variable",
			"#make {}");

  AddLexeme("directive",
			"#run <variable name>",
			"executes a code fragment whose identifier is contained in a variable",
			"#run ");

  AddLexeme("directive",
			"#class <class name> {class body}",
			"creates a new class of objects",
			"#class {}");

  AddLexeme("directive",
			"#modify class <class name> {class body}",
			"modifies an existing class of objects",
			"#modify class {}");

  AddLexeme("directive",
			"#property <property name> = <value>",
			"adds a new private property to the selected class. Applicable only inside the class body code",
			"#property");

  AddLexeme("directive",
			"#public property <property name> = <value>",
			"adds a new public property to the selected class. Applicable only inside the class body code",
			"#public property");

  AddLexeme("directive",
			"#method <method name> (<arguments>) {<method body>}",
			"adds a new private method to the selected class. Applicable only inside the class body code",
			"#method () {}");

  AddLexeme("directive",
			"#public method <method name> (<arguments>) {<method body>}",
			"adds a new public method to the selected class. Applicable only inside the class body code",
			"#public method () {}");

  AddLexeme("directive",
			"#drop property <property name>",
			"removes a property from the class",
			"#drop property");

  AddLexeme("directive",
			"#drop method <method name>",
			"removes a method from the class",
			"#drop method");

  AddLexeme("directive",
			"#drop class <class name>",
			"removes the class of objects",
			"#drop class");

  AddLexeme("directive",
			"#return <value>",
			"returns a value from a class method",
			"#return");

  AddLexeme("directive",
			"#varstack",
			"outputs the contents of the variable stack. Used only as an argument to a function _WriteOut()",
			"#varstack");

  AddLexeme("directive",
			"#funcstack",
			"outputs the contents of the function stack. Used only as an argument to a function _WriteOut()",
			"#funcstack");

  AddLexeme("directive",
			"#prmstack",
			"output the contents of the parameter stack. Used only as an argument to a function _WriteOut()",
			"#prmstack");

  AddLexeme("directive",
			"#objstack",
			"outputs the contents of the objects stack. Used only as an argument to a function _WriteOut()",
			"#objstack");

  AddLexeme("directive",
			"#clstack",
			"outputs the contents of the class stack. Used only as an argument to a function _WriteOut()",
			"#clstack");

  AddLexeme("directive",
			"#procstack",
			"outputs the contents of the procedure stack. Used only as an argument to a function _WriteOut()",
			"#procstack");

  AddLexeme("directive",
			"#frgstack",
			"outputs the contents of the fragment stack. Used only as an argument to a function _WriteOut()",
			"#frgstack");

  AddLexeme("directive",
			"#endl",
			"outputs the end-of-line character (carriage return). Used only as an argument to a function _WriteOut()",
			"#endl");

  AddLexeme("directive",
			"#protect{<some code>}",
			"translates code safely. When an error occurs, it writes message to log and continues the translation",
			"#protect {}");

  AddLexeme("directive",
			"#trigger <condition> {<some code>} ",
			"declares a trigger. Trigger this\
a deferred piece of code that will be executed if the condition declared in the trigger is\
will be true. The trigger is declared once and checked for firing after\
translations of each line of the script. Trigger execution does not affect the broadcast in any way\
the main script, even if errors were received during the translation of the trigger body, or\
an exception was thrown",
			"#trigger {}");

  AddLexeme("directive",
			"#drop trigger <condition>",
			"drops trigger",
			"#drop trigger ");

//методи
  AddLexeme("object method",
			".Сreate(sym cathegory, sym ctor_args)",
			"creates a new object with the specified category. The ctor_args argument contains a list of arguments for the class constructor",
			".Create()");

  AddLexeme("object method",
			".Exist()",
			"checks if an object is present on the stack, returns 1 if success",
			".Exist()");

  AddLexeme("object method",
			".Have(sym prop_name)",
			"checks if a property exists on an object",
			".Have()");

  AddLexeme("object method",
			".Add(sym prop_name, sym val)",
			"adds property to object",
			".Add(,)");

  AddLexeme("object method",
			".Remove(sym prop_name)",
			"removes a property named prop_name",
			".Remove()");

  AddLexeme("object method",
			".Destroy()",
			"deletes the created object",
			".Destroy()");

  AddLexeme("object method",
			".Keep(sym prop_name, sym istrue)",
			"indicates whether the property should be stored on the stack. The istrue argument can be 1 or 0",
			".Keep(,)");

  AddLexeme("object method",
			".Save(sym prop_name, sym istrue)",
			"indicates whether to save this property to a file. The istrue argument can be 1 or 0",
			".Save(,)");

  AddLexeme("object method",
			".Execute(sym prop_name)",
			"executes a deferred code fragment whose identifier is contained in the prop_name property",
			".Execute()");

  AddLexeme("object method",
			".Show()",
			"displays all the properties of the object",
			".Show()");

  AddLexeme("object method",
			".GetName()",
			"returns the name of the object",
			".GetName()");

  AddLexeme("object method",
			".ExportIn(sym pPropNames, sym pPropVals)",
			"exports the properties of an object to two list objects with the given names",
			".ExportIn(,)");
}
//---------------------------------------------------------------------------

int AddLexeme(const String &type, const String &sign,
			  const String &descr, const String &insert_text)
{
  int res;

  try
	 {
	   Lexems.push_back(Lexeme(type, sign, descr, insert_text));
	   res = 1;
	 }
  catch (Exception &e)
	 {
	   res = 0;
	   SaveLogToUserFolder("IDE.log", "ELI", "ELICodeInsight::AddLexeme: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

std::vector<Lexeme> *GetResults(String fragment)
{
  std::vector<Lexeme> *res;

  try
	 {
	   res = new std::vector<Lexeme>;

	   for (int i = 0; i < Lexems.size(); i++)
		  {
			if (Lexems[i].Contain(fragment))
			  res->push_back(Lexems[i]);
		  }
	 }
  catch (Exception &e)
	 {
	   if (res)
		 delete res;

	   SaveLogToUserFolder("IDE.log", "ELI", "ELICodeInsight::GetResults: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

Lexeme *GetLexeme(const String &sign)
{
  Lexeme *res;

  try
	 {
	   for (int i = 0; i < Lexems.size(); i++)
		  {
			if (Lexems[i].Signature == sign)
			  {
				res = &Lexems[i];
                break;
              }
          }
	 }
  catch (Exception &e)
	 {
       res = NULL;
	   SaveLogToUserFolder("IDE.log", "ELI", "ELICodeInsight::GetLexeme: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------



