/*!
Copyright 2019-2020 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef TAMEliThreadH
#define TAMEliThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
//---------------------------------------------------------------------------

#include "eli_interface.h"

class TAMEliThread : public TThread
{
private:
	double passed, interval;
	ELI_INTERFACE *iface;
	String script_path;
	bool logging;

protected:
	void __fastcall Execute();
public:
	__fastcall TAMEliThread(bool CreateSuspended);
	inline void __fastcall SetRunInterval(int val){if (val > 0){interval = val * 60000;}};

	__property ELI_INTERFACE *ELIInterface = {read = iface, write = iface};
	__property String ScriptPath = {read = script_path, write = script_path};
	__property bool Logging = {read = logging, write = logging};
};
//---------------------------------------------------------------------------
#endif
