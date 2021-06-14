//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "..\..\work-functions\RecpOrganizer.h"
#include "RecpThread.h"
#include "AddGroup.h"
#include "Main.h"
#include "AddRecord.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TAddRecordForm *AddRecordForm;

RecipientItem *prev_grp, *prev_itm;

extern TRecpientItemCollection *AddrBook;
extern TRecpientCollectionThread *AddrBookChecker;
extern TAddGroupForm *AddGroupForm;
extern TServerForm *ServerForm;
//---------------------------------------------------------------------------
__fastcall TAddRecordForm::TAddRecordForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TAddRecordForm::ApplyClick(TObject *Sender)
{
  String name = NewGroupList->Items->Strings[NewGroupList->ItemIndex];

  RecipientItem *grp = AddrBook->FindGroup(name),
				*itm = AddrBook->FindItem(Station->Text);

  if (!prev_grp && !prev_itm) //додається новий запис
	{
	  if (itm)
		MessageBox(Handle, L"Такий запис вже існує", L"Помилка", MB_OK|MB_ICONERROR);
	  else if (grp)
		{
		  AddrBook->Add(grp->ID, grp->Node, Station->Text, Host->Text, Port->Text);
		  AddrBook->CreateSortedTree(ServerForm->AddrList);
		  AddrBookChecker->CollectionChanged = true;
		  grp->Node->Expand(true);

		  Close();
		}
	}
  else if (prev_grp && prev_itm) //змінюється існуючий запис
	{
	  if (itm && grp)
		{
		  if (prev_grp->ID != grp->ID) //запис був у групі але група змінилась
			{
			  AddrBook->Add(grp->ID, grp->Node, itm->Name, itm->Host, itm->Port);
			  AddrBook->Remove(prev_itm->ID); //видаляємо старий запис
			  AddrBook->CreateSortedTree(ServerForm->AddrList);
			  AddrBookChecker->CollectionChanged = true;
			  prev_grp->Node->Expand(true);

			  if (itm->Node == ServerForm->AddrList->Selected)
				ServerForm->ShowClientInfo(itm->Name, grp->Name, itm->Host, itm->Port);
			  else
				ServerForm->ClearClientInfo();

			  Close();
			}
		  else //запис був у групі, група не змінилась - редагуються параметри запису
			{
			  itm->Host = Host->Text;
              itm->Port = Port->Text;
			  AddrBook->CreateSortedTree(ServerForm->AddrList);
			  AddrBookChecker->CollectionChanged = true;
			  prev_grp->Node->Expand(true);

              if (itm->Node == ServerForm->AddrList->Selected)
				ServerForm->ShowClientInfo(itm->Name, grp->Name, itm->Host, itm->Port);
              else
				ServerForm->ClearClientInfo();

			  Close();
			}
		}
	  else if (grp)
		{
		  AddrBook->Add(grp->ID, grp->Node, Station->Text, Host->Text, Port->Text);
		  AddrBook->Remove(prev_itm->ID); //видаляємо старий запис
		  AddrBook->CreateSortedTree(ServerForm->AddrList);
		  AddrBookChecker->CollectionChanged = true;
		  prev_grp->Node->Expand(true);

		  Close();
        }
	}
}
//---------------------------------------------------------------------------

void __fastcall TAddRecordForm::ClearData()
{
  Station->Clear();
  Host->Clear();
  Port->Clear();
}
//---------------------------------------------------------------------------

void __fastcall TAddRecordForm::CancelClick(TObject *Sender)
{
  Close();
}
//---------------------------------------------------------------------------

void __fastcall TAddRecordForm::FormShow(TObject *Sender)
{
  Left = ServerForm->ClientWidth / 2 - ClientWidth / 2;
  Top = ServerForm->ClientHeight / 2 - ClientHeight / 2;

  NewGroupList->Clear();
  AddrBook->SelectGroups(NewGroupList->Items);

  prev_itm = AddrBook->FindItem(Station->Text);

  if (prev_itm)
	{
	  prev_grp = AddrBook->FindGroup(prev_itm->ParentNodeID);
	  NewGroupList->ItemIndex = NewGroupList->Items->IndexOf(prev_grp->Name);
	}
}
//---------------------------------------------------------------------------

void __fastcall TAddRecordForm::NewGroupClick(TObject *Sender)
{
  AddGroupForm->Show();
}
//---------------------------------------------------------------------------

void __fastcall TAddRecordForm::StationKeyPress(TObject *Sender, System::WideChar &Key)

{
  if (Key == 13)
    Apply->Click();
}
//---------------------------------------------------------------------------

