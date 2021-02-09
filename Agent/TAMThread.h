/*!
Copyright 2019-2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef TAMThreadH
#define TAMThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "TExchangeConnect.h"
//---------------------------------------------------------------------------

//стани роботи з'єднання
#define ES_WAIT 0
#define ES_SUCC_DL 1
#define ES_SUCC_UL 2
#define ES_SUCC_DL_UL 3
#define ES_NO_EXCHANGE -1
//#define ES_ERROR_STOP -2

class TAMThread : public TThread
{
private:
	TExchangeConnect *Conn;
	TTrayIcon *Icon;
	double passed, interval;

    void __fastcall ShowInfoStatus();
	void __fastcall ShowInfoNewDownload();
	void __fastcall ShowInfoNewUpload();
    void __fastcall Work();

protected:
	void __fastcall Execute();
public:
	__fastcall TAMThread(bool CreateSuspended);

	__property TExchangeConnect *Connection = {read = Conn, write = Conn};
	__property TTrayIcon *InfoIcon = {read = Icon, write = Icon};
	__property double PassedTime = {read = passed, write = passed};
	__property double WaitingInterval = {read = interval, write = interval};
};
//---------------------------------------------------------------------------
#endif
