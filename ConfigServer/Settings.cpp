//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\MyFunc.h"
#include "Main.h"
#include "Settings.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TSettingsForm *SettingsForm;

extern TServerForm *ServerForm;
extern int ListenPort;
//---------------------------------------------------------------------------
__fastcall TSettingsForm::TSettingsForm(TComponent* Owner)
	: TForm(Owner)
{
  Parent = ServerForm;
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::FormShow(TObject *Sender)
{
  Left = ServerForm->ClientWidth / 2 - ClientWidth / 2;
  Top = ServerForm->ClientHeight / 2 - ClientHeight / 2;

  ServicePort->SetFocus();

  ReadSettings();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::ReadSettings()
{
  try
	 {
	   TRegistry *reg = new TRegistry();

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (reg->OpenKey("Software\\AFAConfigServer\\Form", false))
			  {
				if (reg->ValueExists("HideWindow"))
				  StartMinimised->Checked = reg->ReadBool("HideWindow");

                reg->CloseKey();
			  }

			ServicePort->Text = IntToStr(ListenPort);
		  }
	   __finally {delete reg;}

	   EnableAutoStart->Checked = CheckAppAutoStart("AFAConfigServer", FOR_CURRENT_USER);
	 }
  catch (Exception &e)
	 {
	   ServerForm->WriteLog("SettingsForm::ReadSettings: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::WriteSettings()
{
  try
	 {
       TRegistry *reg = new TRegistry();

	   try
		  {
			reg->RootKey = HKEY_CURRENT_USER;

			if (reg->OpenKey("Software\\AFAConfigServer\\Form", false))
			  {
				reg->WriteBool("HideWindow", StartMinimised->Checked);
				reg->CloseKey();
			  }

			ListenPort = ServicePort->Text.ToInt();
		  }
	   __finally {delete reg;}

	   if (EnableAutoStart->Checked)
		 AddAppAutoStart("AFAConfigServer", Application->ExeName, FOR_CURRENT_USER);
	   else
		 RemoveAppAutoStart("AFAConfigServer", FOR_CURRENT_USER);
	 }
  catch (Exception &e)
	 {
	   ServerForm->WriteLog("SettingsForm::WriteSettings: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::CancelClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TSettingsForm::ApplyClick(TObject *Sender)
{
  WriteSettings();
  Close();
}
//---------------------------------------------------------------------------

