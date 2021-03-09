/*!
Copyright 2020 Maxim Noltmeer (m.noltmeer@gmail.com)
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
	TRecpientItemCollection *ACollection;
	bool AChanged;
	int AInterval;

protected:
	void __fastcall Execute();
public:
	__fastcall TRecpientCollectionThread(bool CreateSuspended);

	__property TRecpientItemCollection *Collection = {read = ACollection, write = ACollection};
	__property bool CollectionChanged = {read = AChanged, write = AChanged};
	__property int CheckInterval = {read = AInterval, write = AInterval};
};
//---------------------------------------------------------------------------
#endif
