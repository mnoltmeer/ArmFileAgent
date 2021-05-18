/*!
Copyright 2020-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------
#ifndef RecpThreadH
#define RecpThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>

#include "RecpOrganizer.h"
//---------------------------------------------------------------------------
class TRecpientCollectionThread : public TThread
{
private:
	TRecpientItemCollection *FCollection;
	bool FChanged;
	int FInterval;

protected:
	void __fastcall Execute();
public:
	__fastcall TRecpientCollectionThread(bool CreateSuspended);

	__property TRecpientItemCollection *Collection = {read = FCollection, write = FCollection};
	__property bool CollectionChanged = {read = FChanged, write = FChanged};
	__property int CheckInterval = {read = FInterval, write = FInterval};
};
//---------------------------------------------------------------------------
#endif
