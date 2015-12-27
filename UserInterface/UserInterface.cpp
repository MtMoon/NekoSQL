#include <string>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "UserInterface.h"

using namespace std;

//*******************ConsoleInterface************************
ConsoleInterface::ConsoleInterface()
{
	dm = new DataManager();
	sm = new SysManager(dm);
	im = new IndexManager(dm);
	errh = new ErrorHandler();
	qp = new QueryProcessor(dm, sm, im, errh);
	sa = new SynAnalyze(sm, qp, errh, im);
}

ConsoleInterface::~ConsoleInterface()
{
	delete sa;
	delete qp;
	delete errh;
	delete sm;
	delete dm;
}

void ConsoleInterface::Start()
{
	string cmd;
	ViewTable viewTable;
	string sysStr;
	bool hasView, hasStr;
	while (getline(cin, cmd))
	{
		if (sa->CmdAnalyze(cmd+";", viewTable, hasView, sysStr, hasStr))
		{
			if (hasView)
				qp->PrintViewTable(viewTable);
			if (hasStr)
				cout << sysStr << endl;
		}
	}
}

//*******************FileInterface************************
FileInterface::FileInterface(const string& rootDir_)
{
	dm = new DataManager();
	sm = new SysManager(dm);
	im = new IndexManager(dm);
	errh = new ErrorHandler();
	qp = new QueryProcessor(dm, sm, im, errh);
	sa = new SynAnalyze(sm, qp, errh, im);
	rootDir = rootDir_;
	dbName = "";
	type = -1;
}

FileInterface::~FileInterface()
{
	delete sa;
	delete qp;
	delete errh;
	delete sm;
	delete dm;
}

bool FileInterface::setRootDir(const string& rd)
{
	rootDir = rd;
	return true;
}

bool FileInterface::setDB(const string& db)
{
	dbName = db;
	return true;
}

bool FileInterface::setType(int type_)
{
	if (type_ != 0 && type_ != 1)
		return false;
	type = type_;
	return true;
}

bool FileInterface::isHead(string line) {
	bool ans = false;
	string keyWords[4];
	keyWords[0] = "INSERT";
	keyWords[1] = "DELETE";
	keyWords[2] = "UPDATE";
	keyWords[3] = "SELECT";

	for (int i=0; i<4; i++) {
		if (line.find(keyWords[i]) != string::npos && line.find("(") == string::npos) {
			ans = true;
			break;
		}
	}
	return ans;
}

bool FileInterface::processFile(const string& fname)
{
	if (type == -1)
	{
		cout << "FileInterface:type not defined." << endl;
		return false;
	}
	ifstream fin((rootDir+fname).c_str());
	cout << rootDir+fname << endl;

	ViewTable viewTable;
	bool hasView = false;
	string sysStr = "";
	bool hasStr = false;

	if (type == 0) { //sys
		string line = "";
		string cmd = "";

		while (getline(fin, line)) {
			//cout << "line#" << line << "#" << endl;
			if (line == "" && cmd != " ") {
				sa->CmdAnalyze(cmd, viewTable, hasView, sysStr, hasStr);
				if (hasStr) {
					cout << sysStr << endl;
				}

				if (hasView) {
					qp->PrintViewTable(viewTable);
				}
				cmd = "";
				hasView = false;
				sysStr = "";
				hasStr = false;

			} else {
				cmd += line;
			}
		}

	} else if (type == 1){ //data
		int flag = 0;
		if (dbName.length() == 0)
		{
			cout << "FileInterface:db not chosen." << endl;
			return false;
		}
		string tempStr = sa->SysAnalyze("use "+dbName+";" , flag);
		string line = "";
		string cmd = "";
		string cmdHead = "";
		while (getline(fin,line)) {

			if (line[line.length()-1] <= ' ') {
				line = line.substr(0, line.length()-1);
			}

			if ( isHead(line)) {
				cmdHead = line;
			} else {
				line[line.length()-1] = ';';
				cmd = cmdHead + line;
				cout << "cmd: " << cmd << endl;
				sa->CmdAnalyze(cmd, viewTable, hasView, sysStr, hasStr);
				if (hasStr) {
					cout << sysStr << endl;
				}

				if (hasView) {
					qp->PrintViewTable(viewTable);
				}
				cmd = "";
				hasView = false;
				sysStr = "";
				hasStr = false;
			}
		}


	}
	fin.clear();
	fin.close();
	return true;
}
