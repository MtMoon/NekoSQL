#ifndef USER_INTERFACE_H_
#define USER_INTERFACE_H_

#include "../bufmanager/BufPageManager.h"
#include "../datamanager/DataManager.h"
#include "../indexmanager/IndexManager.h"
#include "../sysmanager/SysManager.h"
#include "../ErrorHandler/ErrorHandler.h"
#include "../QueryProcessor/QueryProcessor.h"
#include "../SynAnalyze/SynAnalyze.h"
#include <string>

class ConsoleInterface
{
public:
	ConsoleInterface();
	~ConsoleInterface();

	void Start();
//private:
	DataManager* dm;
	IndexManager* im;
	SysManager* sm;
	ErrorHandler* errh;
	QueryProcessor* qp;
	SynAnalyze* sa;
};

class FileInterface
{
public:
	FileInterface(const std::string& rootDir_ = "sqlstatements/");
	~FileInterface();
	bool setRootDir(const std::string& rd);
	bool setDB(const std::string& db);
	bool setType(int type_);//0 sys process 1 data process
//private:
	DataManager* dm;
	IndexManager* im;
	SysManager* sm;
	ErrorHandler* errh;
	QueryProcessor* qp;
	SynAnalyze* sa;
	
	std::string rootDir;
	std::string dbName;
	int type;
	bool isHead(std::string line);
	bool processFile(const std::string& fname);
};


#endif
