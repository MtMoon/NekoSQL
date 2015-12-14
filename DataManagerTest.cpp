/*
 * DataManagerTest.cpp
 *
 *  Created on: 2015-11-26
 *      Author: yxy
 */

#include "bufmanager/BufPageManager.h"
#include "datamanager/DataManager.h"
#include "sysmanager/SysManager.h"
#include <iostream>
#include <iomanip>
#include <cstring>
using namespace std;

int main() {

	DataManager* dm = new DataManager();
	dm->setDatabase("test1");

	//TableInfo tb1 = dm->getTableInfo("user");
	//TableInfo tb2 = dm->getTableInfo("user2");

	//cout << tb1.Fname[0] << endl;
	//cout << tb2.Fname[0] << endl;

	// test record1

	DP ageDp;
	ageDp.first = "age";
	Data agedata = RecordTool::int2Data(23);
	ageDp.second = agedata;

	DP phoneDp;
	phoneDp.first = "phone number";
	phoneDp.second.first = NULL;
	phoneDp.second.second = 0;

	DP nameDp;
	nameDp.first = "name";
	Data namedata = RecordTool::str2Data("yxy",3);
	nameDp.second = namedata;

	DP array[3];
	array[2] = nameDp;
	array[0] = ageDp;
	array[1] = phoneDp;

	TableInfo tb = dm->getTableInfo("user");
	LP rpos;
	dm->insertRecord("user", array, 3, rpos);
	//dm->deleteRecord("user", LP(1,2));
	vector<LP> LPVec = dm->getAllLPInTable("user");
	for (int i=0; i<LPVec.size(); i++) {
		cout << LPVec[i].first << " " << LPVec[i].second << endl;
	}

	ConDP namecondi;
	namecondi.name = "phone number";
	namecondi.type = 1;
	namecondi.value_str = "";
	namecondi.value_int = 23;
	namecondi.isnull = true;
	vector<LP> ans = dm->searchRecord("user", namecondi, 3);
	cout << "ans size: " << ans.size() << endl;
	for (int i=0; i<ans.size(); i++) {
		LP a = ans[i];
		cout << a.first << " " << a.second << endl;
		//Data d = dm->getRecordByLP("user", a);
		//	RecordTool::printRecord(tb, d);

	}

	//dm->deleteRecord("user", LP(1,0));




	return 0;
}

