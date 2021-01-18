//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "FirstStart.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TFirstStartForm *FirstStartForm;

extern String StationID, IndexVZ, ConfigServerHost, LogPath;

extern int RemAdmPort, ConfigServerPort;
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
			   L"Не вказано адресу сервера конфігурацій",
			   L"Увага!",
			   MB_OK|MB_ICONWARNING);
  else if (CfgServerPort->Text == "")
	MessageBox(Application->Handle,
			   L"Не вказано порт сервера конфігурацій",
			   L"Увага!",
			   MB_OK|MB_ICONWARNING);
  else if (Index->Text == "")
	MessageBox(Application->Handle,
			   L"Не вказано індекс ВПЗ",
			   L"Увага!",
			   MB_OK|MB_ICONWARNING);
  else if (RemoteAdminPort->Text == "")
	MessageBox(Application->Handle,
			   L"Не вказано порт для віддаленного адміністрування",
			   L"Увага!",
			   MB_OK|MB_ICONWARNING);
  else if (LbStationID->Caption == "")
	MessageBox(Application->Handle,
			   L"Не вдалося визначити ID станції",
			   L"Увага!",
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
			   L"Не вдалося створити запис у реєстрі для автозапуску",
			   L"Увага!",
			   MB_OK|MB_ICONWARNING);
		}

	  if (AllUsersAutoStart->Checked && CheckAppAutoStart("ArmFileAgent", FOR_CURRENT_USER))
		{
		  if (!RemoveAppAutoStart("ArmFileAgent", FOR_CURRENT_USER))
            MessageBox(Application->Handle,
			   L"Не вдалося видалити запис автозапуску з реєстру HKCU",
			   L"Увага!",
			   MB_OK|MB_ICONWARNING);
		}

	  AddFirewallRule();
      WriteSettings();

      Close();
	}
}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::CancelClick(TObject *Sender)
{
  if (MessageDlg("Ініціалізацію не завершено. Ви впевнені, що хочете вийти?",
				 mtConfirmation,
				 mbYesNo, 0) == mrYes)
	{
	  Close();
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

void __fastcall TFirstStartForm::AddFirewallRule()
{
  try
	 {
	   AnsiString cmd;

	   if (system("netsh advfirewall firewall show rule name=\"ArmAgent\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=in action=allow program=\"" + Application->ExeName + "\" enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"ArmAgent\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"ArmAgent\" dir=out action=allow program=\"" + Application->ExeName + "\" enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FARA\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FARA\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FARA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FAGRA\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FAGRA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort + 1) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FAGRA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort + 1) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FAGRA\" dir=in action=allow protocol=TCP localport=" + IntToStr(RemAdmPort + 1) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FAGRA\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FAGRA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort + 1) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FAGRA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort + 1) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FAGRA\" dir=out action=allow protocol=TCP localport=" + IntToStr(RemAdmPort + 1) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FACS\" dir=in") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=in action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=in action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=in action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		   Sleep(100);
		 }

	   if (system("netsh advfirewall firewall show rule name=\"FACS\" dir=out") != 0)
		 {
		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=out action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=domain";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=out action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=private";
		   system(cmd.c_str());
		   Sleep(100);

		   cmd = "netsh advfirewall firewall add rule name=\"FACS\" dir=out action=allow protocol=TCP localport=" + IntToStr(ConfigServerPort) + " enable=yes profile=public";
		   system(cmd.c_str());
		 }
	 }
  catch (Exception &e)
	 {
	   SaveLog(LogPath + "\\init.log", "Створення правила для файрволу: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TFirstStartForm::WriteSettings()
{
  try
	 {
       TRegistry *reg = new TRegistry();

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (reg->OpenKey("Software\\ArmFileAgent", false))
			  {
				if (ConfigServerPort)
				  reg->WriteInteger("ConfigServerPort", ConfigServerPort);

				if (RemAdmPort)
				  reg->WriteInteger("RemAdmPort", RemAdmPort);

				if (ConfigServerHost != "")
				  reg->WriteString("ConfigServerHost", ConfigServerHost);

				if (StationID != "")
				  reg->WriteString("StationID", StationID);

				if (IndexVZ != "")
				  reg->WriteString("IndexVZ", IndexVZ);

				reg->CloseKey();
			  }
		  }
	   __finally {delete reg;}
	 }
  catch (Exception &e)
	 {
	   SaveLog(LogPath + "\\init.log", "Запис налаштувань до реєстру: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

