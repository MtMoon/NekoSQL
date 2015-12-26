/*
 * SynAnalyzeTest.cpp
 *
 *  Created on: 2015-12-26
 *      Author: yxy
 */

#include <vector>
#include <iostream>
#include <cstdlib>
#include "SynAnalyze/SynAnalyze.h"

int main() {

	DataManager* dm = new DataManager();
	SysManager* sm = new SysManager(dm);
	ErrorHandler* errh = new ErrorHandler();
	QueryProcessor* qp = new QueryProcessor(dm, sm, errh);
	SynAnalyze* sa = new SynAnalyze(sm, qp, errh);

	string cmd = "";

	while (getline(cin, cmd)) {
		if ("quit" == cmd) {
			break;
		}

		int flag = -1;
		flag = -1;
		string ans = sa->SysAnalyze(cmd,flag);
		cout << ans << endl;
		cout << "flag: " << flag << endl;

	}

	//cmd = "CREATE TABLE customer2(\nid int(10) NOT NULL,\nname varchar(25) NOT NULL,\ngender varchar(1) NOT NULL,\nPRIMARY KEY(id));";
	//cout << cmd << endl;




	return 0;
}


