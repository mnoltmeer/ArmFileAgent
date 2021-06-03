/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef StatusCheckerH
#define StatusCheckerH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>

#include "RecpOrganizer.h"
//---------------------------------------------------------------------------
class TStatusCheckThread : public TThread
{
private:
	int FInterval;
	TRecpientItemCollection *FCollection;

    void __fastcall Check();

protected:
	void __fastcall Execute();

public:
	__fastcall TStatusCheckThread(bool CreateSuspended);

	__property TRecpientItemCollection *Collection = {read = FCollection, write = FCollection};
    __property int CheckInterval = {read = FInterval, write = FInterval};
};
//---------------------------------------------------------------------------
#endif