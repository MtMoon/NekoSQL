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

	// test record1
	DP nameDp;
	nameDp.first = "name";
	Data namedata = RecordTool::str2Data("yxy",3);
	nameDp.second = namedata;

	DP ageDp;
	ageDp.first = "age";
	Data agedata = RecordTool::str2Data("23",2);
	ageDp.second = agedata;

	DP phoneDp;
	phoneDp.first = "phone number";
	phoneDp.second.first = NULL;
	phoneDp.second.second = 0;

	DP array[3];
	array[0] = nameDp;
	array[1] = ageDp;
	array[2] = phoneDp;

	//dm->insertRecord("user", array, 3);
	vector<LP> ans = dm->searchRecord("user", nameDp);
	cout << ans.size() << endl;
	LP a = ans[0];
	cout << a.first << " " << a.second << endl;



	return 0;
}


