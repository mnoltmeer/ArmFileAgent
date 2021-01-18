/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef ClientConfigLinksH
#define ClientConfigLinksH
//---------------------------------------------------------------------------

struct ClientConfigLink
{
  String IndexVZ;
  String StationID;
  String FileName;

  bool operator ==(ClientConfigLink lnk);
};

class ClientConfigLinks
{
private:
	std::vector<ClientConfigLink> FLinks;

	ClientConfigLink *ReadLinks(int ind);
	void WriteLinks(int ind, ClientConfigLink *lnk);

public:
	ClientConfigLinks(){};
    inline virtual ~ClientConfigLinks(){FLinks.clear();}

	void Add(const String &index, const String &station, const String &file);
	void Add(ClientConfigLink lnk);
	void Remove(int ind);
	void Remove(const String &index, const String &station, const String &file);
	int IndexOf(const String &index, const String &station, const String &file);
	int IndexOf(ClientConfigLink link);
	inline int Count(){return FLinks.size();}

	__property ClientConfigLink* Links[int ind] = {read = ReadLinks, write = WriteLinks};
};

class ClientConfigManager
{
private:
	ClientConfigLinks *FLinks;

	inline int GetCount(){return FLinks->Count();}

public:
	ClientConfigManager(){FLinks = new ClientConfigLinks();}
	ClientConfigManager(ClientConfigLinks *links);
    ClientConfigManager(const String &file);
	inline virtual ~ClientConfigManager(){if (FLinks) delete FLinks;}

	void AddLink(const String &index, const String &station, const String &file);
	void RemoveLink(int ind);
	void RemoveLink(const String &index, const String &station, const String &file);
	void RemoveLinks(const String &index, const String &station);
	void RemoveLinks(const String &index);
	ClientConfigLinks *GetLinks(const String &index, const String &station);
	ClientConfigLinks *GetLinks(const String &index, bool index_only);

	void SaveToFile(const String &file);
	void LoadFromFile(const String &file);

	__property int Count = {read = GetCount};
	__property ClientConfigLinks* Items = {read = FLinks};
};

#endif
