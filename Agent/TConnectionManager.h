//---------------------------------------------------------------------------

#ifndef TConnectionManagerH
#define TConnectionManagerH
//---------------------------------------------------------------------------

#include "TExchangeConnect.h"
#include "TAMThread.h"

class TConnectionManager
{
private:
	std::vector<TExchangeConnect*> FConns;
	std::vector<TAMThread*> FThreadList;

	TExchangeConnect *ReadConns(int ind);
	void WriteConns(int ind, TExchangeConnect *conn);

	TAMThread *ReadThreads(int ind);
	void WriteThreads(int ind, TAMThread *th);

	inline int FCount(){return FConns.size();}

    int FDestroyConnections();

public:
	TConnectionManager(){};
	inline virtual ~TConnectionManager(){FDestroyConnections()}

	TExchangeConnect *Add(int ID, TTrayIcon *Icon, TThreadSafeLog *Log);
	TExchangeConnect *Add(String cfg_file, int ID, TTrayIcon *Icon, TThreadSafeLog *Log);
	void Add(TExchangeConnect *conn);
	void Remove(int ind);
	void Remove(const String &caption);
	int IndexOf(const String &caption);
	int IndexOf(const wchar_t *cfg_file);
	int IndexOf(TExchangeConnect *conn);
	int GenConnectionID();
	TExchangeConnect *Find(int id);
	TAMThread *FindThread(unsigned int thread_id);
	void DeleteThread(unsigned int id);
	void DeleteThreads();
	void Run(TExchangeConnect *conn);
	void Resume(TExchangeConnect *conn);
	void Stop(TExchangeConnect *conn);
	void Run(int id);
	void Resume(int id));
	void Stop(int id));
	void Run(const String &caption);
	void Resume(const String &caption);
	void Stop(const String &caption);


	__property int Count = {read = FCount};
	__property TExchangeConnect* Connections[int ind] = {read = ReadConns, write = WriteConns};
	__property TAMThread* Threads[int ind] = {read = ReadThreads, write = WriteThreads};
};

#endif
