/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef UpdateThreadH
#define UpdateThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
//---------------------------------------------------------------------------
class TUpdateThread : public TThread
{
private:
	String FAgentPath;
	String FUpdatePath;

    void __fastcall TerminateAgent();
	bool __fastcall NeedToUpdateAgent();
	void __fastcall UpdateAgent();

protected:
	void __fastcall Execute();
public:
	__fastcall TUpdateThread(const String &agent_path, const String &upd_path);

    void __fastcall TryToUpdateAgent();
};
//---------------------------------------------------------------------------
#endif
