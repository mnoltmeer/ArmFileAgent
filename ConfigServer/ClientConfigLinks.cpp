/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#pragma hdrstop
#include "..\..\work-functions\MyFunc.h"
#include "ClientConfigLinks.h"
//---------------------------------------------------------------------------

bool ClientConfigLink::operator ==(ClientConfigLink lnk)
{
  if ((lnk.IndexVZ == IndexVZ) && (lnk.StationID == StationID) && (lnk.FileName == FileName))
	return true;
  else
    return false;
}
//---------------------------------------------------------------------------

ClientConfigLink *ClientConfigLinks::ReadLinks(int ind)
{
  if (ind < 0)
	throw new Exception("ClientConfigLinks::Links: Out of bounds!");
  else if (ind < FLinks.size())
	return &FLinks[ind];
  else
	{
	  throw new Exception("ClientConfigLinks::Links: Out of bounds!");
    }
}
//---------------------------------------------------------------------------

void ClientConfigLinks::WriteLinks(int ind, ClientConfigLink *lnk)
{
  if (ind < 0)
	throw new Exception("ClientConfigLinks::Links: Out of bounds!");
  else if (ind < FLinks.size())
	FLinks[ind] = *lnk;
  else
	throw new Exception("ClientConfigLinks::Links: Out of bounds!");
}
//---------------------------------------------------------------------------

void ClientConfigLinks::Add(const String &index, const String &station, const String &file)
{
  try
	 {
	   ClientConfigLink lnk;

	   lnk.IndexVZ = index;
	   lnk.StationID = station;
	   lnk.FileName = file;

	   FLinks.push_back(lnk);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigLinks::Add: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigLinks::Add(ClientConfigLink lnk)
{
  try
	 {
	   FLinks.push_back(lnk);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigLinks::Add: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigLinks::Remove(int ind)
{
  try
	 {
	   FLinks.erase(FLinks.begin() + ind);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigLinks::Remove: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigLinks::Remove(const String &index, const String &station, const String &file)
{
  try
	 {
	   FLinks.erase(FLinks.begin() + IndexOf(index, station, file));
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigLinks::Remove: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

int ClientConfigLinks::IndexOf(const String &index, const String &station, const String &file)
{
  int res;

  try
	 {
	   ClientConfigLink lnk;

	   lnk.IndexVZ = index;
	   lnk.StationID = station;
	   lnk.FileName = file;

       res = IndexOf(lnk);
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   throw new Exception("ClientConfigLinks::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

int ClientConfigLinks::IndexOf(ClientConfigLink link)
{
  int res;

  try
	 {
       for (int i = 0; i < FLinks.size(); i++)
		  {
			if (FLinks[i] == link)
			  {
				res = i;
				break;
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   res = -1;
	   throw new Exception("ClientConfigLinks::IndexOf: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

ClientConfigManager::ClientConfigManager(ClientConfigLinks *links)
{
  if (FLinks) delete FLinks;

  FLinks = links;
}
//---------------------------------------------------------------------------

ClientConfigManager::ClientConfigManager(const String &file)
{
  FLinks = new ClientConfigLinks();

  LoadFromFile(file);
}
//---------------------------------------------------------------------------

void ClientConfigManager::AddLink(const String &index, const String &station, const String &file)
{
  try
	 {
	   FLinks->Add(index, station, file);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigManager::AddLink: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigManager::RemoveLink(int ind)
{
  try
	 {
	   FLinks->Remove(ind);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigManager::RemoveLink: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigManager::RemoveLink(const String &index, const String &station, const String &file)
{
  try
	 {
	   FLinks->Remove(index, station, file);
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigManager::RemoveLink: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigManager::RemoveLinks(const String &index, const String &station)
{
  try
	 {
	   int ind = 0;

	   while (ind < Count)
		 {
		   if ((FLinks->Links[ind]->IndexVZ == index) &&
			   (FLinks->Links[ind]->StationID == station))
			 {
			   RemoveLink(ind);
			 }
		   else
			 ind++;
		 }
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigManager::RemoveLinks: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigManager::RemoveLinks(const String &index)
{
  try
	 {
	   int ind = 0;

	   while (ind < Count)
		 {
		   if (FLinks->Links[ind]->IndexVZ == index)
			 RemoveLink(ind);
		   else
			 ind++;
         }
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigManager::RemoveLinks: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

ClientConfigLinks* ClientConfigManager::GetLinks(const String &index, const String &station)
{
  ClientConfigLinks* res;

  try
	 {
	   res = new ClientConfigLinks();

	   for (int i = 0; i < FLinks->Count(); i++)
		  {
			if ((FLinks->Links[i]->IndexVZ == index) && (FLinks->Links[i]->StationID == station))
			  res->Add(*FLinks->Links[i]);
		  }
	 }
  catch (Exception &e)
	 {
	   if (res) delete res;

	   res = NULL;
	   throw new Exception("ClientConfigManager::GetLinks: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

ClientConfigLinks* ClientConfigManager::GetLinks(const String &index, bool index_only)
{
  ClientConfigLinks* res;

  try
	 {
	   res = new ClientConfigLinks();

	   for (int i = 0; i < FLinks->Count(); i++)
		  {
			if (FLinks->Links[i]->IndexVZ == index)
			  {
				if (index_only && (FLinks->Links[i]->StationID == ""))
				  res->Add(*FLinks->Links[i]);
				else if (!index_only)
				  res->Add(*FLinks->Links[i]);
			  }
		  }
	 }
  catch (Exception &e)
	 {
	   if (res) delete res;

	   res = NULL;
	   throw new Exception("ClientConfigManager::GetLinks: " + e.ToString());
	 }

  return res;
}
//---------------------------------------------------------------------------

void ClientConfigManager::LoadFromFile(const String &file)
{
  try
	 {
	   TStringList *loader = new TStringList();
	   TStringList *lst = new TStringList();

	   try
		  {
			loader->LoadFromFile(file);

			for (int i = 0; i < loader->Count; i++)
			   {
				 lst->Clear();
				 StrToList(lst, loader->Strings[i], ";");

				 AddLink(lst->Strings[0], lst->Strings[1], lst->Strings[2]);
			   }
		  }
	   __finally {delete loader; delete lst;}
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigManager::LoadFromFile: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

void ClientConfigManager::SaveToFile(const String &file)
{
  try
	 {
       TStringList *saver = new TStringList();

	   try
		  {
			for (int i = 0; i < Count; i++)
			   {
				 saver->Add(FLinks->Links[i]->IndexVZ + ";" +
							FLinks->Links[i]->StationID + ";" +
							FLinks->Links[i]->FileName);
			   }

			saver->SaveToFile(file);
		  }
	   __finally {delete saver;}
	 }
  catch (Exception &e)
	 {
	   throw new Exception("ClientConfigManager::SaveToFile: " + e.ToString());
	 }
}
//---------------------------------------------------------------------------

#pragma package(smart_init)
