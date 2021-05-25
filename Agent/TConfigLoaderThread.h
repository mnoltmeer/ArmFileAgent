/*!
Copyright 2021 Maxim Noltmeer (m.noltmeer@gmail.com)
*/
//---------------------------------------------------------------------------

#ifndef TConfigLoaderThreadH
#define TConfigLoaderThreadH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Xml.adomxmldom.hpp>
#include <Xml.XMLDoc.hpp>
#include <Xml.xmldom.hpp>
#include <Xml.XMLIntf.hpp>
#include <Xml.Win.msxmldom.hpp>
//---------------------------------------------------------------------------

struct ReceivedFile
{
  String Name;
  int Size;
  int Version[4];
  TDateTime Changed;
};

class TConfigLoaderThread : public TThread
{
private:
	double FPassed, FInterval;
	bool FConnected;

	int __fastcall GetConfigurationFromServer();
    void __fastcall WorkWithFileList(const String &xml_str);
//робить імпорт переліку файлів з XML, повертає вектор відомостей про файли
	std::vector<ReceivedFile> __fastcall XMLImportFileList(String xml_text);
//порівнює відомості про віддалений файл з даними локального файлу
//повертає true, якщо файл потрібно завантажити
	bool __fastcall CompareLocalAndRemoteFiles(ReceivedFile &recvd_file);
	void __fastcall LoadFileFromConfigurationServer(ReceivedFile *file);
	void __fastcall ProcessLoadedFile(const String &file_name);
	void __fastcall RemoveUnnecessaryFiles(std::vector<ReceivedFile> *remote_files);
	void __fastcall CheckAndRunExistConfig(const String &remote_file);
	void __fastcall CheckAndRunExistModules();
    bool __fastcall IsModuleRunning(const String &file_name);
	void __fastcall StopRunningModule(const String &file_name);
    void __fastcall RunModule(const String &file_name);

protected:
	void __fastcall Execute();

public:
	__fastcall TConfigLoaderThread(bool CreateSuspended);

    __property bool Connected = {read = FConnected};
};
//---------------------------------------------------------------------------
#endif
