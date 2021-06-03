//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
//---------------------------------------------------------------------------
#include <Vcl.Styles.hpp>
#include <Vcl.Themes.hpp>
USEFORM("Settings.cpp", SettingsForm);
USEFORM("Main.cpp", ServerForm);
USEFORM("AddGroup.cpp", AddGroupForm);
USEFORM("AddRecord.cpp", AddRecordForm);
//---------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	try
	{
		//�������� �� ��������� ��������� ��������
		if (FindWindowW(NULL, L"������ ������������ ��������� ������ ��� ��"))
		  Application->Terminate();

		Application->Initialize();
		Application->MainFormOnTaskBar = true;
		TStyleManager::TrySetStyle("Windows10 SlateGray");
		Application->CreateForm(__classid(TServerForm), &ServerForm);
		Application->CreateForm(__classid(TAddRecordForm), &AddRecordForm);
		Application->CreateForm(__classid(TAddGroupForm), &AddGroupForm);
		Application->CreateForm(__classid(TSettingsForm), &SettingsForm);
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
