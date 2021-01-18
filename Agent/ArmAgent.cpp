//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("FirstStart.cpp", FirstStartForm);
USEFORM("Main.cpp", MainForm);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
        //перевірка на запущений екземпляр програми
		if (FindWindowW(NULL, L"Файловий агент АРМ ВЗ"))
		  Application->Terminate();

		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		TStyleManager::TrySetStyle("TabletDark");

		Application->CreateForm(__classid(TMainForm), &MainForm);
		Application->CreateForm(__classid(TFirstStartForm), &FirstStartForm);
		Application->Run();
	}
	catch (Exception &exception)
	{
		Application->ShowException(&exception);
	}
	catch (...)
	{
		try
		{
			throw Exception("");
		}
		catch (Exception &exception)
		{
			Application->ShowException(&exception);
		}
	}
	return 0;
}
//---------------------------------------------------------------------------
