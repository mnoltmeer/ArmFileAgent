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
	TTrayIcon *FIcon;

	TExchangeConnect *ReadConns(int ind);
	void WriteConns(int ind, TExchangeConnect *conn);

	TAMThread *ReadThreads(int ind);
	void WriteThreads(int ind, TAMThread *th);

	inline int FCount(){return FConns.size();}
	inline int FThCount(){return FThreadList.size();}

    void FDestroyConnections();

public:
	TConnectionManager(){};
	inline virtual ~TConnectionManager(){FDestroyConnections(); DeleteThreads();}

	TExchangeConnect *Add(TThreadSafeLog *Log);
	TExchangeConnect *Add(String cfg_file, TThreadSafeLog *Log);
	void Add(TExchangeConnect *conn);
	void Remove(int ind);
	void Remove(const String &caption);
	void Remove(TExchangeConnect *conn);
	int IndexOf(const String &caption);
	int IndexOf(const wchar_t *cfg_file);
	int IndexOf(TExchangeConnect *conn);
	int GenConnectionID();
	TExchangeConnect *Find(int id);
	TExchangeConnect *Find(const String &cfg_file);
	TAMThread *FindThread(unsigned int thread_id);
	TAMThread *AddThread(TExchangeConnect *conn);
	void DeleteThread(unsigned int id);
	void DeleteThreads();
	void Run(TExchangeConnect *conn);
	void Resume(TExchangeConnect *conn);
	void Stop(TExchangeConnect *conn);
	void Run(int id);
	void Resume(int id);
	void Stop(int id);
	void Run(const String &caption);
	void Resume(const String &caption);
	void Stop(const String &caption);

	__property int ConnectionCount = {read = FCount};
	__property int ThreadCount = {read = FThCount};
	__property TExchangeConnect* Connections[int ind] = {read = ReadConns, write = WriteConns};
	__property TAMThread* Threads[int ind] = {read = ReadThreads, write = WriteThreads};
    __property TTrayIcon *InfoIcon = {read = FIcon, write = FIcon};
};

#endif
