//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\RecpOrganizer.h"
#include "RecpThread.h"
#include "AddRecord.h"
#include "Main.h"
#include "AddGroup.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAddGroupForm *AddGroupForm;

extern TRecpientItemCollection *AddrBook;
extern TRecpientCollectionThread *AddrBookChecker;
extern TAddRecordForm *AddRecordForm;
extern TServerForm *ServerForm;
//---------------------------------------------------------------------------
__fastcall TAddGroupForm::TAddGroupForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TAddGroupForm::ApplyClick(TObject *Sender)
{
  RecipientItem *grp = AddrBook->FindGroup(Name->Text);

  if (!grp)
	{
	  AddrBook->Add(0,
					ServerForm->AddrList->Items->Add(ServerForm->AddrList->Selected,
					Name->Text), Name->Text);

	  AddrBook->CreateSortedTree(ServerForm->AddrList);
	  AddrBookChecker->CollectionChanged = true;

	  AddRecordForm->NewGroupList->Clear();
	  AddrBook->SelectGroups(AddRecordForm->NewGroupList->Items);

      Close();
	}
  else
	{
	  MessageBox(Handle, L"Таке ім'я вже існує", L"Помилка", MB_OK|MB_ICONERROR);
    }
}
//---------------------------------------------------------------------------

void __fastcall TAddGroupForm::FormShow(TObject *Sender)
{
  Left = ServerForm->ClientWidth / 2 - ClientWidth / 2;
  Top = ServerForm->ClientHeight / 2 - ClientHeight / 2;
  Name->Clear();
}
//---------------------------------------------------------------------------

