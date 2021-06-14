/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef StatusCheckerH
#define StatusCheckerH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <memory>

#include "..\..\work-functions\TCPRequester.h"
#include "..\..\work-functions\RecpOrganizer.h"
//---------------------------------------------------------------------------
class TStatusCheckThread : public TThread
{
private:
	int FInterval;
	TRecpientItemCollection *FCollection;
	std::unique_ptr<TTCPRequester> FSender;

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
