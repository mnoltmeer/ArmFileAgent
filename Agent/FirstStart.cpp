//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "Main.h"
#include "FirstStart.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFirstStartForm *FirstStartForm;

extern TMainForm *MainForm;
extern String StationID, IndexVZ, ConfigServerHost, LogPath;
extern int RemAdmPort, ConfigServerPort;
bool used_migration;
//---------------------------------------------------------------------------
__fastcall TFirstStartForm::TFirstStartForm(TComponent* Owner)
	: TForm(Owner)
{

}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::FormShow(TObject *Sender)
{
  MigrationInitPanel->Hide();
  ManualInitPanel->Show();
  LbStationID->Caption = GetPCName();
  used_migration = false;
}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::InitKindClick(TObject *Sender)
{
  if (InitKind->Buttons[0]->Checked)
	{
	  MigrationInitPanel->Hide();
	  ManualInitPanel->Show();
	}
  else if (InitKind->Buttons[1]->Checked)
	{
	  MigrationInitPanel->Show();
	  ManualInitPanel->Hide();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::ApplyClick(TObject *Sender)
{
  if (CfgServerHost->Text == "")
	MessageBox(Application->Handle,
			   L"�� ������� ������ ������� ������������",
			   L"�����!",
			   MB_OK|MB_ICONWARNING);
  else if (CfgServerPort->Text == "")
	MessageBox(Application->Handle,
			   L"�� ������� ���� ������� ������������",
			   L"�����!",
			   MB_OK|MB_ICONWARNING);
  else if (Index->Text == "")
	MessageBox(Application->Handle,
			   L"�� ������� ������ ���",
			   L"�����!",
			   MB_OK|MB_ICONWARNING);
  else if (RemoteAdminPort->Text == "")
	MessageBox(Application->Handle,
			   L"�� ������� ���� ��� ����������� �������������",
			   L"�����!",
			   MB_OK|MB_ICONWARNING);
  else if (LbStationID->Caption == "")
	MessageBox(Application->Handle,
			   L"�� ������� ��������� ID �������",
			   L"�����!",
			   MB_OK|MB_ICONWARNING);
  else
	{
	  StationID = LbStationID->Caption;
	  IndexVZ = Index->Text;
	  ConfigServerHost = CfgServerHost->Text;
	  ConfigServerPort = CfgServerPort->Text.ToInt();
	  RemAdmPort = RemoteAdminPort->Text.ToInt();

	  if (EnableAutoStart->Checked)
		{
		  if (!AddAppAutoStart("ArmFileAgent", Application->ExeName, AllUsersAutoStart->Checked))
            MessageBox(Application->Handle,
			   L"�� ������� �������� ����� � ����� ��� �����������",
			   L"�����!",
			   MB_OK|MB_ICONWARNING);
		}

	  if (AllUsersAutoStart->Checked && CheckAppAutoStart("ArmFileAgent", FOR_CURRENT_USER))
		{
		  if (!RemoveAppAutoStart("ArmFileAgent", FOR_CURRENT_USER))
            MessageBox(Application->Handle,
			   L"�� ������� �������� ����� ����������� � ������ HKCU",
			   L"�����!",
			   MB_OK|MB_ICONWARNING);
		}

	  if (ManageFirewall->Checked)
        MainForm->AddFirewallRule();

	  MainForm->WriteSettings();

	  if (used_migration)
		{
          ShutdownProcessByExeName("ArmMngr.exe");

		  if (CheckAppAutoStart("ArmFileManager", FOR_ALL_USERS))
			RemoveAppAutoStart("ArmFileManager", FOR_ALL_USERS);
		  else if (CheckAppAutoStart("ArmFileManager", FOR_CURRENT_USER))
			RemoveAppAutoStart("ArmFileManager", FOR_CURRENT_USER);
        }

      //StartProcessByExeName(Application->ExeName);
	  Close();
      MainForm->StartApplication();
	  //MainForm->Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::CancelClick(TObject *Sender)
{
  if (MessageBox(Application->Handle,
				 L"����������� �� ���������. �� �������, �� ������ �����?",
				 L"ϳ����������� 䳿!",
				 MB_YESNO|MB_ICONWARNING) == mrYes)
	{
	  Close();
	  MainForm->Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::EnableAutoStartClick(TObject *Sender)
{
  if (EnableAutoStart->Checked)
	AllUsersAutoStart->Enabled = true;
  else
	{
	  AllUsersAutoStart->Enabled = false;
	  AllUsersAutoStart->Checked = false;
	}
}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::OpenManagerCfgClick(TObject *Sender)
{
  try
	 {
	   if (OpenCfgDialog->Execute())
		 {
		   SaveLog(LogPath + "\\init.log",
				   "��������� ������ ������� � ����� " + OpenCfgDialog->FileName);

		   String index, port;
		   bool autorun, autorun_all;

		   int rp;

		   rp = ReadParameter(OpenCfgDialog->FileName, "IndexVZ", &index, TT_TO_STR);

		   if (rp != RP_OK)
			 {
			   index = "";

			   MessageBox(Application->Handle,
						  L"������� ������� ��������� IndexVZ",
						  L"�����!",
						  MB_OK|MB_ICONWARNING);
			 }

		   rp = ReadParameter(OpenCfgDialog->FileName, "RemAdmPort", &port, TT_TO_STR);

		   if (rp != RP_OK)
			 {
			   port = "";

			   MessageBox(Application->Handle,
						  L"������� ������� ��������� RemAdmPort",
						  L"�����!",
						  MB_OK|MB_ICONWARNING);
			 }

		   rp = ReadParameter(OpenCfgDialog->FileName, "EnableAutoStart", &autorun, TT_TO_BOOL);

		   if (rp != RP_OK)
			 {
			   autorun = false;

			   MessageBox(Application->Handle,
						  L"������� ������� ��������� EnableAutoStart",
						  L"�����!",
						  MB_OK|MB_ICONWARNING);
			 }

		   rp = ReadParameter(OpenCfgDialog->FileName, "AutoStartForAllUsers", &autorun_all, TT_TO_BOOL);

		   if (rp != RP_OK)
			 {
               autorun_all = false;

			   MessageBox(Application->Handle,
						  L"������� ������� ��������� AutoStartForAllUsers",
						  L"�����!",
						  MB_OK|MB_ICONWARNING);
			 }

		   MigrIndexVZ->Caption = index;
		   MigrRemAdmPort->Caption = port;

		   if (autorun)
			 MigrAutoStart->Caption = "���";
		   else
			 MigrAutoStart->Caption = "�";

		   if (autorun_all)
			 MigrAutoStartForAll->Caption = "���";
		   else
			 MigrAutoStartForAll->Caption = "�";

		   ManagerCfgPath->Text = OpenCfgDialog->FileName;
		   Index->Text = index;
           RemoteAdminPort->Text = port;
		   EnableAutoStart->Checked = autorun;
		   AllUsersAutoStart->Checked = autorun_all;

		   used_migration = true;
		 }
	 }
  catch (Exception &e)
	 {
	   SaveLog(LogPath + "\\init.log", "������� �������: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

