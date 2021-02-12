/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef GuardThreadH
#define GuardThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
//---------------------------------------------------------------------------
class TGuardThread : public TThread
{
private:
	String FAgentPath;
    String FAgentName;
	double FPassed, FInterval;

	void __fastcall CheckAgent();

protected:
	void __fastcall Execute();
public:
	__fastcall TGuardThread(bool CreateSuspended, const String agent_path);
};
//---------------------------------------------------------------------------
#endif
