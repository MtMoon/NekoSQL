/*
 * FileInterface.cpp
 *
 *  Created on: 2015-12-26
 *      Author: yxy
 */

#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include "SynAnalyze/SynAnalyze.h"

using namespace std;


/*bool isHead(string line) {
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

int main() {

	ifstream fin("sqlstatements/create.sql");
	int type = 0; //0 sys process 1 data process

	DataManager* dm = new DataManager();
	IndexManager* im = new IndexManager(dm);
	SysManager* sm = new SysManager(dm);
	ErrorHandler* errh = new ErrorHandler();
	QueryProcessor* qp = new QueryProcessor(dm, sm, errh);
	SynAnalyze* sa = new SynAnalyze(sm, qp, errh, im);

	ViewTable viewTable;
	bool hasView = false;
	string sysStr = "";
	bool hasStr = false;

	if (type == 0) { //sys
		string line = "";
		string cmd = "";

		while (getline(fin, line)) {
			if (line == "") {
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
		string tempStr = sa->SysAnalyze("use orderDB;" , flag);
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



	return 0;

}*/




